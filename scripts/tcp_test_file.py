import hashlib
import socket
import struct
import sys

HOST, PORT = "127.0.0.1", 5000
DATA_DIR = "data"


def recv_exact(sock, n):
    buf = bytearray()
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError(f"server closed after {len(buf)}/{n} bytes")
        buf += chunk
    return bytes(buf)


def send_frame(sock, payload):
    sock.sendall(struct.pack(">I", len(payload)) + payload)


def recv_frame(sock):
    (length,) = struct.unpack(">I", recv_exact(sock, 4))
    return recv_exact(sock, length)


def request(name):
    with socket.create_connection((HOST, PORT), timeout=5) as sock:
        send_frame(sock, name.encode())
        header = recv_frame(sock)            # framed: 4-byte length, then 16 bytes
        approved, size = struct.unpack("<B7xQ", header)
        body = recv_exact(sock, size) if approved else b""   # raw, not framed
        return approved, size, body


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    names = sys.argv[1:] or ["empty_0B.bin", "one_1B.bin"]

    for bad in ["nope.bin", "../etc/passwd", ""]:
        try:
            approved, _, _ = request(bad)
            print(f"DENIED  {bad!r}: {'PASS' if not approved else 'FAIL (approved)'}")
        except Exception as e:
            print(f"DENIED  {bad!r}: FAIL ({e})")

    with socket.create_connection((HOST, PORT), timeout=5) as sock:
        for name in names:
            send_frame(sock, name.encode())
            approved, size = struct.unpack("<B7xQ", recv_frame(sock))
            body = recv_exact(sock, size)
            print(name, approved, size, len(body))

    '''
    for name in names:
        try:
            approved, size, body = request(name)
            with open(f"{DATA_DIR}/{name}", "rb") as f:
                expected = f.read()
            ok = approved and size == len(expected) and sha(body) == sha(expected)
            print(f"FILE    {name}: {'PASS' if ok else 'FAIL'} (header size={size}, got={len(body)}, expected={len(expected)})")
        except Exception as e:
            print(f"FILE    {name}: FAIL ({e})")
    '''

main()