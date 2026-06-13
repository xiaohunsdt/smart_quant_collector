#!/usr/bin/env python3
"""Prepare shared memory and stdin channel-map blob for standalone rithmic_gateway debug."""

from __future__ import annotations

import ctypes
import ctypes.util
import struct
import subprocess
import sys
from pathlib import Path

SHM_MAGIC = 0x52534D5153484D50
SHM_NAME = b"/sqc_rithmic_debug"

# Matches enabled futures symbols in config/config.yaml (SerializeChannelMap wire format).
CHANNELS = (
    ("CME", "ES", 1),
    ("CME", "NQ", 2),
    ("COMEX", "GC", 3),
)


def repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def shm_size_bytes(root: Path) -> int:
    cpp = root / "scripts" / "print_rithmic_shm_size.cpp"
    debug_dir = root / "build" / "debug"
    debug_dir.mkdir(parents=True, exist_ok=True)
    exe = debug_dir / "print_rithmic_shm_size"
    subprocess.run(
        [
            "g++",
            "-std=c++20",
            f"-I{root}",
            f"-I{root}/src",
            str(cpp),
            "-o",
            str(exe),
        ],
        check=True,
    )
    return int(subprocess.check_output([exe], text=True).strip())


def serialize_channel_map() -> bytes:
    buf = struct.pack("<I", len(CHANNELS))
    for exchange, ticker, channel_id in CHANNELS:
        exc_enc = exchange.encode("ascii")
        tkr_enc = ticker.encode("ascii")
        if len(exc_enc) > 255:
            raise ValueError(f"exchange too long: {exchange}")
        if len(tkr_enc) > 255:
            raise ValueError(f"ticker too long: {ticker}")
        buf += struct.pack("B", len(exc_enc))
        buf += exc_enc
        buf += struct.pack("B", len(tkr_enc))
        buf += tkr_enc
        buf += struct.pack("<I", channel_id)
    return buf


def create_shm(size: int) -> None:
    libc = ctypes.CDLL(ctypes.util.find_library("c"), use_errno=True)
    libc.shm_unlink.argtypes = [ctypes.c_char_p]
    libc.shm_unlink.restype = ctypes.c_int
    libc.shm_open.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_uint]
    libc.shm_open.restype = ctypes.c_int
    libc.ftruncate.argtypes = [ctypes.c_int, ctypes.c_int64]
    libc.ftruncate.restype = ctypes.c_int
    libc.mmap.argtypes = [
        ctypes.c_void_p,
        ctypes.c_size_t,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_int64,
    ]
    libc.mmap.restype = ctypes.c_void_p
    libc.munmap.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
    libc.munmap.restype = ctypes.c_int
    libc.close.argtypes = [ctypes.c_int]
    libc.close.restype = ctypes.c_int

    O_CREAT = 0x40
    O_RDWR = 2
    O_EXCL = 0x80
    PROT_READ = 1
    PROT_WRITE = 2
    MAP_SHARED = 1

    libc.shm_unlink(SHM_NAME)
    fd = libc.shm_open(SHM_NAME, O_CREAT | O_RDWR | O_EXCL, 0o600)
    if fd < 0:
        err = ctypes.get_errno()
        raise OSError(err, f"shm_open({SHM_NAME!r}) failed")

    if libc.ftruncate(fd, size) != 0:
        err = ctypes.get_errno()
        libc.close(fd)
        libc.shm_unlink(SHM_NAME)
        raise OSError(err, "ftruncate failed")

    addr = libc.mmap(None, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)
    if addr == ctypes.c_void_p(-1).value:
        err = ctypes.get_errno()
        libc.close(fd)
        libc.shm_unlink(SHM_NAME)
        raise OSError(err, "mmap failed")

    try:
        ctypes.memmove(addr, struct.pack("<QQ", SHM_MAGIC, 1), 16)
    finally:
        libc.munmap(addr, size)
        libc.close(fd)


def main() -> int:
    root = repo_root()
    debug_dir = root / "build" / "debug"
    debug_dir.mkdir(parents=True, exist_ok=True)

    size = shm_size_bytes(root)
    create_shm(size)

    channel_map_path = debug_dir / "rithmic_channel_map.bin"
    channel_map_path.write_bytes(serialize_channel_map())

    print(f"shm: {SHM_NAME.decode()} ({size} bytes)")
    print(f"channel map: {channel_map_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
