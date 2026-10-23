#!/usr/bin/env python3
"""Generate files of specific sizes for testing socket file transfers.

Usage:
    python gen_test_files.py [--out DIR] [--chunk 1024]

Content is deterministic pseudo-random bytes (seeded per file), so runs are
reproducible and corruption / reordered chunks / truncation will show up in a
checksum comparison. A SHA256SUMS manifest is written alongside the files.
"""
import argparse
import hashlib
import random
from pathlib import Path

FILES_DIRECTORY = "data"
WRITE_BLOCK = 64 * 1024  # how much we generate/write at a time (constant memory)


def sizes(chunk: int) -> dict[str, int]:
    return {
        "empty_0B": 0,
        "one_1B": 1,
        "chunk_minus_1": chunk - 1,
        "chunk_exact": chunk,
        "chunk_plus_1": chunk + 1,
        "two_chunks_exact": chunk * 2,  # exact multiple, last chunk is full
        "1MB": 1024 * 1024,
        "100MB": 100 * 1024 * 1024,
    }


def write_file(path: Path, size: int, seed: int) -> str:
    rng = random.Random(seed)
    h = hashlib.sha256()
    remaining = size
    with open(path, "wb") as f:
        while remaining > 0:
            n = min(WRITE_BLOCK, remaining)
            block = rng.randbytes(n)
            f.write(block)
            h.update(block)
            remaining -= n
    return h.hexdigest()


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--out", default=FILES_DIRECTORY, help="output directory")
    ap.add_argument("--chunk", type=int, default=1024, help="your transfer chunk size")
    args = ap.parse_args()

    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    lines = []
    for i, (name, size) in enumerate(sizes(args.chunk).items()):
        path = out / f"{name}.bin"
        digest = write_file(path, size, seed=i)
        lines.append(f"{digest}  {path.name}")
        print(f"{path.name:24} {size:>12,} bytes")

    (out / "SHA256SUMS").write_text("\n".join(lines) + "\n")
    print(f"\nWrote {len(lines)} files + SHA256SUMS to {out}/")
    print(f"Verify received files with: cd <recv_dir> && sha256sum -c SHA256SUMS")


if __name__ == "__main__":
    main()