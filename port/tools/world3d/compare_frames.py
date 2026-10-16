#!/usr/bin/env python3
"""Compares two binary PPM pictures (gta2 --replay --screenshot): prints the share of differing pixels
(a pixel differs when any channel differs by more than 24) and writes a picture of the differences.

    python3 -I compare_frames.py A.ppm B.ppm DIFF.png

Standard library only."""
import struct
import sys
import zlib


def read_ppm(path):
    with open(path, "rb") as f:
        data = f.read()
    magic, size, maxval, pixels = data.split(b"\n", 3)
    if magic != b"P6" or maxval != b"255":
        raise SystemExit(f"{path}: not a binary 8-bit PPM")
    w, h = map(int, size.split())
    if len(pixels) < w * h * 3:
        raise SystemExit(f"{path}: truncated")
    return w, h, pixels


def write_png(path, w, h, rgb):
    raw = b"".join(b"\x00" + rgb[y * w * 3:(y + 1) * w * 3] for y in range(h))

    def chunk(tag, body):
        return struct.pack(">I", len(body)) + tag + body + struct.pack(">I", zlib.crc32(tag + body) & 0xFFFFFFFF)

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)) +
                chunk(b"IDAT", zlib.compress(raw)) + chunk(b"IEND", b""))


def main():
    if len(sys.argv) != 4:
        raise SystemExit(__doc__)
    wa, ha, a = read_ppm(sys.argv[1])
    wb, hb, b = read_ppm(sys.argv[2])
    if (wa, ha) != (wb, hb):
        raise SystemExit(f"sizes differ: {wa}x{ha} and {wb}x{hb}")
    out = bytearray(wa * ha * 3)
    differing = 0
    for i in range(wa * ha):
        j = i * 3
        d = max(abs(a[j] - b[j]), abs(a[j + 1] - b[j + 1]), abs(a[j + 2] - b[j + 2]))
        if d > 24:
            differing += 1
            out[j] = 255  # red: differs
        else:
            g = a[j] // 3 + a[j + 1] // 3 + a[j + 2] // 3
            out[j] = out[j + 1] = out[j + 2] = g // 3  # the picture, dimmed
    write_png(sys.argv[3], wa, ha, bytes(out))
    print(f"differing pixels: {100.0 * differing / (wa * ha):.2f}%")


main()
