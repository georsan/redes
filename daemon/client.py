#!/usr/bin/python2

import socket
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(('0.0.0.0', 5020))
client.send("CLIENTE EN PYTHON EXAMEN")
client.close()
