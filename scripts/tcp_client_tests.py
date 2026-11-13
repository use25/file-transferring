#!/usr/bin/env python3
"""Manual tests for the length-prefixed echo server.
Protocol: 4-byte big-endian length, then that many bytes. Server echoes in the same framing.
Usage: python3 test_echo_server.py [test_name ...]   (no args = run all)
After each test, read the SERVER log: that is where the interesting output is.
"""
import socket
import struct
import sys
import threading
import time

HOST, PORT = "127.0.0.1", 5000

def connect():
    s = socket.create_connection((HOST, PORT), timeout=3)
    return s

def fetch(s, name):
    s.sendall(frame(name.encode()))
    approved, size = struct.unpack("<B7xQ", read_frame(s))
    body = recv_exact(s, size) if approved else b""
    return approved, size, body

def frame(payload: bytes) -> bytes:
    return struct.pack(">I", len(payload)) + payload


def recv_exact(s, n):
    buf = b""
    while len(buf) < n:
        chunk = s.recv(n - len(buf))
        if not chunk:
            raise EOFError(f"peer closed after {len(buf)}/{n} bytes")
        buf += chunk
    return buf


def read_frame(s):
    (n,) = struct.unpack(">I", recv_exact(s, 4))
    return recv_exact(s, n)


def rst_close(s):
    # SO_LINGER (on, 0s): close() sends RST instead of FIN
    s.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack("ii", 1, 0))
    s.close()


# ---------------------------------------------------------------- tests

def test_1025_rejected():
    """Expect: server logs ERROR_MAX_LENGTH message and drops THIS client only.
    Client sees EOF or ConnectionReset (server closes with unread body -> RST)."""
    s = connect()
    s.sendall(frame(b"a" * 1025))
    try:
        data = s.recv(10)
        print("recv after reject:", data, "(b'' = clean FIN)")
    except ConnectionResetError:
        print("ConnectionResetError (server closed with unread data -> RST)")
    s.close()


def test_zero_length():
    """Expect: server logs '0 bytes' message and drops the client (your protocol rule)."""
    s = connect()
    s.sendall(struct.pack(">I", 0))
    try:
        print("recv:", s.recv(10))
    except ConnectionResetError:
        print("reset")
    s.close()


def test_die_mid_body():
    """Header promises 100 bytes, only 10 arrive, then clean close.
    Expect: server read() returns 0 INSIDE the body loop -> ERROR_SOCKET_CLOSED, thread exits."""
    s = connect()
    s.sendall(struct.pack(">I", 100) + b"x" * 10)
    time.sleep(0.3)
    s.close()


def test_rst_on_read():
    """Send a message, don't read the echo, RST-close.
    Expect: 'errno: ECONNRESET' or EPIPE line on server (timing dependent), clean thread exit."""
    s = connect()
    s.sendall(frame(b"hello"))
    time.sleep(0.2)  # let the server echo into our unread buffer
    rst_close(s)

def test_split_header():

    """Send the 4-byte header in two pieces with a pause between.
    With your CURRENT code this is expected to FAIL (server sees a 2-byte header read and
    disconnects). That is the 'short header' gap, now reproducible on demand.
    A ReadExact helper would make it pass."""
    s = connect()
    hdr = struct.pack(">I", 5)
    s.sendall(hdr[:2])
    time.sleep(0.5)
    s.sendall(hdr[2:] + b"hello")
    try:
        print("echo:", read_frame(s))
    except (EOFError, ConnectionResetError) as e:
        print("FAILED as predicted:", e)
    s.close()

def test_exact_1024():
    """1024-byte name is the max allowed: expect a normal DENIED reply, not a disconnect."""
    s = connect()
    approved, _, _ = fetch(s, "a" * 1024)
    print("denied as expected:", approved == 0)
    s.close()

def test_slow_body():
    """Dribble a valid filename one byte at a time. Expect: file arrives intact."""
    s = connect()
    name = b"one_1B.bin"
    pkt = frame(name)
    s.sendall(pkt[:4])
    for b in name:
        s.sendall(bytes([b]))
        time.sleep(0.05)
    approved, size = struct.unpack("<B7xQ", read_frame(s))
    body = recv_exact(s, size)
    print("body ok:", approved == 1 and body == open("data/one_1B.bin", "rb").read())
    s.close()

def test_many_clients():
    """20 concurrent clients x 20 requests of chunk_plus_1.bin. Expect: no errors."""
    expected = open("data/chunk_plus_1.bin", "rb").read()
    errors = []

    def worker(i):
        try:
            s = connect()
            for j in range(20):
                ok, _, body = fetch(s, "chunk_plus_1.bin")
                if not ok or body != expected:
                    errors.append((i, j))
            s.close()
        except Exception as e:
            errors.append((i, repr(e)))

    ts = [threading.Thread(target=worker, args=(i,)) for i in range(20)]
    [t.start() for t in ts]
    [t.join() for t in ts]
    print("errors:", errors or "none")


def test_kill_mid_transfer():
    """Request 100MB.bin, read a little, RST-close. Expect: server logs ECONNRESET/EPIPE,
    that thread exits, the server keeps running."""
    s = connect()
    s.sendall(frame(b"100MB.bin"))
    read_frame(s)
    s.recv(4096)
    rst_close(s)

TESTS = {n[5:]: f for n, f in globals().items() if n.startswith("test_")}

if __name__ == "__main__":
    names = sys.argv[1:] or list(TESTS)
    for n in names:
        print(f"\n=== {n} ===")
        TESTS[n]()
        time.sleep(0.5)