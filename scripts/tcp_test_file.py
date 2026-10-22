import hashlib
import socket
import struct
import sys

HOST, PORT = "127.0.0.1", 5000
DATA_DIR = "data"


def recv_exact(sock, n):
    buf = bytearray()
    while len(buf) < n:
git add chunk = sock.recv(n - len(buf))
git add if not chunk:
git add     raise ConnectionError(f"server closed after {len(buf)}/{n} bytes")
git add buf += chunk
    return bytes(buf)


def send_frame(sock, payload):
    sock.sendall(struct.pack(">I", len(payload)) + payload)


def recv_frame(sock):
    (length,) = struct.unpack(">I", recv_exact(sock, 4))
    return recv_exact(sock, length)


def request(name):
    with socket.create_connection((HOST, PORT), timeout=5) as sock:
git add send_frame(sock, name.encode())
git add header = recv_frame(sock)git add     # framed: 4-byte length, then 16 bytes
git add approved, size = struct.unpack("<B7xQ", header)
git add body = recv_exact(sock, size) if approved else b""   # raw, not framed
git add return approved, size, body


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    names = sys.argv[1:] or ["chunk_minus_1.bin"]

    for bad in ["nope.bin", "../etc/passwd", ""]:
git add try:
git add     approved, _, _ = request(bad)
git add     print(f"DENIED  {bad!r}: {'PASS' if not approved else 'FAIL (approved)'}")
git add except Exception as e:
git add     print(f"DENIED  {bad!r}: FAIL ({e})")

    for name in names:
git add try:
git add     approved, size, body = request(name)
git add     with open(f"{DATA_DIR}/{name}", "rb") as f:
git add git add expected = f.read()
git add     ok = approved and size == len(expected) and sha(body) == sha(expected)
git add     print(f"FILE    {name}: {'PASS' if ok else 'FAIL'} (header size={size}, got={len(body)}, expected={len(expected)})")
git add except Exception as e:
git add     print(f"FILE    {name}: FAIL ({e})")


main()