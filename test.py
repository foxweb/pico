#!/usr/bin/env python3

import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("localhost", 8000))
sock.send(b"GET /"+b"C"*2000000+b"HTTP/1.1\r\nHost:localhost:8000\r\n\r\n")
response = sock.recv(4096)
sock.close()
