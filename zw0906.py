#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
fingerprint_manager.py
Programa interactivo para enrolar, buscar, comparar y borrar huellas
en un sensor ZW0906 (ZFM-20 protocol), con selección de COM si no se pasa -p.
"""

import sys
import time
import argparse
import serial
import serial.tools.list_ports

# --- Protocolo ZFM ---
HEADER      = bytes([0xEF, 0x01])
ADDRESS     = bytes([0xFF, 0xFF, 0xFF, 0xFF])
CMD_PACKET  = 0x01
ACK_PACKET  = 0x07

# Códigos de comando
PS_GET_IMAGE  = 0x01
PS_GEN_CHAR   = 0x02
PS_MATCH      = 0x03
PS_SEARCH     = 0x04
PS_REG_MODEL  = 0x05
PS_STORE      = 0x06
PS_LOAD_CHAR  = 0x07
PS_DELETE     = 0x0C
PS_EMPTY      = 0x0D
PS_HANDSHAKE  = 0x13  # Verify Password

# Significado de códigos de confirmación
CONFIRM_CODES = {
    0x00: "OK",
    0x01: "Error: turno dedo",
    0x02: "Error: captura",
    0x03: "Error: imagen ruidosa",
    0x06: "Timeout colocando dedo",
    0x07: "Error: no se pueden extraer características",
    0x08: "Error: buffer lleno",
    0x09: "Error: página no encontrada",
    0x0A: "Error: comando inválido",
    0x0B: "Error: ancho de imagen",
    0x0C: "Error: longitud de paquete",
    0x1D: "Error: fallo al operar puerto",
}

# --- Helpers de paquete y checksum ---
def calc_checksum(packet_type: int, payload: bytes) -> bytes:
    length = len(payload) + 2
    hi, lo = (length >> 8) & 0xFF, length & 0xFF
    s = packet_type + hi + lo + sum(payload)
    return bytes([ (s >> 8) & 0xFF, s & 0xFF ])

def build_packet(cmd: int, params: bytes = b'') -> bytes:
    payload = bytes([cmd]) + params
    chk = calc_checksum(CMD_PACKET, payload)
    length = len(payload) + 2
    return HEADER + ADDRESS + bytes([CMD_PACKET, (length>>8)&0xFF, length&0xFF]) + payload + chk

def read_packet(ser: serial.Serial, timeout: float = 1.0) -> bytes:
    deadline = time.time() + timeout
    buf = b''
    while time.time() < deadline:
        n = ser.in_waiting
        if n:
            buf += ser.read(n)
        else:
            time.sleep(0.01)
    return buf

def parse_ack(packet: bytes) -> (int, bytes):
    if len(packet) < 12 or packet[6] != ACK_PACKET:
        raise ValueError("Paquete inválido o demasiado corto")
    length  = (packet[7] << 8) | packet[8]
    payload = packet[9 : 9 + length - 2]
    return payload[0], payload

# --- Comandos de alto nivel ---
def ps_handshake(ser):
    # La contraseña por defecto es 0x00000000 (4 bytes big-endian)
    pwd = (0).to_bytes(4, 'big')
    ser.write(build_packet(PS_HANDSHAKE, pwd))
    code, _ = parse_ack(read_packet(ser))
    return code

def ps_get_image(ser):
    ser.write(build_packet(PS_GET_IMAGE))
    return parse_ack(read_packet(ser))[0]

def ps_gen_char(ser, buf_id):
    ser.write(build_packet(PS_GEN_CHAR, bytes([buf_id])))
    return parse_ack(read_packet(ser))[0]

def ps_reg_model(ser):
    ser.write(build_packet(PS_REG_MODEL))
    return parse_ack(read_packet(ser))[0]

def ps_store(ser, page_id, buf_id):
    params = bytes([buf_id, (page_id>>8)&0xFF, page_id&0xFF])
    ser.write(build_packet(PS_STORE, params))
    return parse_ack(read_packet(ser))[0]

def ps_search(ser, buf_id, start, count):
    params = bytes([
        buf_id,
        (start>>8)&0xFF, start&0xFF,
        (count>>8)&0xFF, count&0xFF
    ])
    ser.write(build_packet(PS_SEARCH, params))
    code, payload = parse_ack(read_packet(ser))
    if code == 0x00:
        mid   = (payload[1]<<8) | payload[2]
        score = (payload[3]<<8) | payload[4]
        return code, mid, score
    return code, None, None

def ps_load_char(ser, page_id, buf_id):
    params = bytes([buf_id, (page_id>>8)&0xFF, page_id&0xFF])
    ser.write(build_packet(PS_LOAD_CHAR, params))
    return parse_ack(read_packet(ser))[0]

def ps_match(ser):
    ser.write(build_packet(PS_MATCH, bytes([1,2])))
    code, payload = parse_ack(read_packet(ser))
    if code == 0x00:
        score = (payload[1]<<8) | payload[2]
        return code, score
    return code, None

def ps_delete(ser, page_id, num=1):
    params = bytes([
        (page_id>>8)&0xFF, page_id&0xFF,
        (num>>8)&0xFF, num&0xFF
    ])
    ser.write(build_packet(PS_DELETE, params))
    return parse_ack(read_packet(ser))[0]

def ps_empty(ser):
    ser.write(build_packet(PS_EMPTY))
    return parse_ack(read_packet(ser))[0]

# --- Serial y utilidades ---
def list_ports():
    return [(p.device, p.description) for p in serial.tools.list_ports.comports()]

def choose_port(ports):
    print("Puertos serie disponibles:")
    for i, (dev, desc) in enumerate(ports):
        print(f"[{i}] {dev} — {desc}")
    sel = input("Elige un puerto (número): ").strip()
    return ports[int(sel)][0]

def open_serial(port, baud):
    ser = serial.Serial(port, baud, timeout=0.1)
    time.sleep(0.5)  # deja arrancar el módulo
    code = ps_handshake(ser)
    if code != 0x00:
        ser.close()
        sys.exit(f"Handshake falló ({hex(code)}): {CONFIRM_CODES.get(code)}")
    return ser

# --- Menú interactivo principal ---
def main():
    parser = argparse.ArgumentParser(description="Gestión de huellas ZW0906")
    parser.add_argument("-p","--port", help="Puerto COM (p.ej. COM3)", default=None)
    parser.add_argument("-b","--baud", help="Baudios (por defecto 57600)",
                        type=int, default=57600)
    args = parser.parse_args()

    port = args.port
    if not port:
        ports = list_ports()
        if not ports:
            sys.exit("No se hallaron puertos serie.")
        port = choose_port(ports)

    ser = open_serial(port, args.baud)
    print(f"Conectado a {port} @ {args.baud} baudios.")

    try:
        while True:
            print("\nMenú:")
            print(" 1) Enrolar huella (ID)")
            print(" 2) Buscar huella en base")
            print(" 3) Match contra ID específico")
            print(" 4) Borrar ID específico")
            print(" 5) Vaciar toda la base")
            print(" 6) Salir")
            opt = input("> ").strip()

            if opt == '1':
                pid = int(input("ID para almacenar (0…65535): "))
                print(" Coloca dedo…",   CONFIRM_CODES.get(ps_get_image(ser),   "??"))
                print(" GenChar 1…",      CONFIRM_CODES.get(ps_gen_char(ser,1),"??"))
                print(" Retira y coloca de nuevo…"); time.sleep(1.5)
                print(" Coloca dedo…",   CONFIRM_CODES.get(ps_get_image(ser),   "??"))
                print(" GenChar 2…",      CONFIRM_CODES.get(ps_gen_char(ser,2),"??"))
                print(" RegModel…",       CONFIRM_CODES.get(ps_reg_model(ser),   "??"))
                print(f" Store ID={pid}…", CONFIRM_CODES.get(ps_store(ser,pid,1), "??"))

            elif opt == '2':
                start = int(input("Página inicio (0): "))
                count = int(input("Número de páginas a buscar: "))
                print(" Coloca dedo…", CONFIRM_CODES.get(ps_get_image(ser), "??"))
                print(" GenChar…",    CONFIRM_CODES.get(ps_gen_char(ser,1), "??"))
                code, mid, score = ps_search(ser,1,start,count)
                if code==0x00:
                    print(f" → Encontrado ID={mid}, Score={score}")
                else:
                    print(" → No coincide:", CONFIRM_CODES.get(code,"??"))

            elif opt == '3':
                pid = int(input("ID a comparar: "))
                print(" Coloca dedo…", CONFIRM_CODES.get(ps_get_image(ser), "??"))
                print(" GenChar…",    CONFIRM_CODES.get(ps_gen_char(ser,1), "??"))
                print(f" LoadChar ID={pid}…", CONFIRM_CODES.get(ps_load_char(ser,pid,2),"??"))
                code, score = ps_match(ser)
                if code==0x00:
                    print(f" → Match OK, Score={score}")
                else:
                    print(" → No match:", CONFIRM_CODES.get(code,"??"))

            elif opt == '4':
                pid = int(input("ID a borrar: "))
                print(f" → Borrando ID={pid}…", CONFIRM_CODES.get(ps_delete(ser,pid),"??"))

            elif opt == '5':
                print(" → Vaciar base…", CONFIRM_CODES.get(ps_empty(ser), "??"))

            elif opt == '6':
                break

            else:
                print("Opción inválida, intenta de nuevo.")

    except KeyboardInterrupt:
        pass
    finally:
        ser.close()
        print("Puerto cerrado, hasta luego.")

if __name__ == "__main__":
    main()
