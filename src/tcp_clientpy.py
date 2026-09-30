import socket, struct

FAKE_LEN = 100000
payload = b'A' * FAKE_LEN

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('127.0.0.1', 5000))
s.sendall(struct.pack('!I', FAKE_LEN))
s.sendall(payload)