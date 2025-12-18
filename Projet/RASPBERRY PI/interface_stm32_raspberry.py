#!/usr/bin/env python3
import serial
import time
import sys

def open_serial(port="/dev/ttyS0", baudrate=115200):
    try:
        ser = serial.Serial(port, baudrate, timeout=1, write_timeout=1)
        print(f"[OK] Port ouvert : {port} @ {baudrate} bauds")
        return ser
    except Exception as e:
        print(f"[ERREUR] Impossible d'ouvrir le port série : {e}")
        sys.exit(1)

def send_command(ser, cmd):
    ser.write((cmd + "\n").encode())
    time.sleep(0.05)
    response = ser.read_all().decode(errors="ignore")
    print(f"→ Réponse à {cmd} : {response if response else '(aucune)'}")
    return response

def menu(ser):
    while True:
        print("""
===========================
   Communication STM32
===========================
1 - Obtenir température (GET_T)
2 - Obtenir pression (GET_P)
0 - Quitter
""")
        choix = input("Choix : ").strip()
        if choix == "1":
            send_command(ser, "GET_T")
        elif choix == "2":
            send_command(ser, "GET_P")
        elif choix == "0":
            print("Fermeture et sortie…")
            return
        else:
            print("Choix invalide.")

if __name__ == "__main__":
    ser = open_serial("/dev/ttyS0", 115200)
    try:
        menu(ser)
    finally:
        ser.close()
        print("[OK] Port série fermé.")