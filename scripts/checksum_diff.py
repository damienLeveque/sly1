#!/usr/bin/env python3
"""
checksum_diff.py - Surgical ELF diff tool for matching decompilation projects

Pinpoints the first byte-level divergence between an original target ELF and
a freshly built one, and classifies the root cause (code mismatch vs. data
size change vs. symbol shift vs. padding drift).

Usage:
  python3 checksum_diff.py [orig] [built] [map] [--sha1 <file>] [--no-color]

Defaults:
  orig  = disc/SCUS_971.98
  built = out/SCUS_971.98
  map   = out/SCUS_971.98.map
"""

from __future__ import annotations

import argparse
import struct
import subprocess
import sys
from bisect import bisect_right
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

# ── toolchain ────────────────────────────────────────────────────────────────
# Override by setting CROSS in the environment (e.g. mips-linux-gnu-, mips64-elf-)
import os
CROSS = os.environ.get("CROSS", "mips-linux-gnu-")


# ── color / output ───────────────────────────────────────────────────────────

class C:
    R = G = Y = CY = B = X = ""

def enable_color():
    C.R  = "\033[91m"
    C.G  = "\033[92m"
    C.Y  = "\033[93m"
    C.CY = "\033[96m"
    C.B  = "\033[1m"
    C.X  = "\033[0m"

def hr(title: str = "") -> None:
    w = 72
    if title:
        pad = (w - len(title) - 2) // 2
        print(f"\n{C.CY}{'─'*pad} {title} {'─'*(w-pad-len(title)-2)}{C.X}")
    else:
        print(f"{C.CY}{'─'*w}{C.X}")

def ok(m: str)   -> None: print(f"  {C.G}✓{C.X} {m}")
def err(m: str)  -> None: print(f"  {C.R}✗{C.X} {m}")
def warn(m: str) -> None: print(f"  {C.Y}!{C.X} {m}")
def info(m: str) -> None: print(f"  {C.CY}→{C.X} {m}")


# ── ELF parsing (native, no readelf dependency for the critical path) ────────

@dataclass
class ElfInfo:
    path: Path
    data: bytes
    is_le: bool                    # endianness
    is_64: bool                    # ELFCLASS
    e_type: int
    e_machine: int
    e_entry: int
    e_phoff: int
    e_shoff: int
    e_phentsize: int
    e_phnum: int
    e_shentsize: int
    e_shnum: int
    e_shstrndx: int
    segments: list[dict]           # PT_LOAD entries
    sections: list[dict]           # section header table

def parse_elf(path: Path) -> ElfInfo:
    data = path.read_bytes()
    if data[:4] != b"\x7fELF":
        raise ValueError(f"{path}: not an ELF file")

    ei_class = data[4]   # 1=32bit, 2=64bit
    ei_data  = data[5]   # 1=LE, 2=BE
    is_64 = (ei_class == 2)
    is_le = (ei_data == 1)
    endian = "<" if is_le else ">"

    if is_64:
        # Ehdr64: e_type(H) machine(H) version(I) entry(Q) phoff(Q) shoff(Q)
        #         flags(I) ehsize(H) phentsize(H) phnum(H) shentsize(H)
        #         shnum(H) shstrndx(H)
        fmt = endian + "HHIQQQIHHHHHH"
        (e_type, e_machine, _, e_entry, e_phoff, e_shoff, _,
         _, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx) = \
            struct.unpack_from(fmt, data, 16)
        ph_fmt = endian + "IIQQQQQQ"  # type flags offset vaddr paddr filesz memsz align
        ph_fields = ("p_type","p_flags","p_offset","p_vaddr","p_paddr",
                     "p_filesz","p_memsz","p_align")
        sh_fmt = endian + "IIQQQQIIQQ"
        sh_fields = ("sh_name","sh_type","sh_flags","sh_addr","sh_offset",
                     "sh_size","sh_link","sh_info","sh_addralign","sh_entsize")
    else:
        fmt = endian + "HHIIIIIHHHHHH"
        (e_type, e_machine, _, e_entry, e_phoff, e_shoff, _,
         _, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx) = \
            struct.unpack_from(fmt, data, 16)
        ph_fmt = endian + "IIIIIIII"  # type offset vaddr paddr filesz memsz flags align
        ph_fields = ("p_type","p_offset","p_vaddr","p_paddr","p_filesz",
                     "p_memsz","p_flags","p_align")
        sh_fmt = endian + "IIIIIIIIII"
        sh_fields = ("sh_name","sh_type","sh_flags","sh_addr","sh_offset",
                     "sh_size","sh_link","sh_info","sh_addralign","sh_entsize")

    segments = []
    for i in range(e_phnum):
        off = e_phoff + i * e_phentsize
        vals = struct.unpack_from(ph_fmt, data, off)
        d = dict(zip(ph_fields, vals))
        if d["p_type"] == 1:  # PT_LOAD
            segments.append(d)

    sections = []
    for i in range(e_shnum):
        off = e_shoff + i * e_shentsize
        vals = struct.unpack_from(sh_fmt, data, off)
        sections.append(dict(zip(sh_fields, vals)))

    # resolve section names
    if sections and e_shstrndx < len(sections):
        strtab = sections[e_shstrndx]
        strdata = data[strtab["sh_offset"]:strtab["sh_offset"]+strtab["sh_size"]]
        for s in sections:
            n = s["sh_name"]
            end = strdata.find(b"\x00", n)
            s["name"] = strdata[n:end].decode("ascii", errors="replace") if end >= 0 else ""

    return ElfInfo(
        path=path, data=data, is_le=is_le, is_64=is_64,
        e_type=e_type, e_machine=e_machine, e_entry=e_entry,
        e_phoff=e_phoff, e_shoff=e_shoff,
        e_phentsize=e_phentsize, e_phnum=e_phnum,
        e_shentsize=e_shentsize, e_shnum=e_shnum, e_shstrndx=e_shstrndx,
        segments=segments, sections=sections,
    )

def file_to_vaddr(elf: ElfInfo, off: int):
    for s in elf.segments:
        if s["p_offset"] <= off < s["p_offset"] + s["p_filesz"]:
            return s["p_vaddr"] + (off - s["p_offset"])
    return None

def section_at_vaddr(elf: ElfInfo, vaddr: int):
    """Return the section containing a given virtual address (or None)."""
    for s in elf.sections:
        if s["sh_addr"] and s["sh_addr"] <= vaddr < s["sh_addr"] + s["sh_size"]:
            return s
    return None

def section_at_offset(elf: ElfInfo, off: int):
    """Return the section containing a given file offset (or None)."""
    for s in elf.sections:
        if s["sh_size"] and s["sh_type"] != 8:  # skip SHT_NOBITS (.bss)
            if s["sh_offset"] <= off < s["sh_offset"] + s["sh_size"]:
                return s
    return None


# ── symbol parsing via nm (kept as a convenience; tolerant of column count) ──

def run_tool(tool: str, *args) -> str:
    try:
        r = subprocess.run([CROSS + tool, *args],
                           capture_output=True, text=True, check=False)
        return r.stdout
    except FileNotFoundError:
        return ""

def nm_defined(path: Path) -> list[tuple[int, str, str]]:
    """Returns sorted list of (addr, type, name) for defined symbols."""
    out = []
    raw = run_tool("nm", "-n", "--defined-only", str(path))
    if not raw:
        return out
    for line in raw.splitlines():
        parts = line.split()
        # possible shapes:
        #   <addr> <type> <name>
        #   <addr> <size> <type> <name>   (with --print-size; we don't use it but be defensive)
        if len(parts) < 3:
            continue
        try:
            addr = int(parts[0], 16)
        except ValueError:
            continue
        if len(parts) == 3:
            out.append((addr, parts[1], parts[2]))
        else:
            # assume last is name, second-to-last is type
            out.append((addr, parts[-2], parts[-1]))
    out.sort()
    return out

def nm_sized(path: Path) -> dict[str, tuple[int, int]]:
    """Returns {name: (addr, size)} using `nm -S`."""
    result = {}
    raw = run_tool("nm", "-n", "-S", "--defined-only", str(path))
    if not raw:
        return result
    for line in raw.splitlines():
        parts = line.split()
        if len(parts) < 4:
            continue
        try:
            addr = int(parts[0], 16)
            size = int(parts[1], 16)
        except ValueError:
            continue
        name = parts[-1]
        result[name] = (addr, size)
    return result


# ── map file (linker map — format varies: GNU ld, ProDG, etc.) ───────────────

def load_map(path: Path) -> list[tuple[int, str]]:
    """Load a linker map. Accepts any line starting with 0x... <name>.
    Robust to GNU ld and ProDG-style maps."""
    syms = []
    try:
        with open(path) as f:
            for line in f:
                s = line.strip()
                if not s or not s.startswith("0x"):
                    continue
                parts = s.split()
                if len(parts) < 2:
                    continue
                try:
                    addr = int(parts[0], 16)
                except ValueError:
                    continue
                # name is usually last token; skip section header lines like ".text  0x..."
                name = parts[-1]
                if name.startswith(".") or name in ("=", "ASSERT", "LOAD"):
                    continue
                syms.append((addr, name))
    except FileNotFoundError:
        return []
    syms.sort()
    return syms

def sym_before(map_syms: list[tuple[int,str]], addr: int, window: int = 0x2000):
    """Return the symbol whose address is the largest <= addr, within window."""
    if not map_syms:
        return None
    addrs = [a for a, _ in map_syms]
    i = bisect_right(addrs, addr) - 1
    if i < 0:
        return None
    a, name = map_syms[i]
    if addr - a > window:
        return None
    return (a, name)


# ── run grouping ─────────────────────────────────────────────────────────────

def find_diff_runs(a: bytes, b: bytes) -> list[tuple[int,int]]:
    """Return list of (start, end_inclusive) where a and b differ, over min(len)."""
    n = min(len(a), len(b))
    runs: list[tuple[int,int]] = []
    i = 0
    while i < n:
        if a[i] != b[i]:
            j = i
            while j < n and a[j] != b[j]:
                j += 1
            runs.append((i, j - 1))
            i = j
        else:
            i += 1
    return runs


# ── MIPS instruction classification (endian-aware) ───────────────────────────

def read_word(data: bytes, off: int, is_le: bool) -> int | None:
    if off + 4 > len(data):
        return None
    return struct.unpack_from("<I" if is_le else ">I", data, off)[0]

def classify_instruction_diff(w_orig: int, w_built: int, pc: int):
    """Given two differing MIPS instruction words, return a human-readable hint."""
    if w_orig == w_built:
        return None

    op_o = (w_orig  >> 26) & 0x3F
    op_b = (w_built >> 26) & 0x3F

    # opcode changed entirely
    if op_o != op_b:
        return f"opcode changed: 0x{op_o:02x} → 0x{op_b:02x} (likely different instruction emitted)"

    # J / JAL: 26-bit target shifted left 2, combined with top 4 bits of PC+4
    if op_o in (0x02, 0x03):  # J, JAL
        tgt_o = ((pc + 4) & 0xF0000000) | ((w_orig  & 0x03FFFFFF) << 2)
        tgt_b = ((pc + 4) & 0xF0000000) | ((w_built & 0x03FFFFFF) << 2)
        name = "J" if op_o == 0x02 else "JAL"
        return f"{name} target: 0x{tgt_o:08x} → 0x{tgt_b:08x} (shift {tgt_b-tgt_o:+d})"

    # Branches: 16-bit signed offset
    if op_o in (0x04, 0x05, 0x06, 0x07, 0x01):  # BEQ, BNE, BLEZ, BGTZ, REGIMM
        def bt(w):
            off = w & 0xFFFF
            if off & 0x8000: off -= 0x10000
            return (pc + 4 + (off << 2)) & 0xFFFFFFFF
        return f"branch target: 0x{bt(w_orig):08x} → 0x{bt(w_built):08x}"

    # ADDIU / ADDI / ORI / ANDI / XORI / LUI / LW / SW — I-type; check imm vs regs
    I_TYPES = {0x08:"ADDI", 0x09:"ADDIU", 0x0C:"ANDI", 0x0D:"ORI", 0x0E:"XORI",
               0x0F:"LUI",  0x23:"LW",    0x2B:"SW",   0x20:"LB",   0x24:"LBU",
               0x21:"LH",   0x25:"LHU",   0x28:"SB",   0x29:"SH"}
    if op_o in I_TYPES:
        rs_o, rt_o = (w_orig  >> 21) & 0x1F, (w_orig  >> 16) & 0x1F
        rs_b, rt_b = (w_built >> 21) & 0x1F, (w_built >> 16) & 0x1F
        imm_o = w_orig  & 0xFFFF
        imm_b = w_built & 0xFFFF
        if op_o in (0x08, 0x09):  # sign-extend for ADDI/ADDIU
            if imm_o & 0x8000: imm_o -= 0x10000
            if imm_b & 0x8000: imm_b -= 0x10000
        hints = []
        if (rs_o, rt_o) != (rs_b, rt_b):
            hints.append(f"regs ${rs_o},${rt_o} → ${rs_b},${rt_b}")
        if imm_o != imm_b:
            hints.append(f"imm {imm_o} → {imm_b} (Δ={imm_b-imm_o:+d})")
        if hints:
            return f"{I_TYPES[op_o]}: " + ", ".join(hints)

    # SPECIAL (R-type) — register allocation changes show up here a lot
    if op_o == 0x00:
        def rtype(w):
            return ((w>>21)&0x1F, (w>>16)&0x1F, (w>>11)&0x1F, (w>>6)&0x1F, w & 0x3F)
        rs_o, rt_o, rd_o, sh_o, fn_o = rtype(w_orig)
        rs_b, rt_b, rd_b, sh_b, fn_b = rtype(w_built)
        if fn_o != fn_b:
            return f"R-type funct 0x{fn_o:02x} → 0x{fn_b:02x}"
        return (f"R-type regs: ${rs_o},${rt_o},${rd_o} → "
                f"${rs_b},${rt_b},${rd_b} (reg-alloc drift)")

    return f"word differs: 0x{w_orig:08x} → 0x{w_built:08x}"


# ── SHA1 comparison (the authoritative check) ────────────────────────────────

import hashlib

def sha1(data: bytes) -> str:
    return hashlib.sha1(data).hexdigest()

def load_expected_sha1(path: Path) -> str | None:
    if not path.exists():
        return None
    try:
        line = path.read_text().strip().split()[0]
        return line.lower() if len(line) == 40 else None
    except Exception:
        return None


# ── main ─────────────────────────────────────────────────────────────────────

def build_parser():
    p = argparse.ArgumentParser(
        description="Surgical ELF diff for matching decomp projects.")
    p.add_argument("orig",  nargs="?", default="disc/SCUS_971.98")
    p.add_argument("built", nargs="?", default="out/SCUS_971.98")
    p.add_argument("map",   nargs="?", default="out/SCUS_971.98.map")
    p.add_argument("--sha1", default="config/checksum.sha1",
                   help="File containing expected sha1 (default: config/checksum.sha1)")
    p.add_argument("--no-color", action="store_true",
                   help="Disable ANSI colors (auto-disabled when stdout isn't a TTY)")
    p.add_argument("--max-ranges", type=int, default=30,
                   help="Max diff ranges to display (default 30)")
    return p

def print_banner(orig, built, map_path, sha1_path):
    print(f"\n{C.B}{C.CY}╔════════════════════════════════════════════════╗{C.X}")
    print(f"{C.B}{C.CY}║       SURGICAL CHECKSUM DIFF  (generic)        ║{C.X}")
    print(f"{C.B}{C.CY}╚════════════════════════════════════════════════╝{C.X}")
    print(f"  orig  : {orig}")
    print(f"  built : {built}")
    print(f"  map   : {map_path}")
    print(f"  sha1  : {sha1_path}")

def pretty_name(name: str) -> str:
    """Strip .NON_MATCHING suffix and highlight if present."""
    if name.endswith(".NON_MATCHING"):
        return name[:-len(".NON_MATCHING")] + f" {C.Y}(NON_MATCHING stub){C.X}"
    return name

def main() -> int:
    args = build_parser().parse_args()

    if not args.no_color and sys.stdout.isatty():
        enable_color()

    orig_path  = Path(args.orig)
    built_path = Path(args.built)
    map_path   = Path(args.map)
    sha1_path  = Path(args.sha1)

    for p in (orig_path, built_path):
        if not p.exists():
            print(f"error: {p} not found", file=sys.stderr)
            return 2

    print_banner(orig_path, built_path, map_path, sha1_path)

    orig  = parse_elf(orig_path)
    built = parse_elf(built_path)

    # ── 0. SHA1 (authoritative) ──────────────────────────────────────────────
    hr("0  SHA1 (authoritative)")
    h_orig  = sha1(orig.data)
    h_built = sha1(built.data)
    if h_orig == h_built:
        ok(f"IDENTICAL: {h_built}")
        expected = load_expected_sha1(sha1_path)
        if expected and expected != h_built:
            warn(f"But {sha1_path} expects {expected}")
        return 0
    err(f"orig : {h_orig}")
    err(f"built: {h_built}")
    expected = load_expected_sha1(sha1_path)
    if expected:
        match_o = "✓" if expected == h_orig  else "✗"
        match_b = "✓" if expected == h_built else "✗"
        info(f"expected: {expected}   orig {match_o}   built {match_b}")

    # ── 1. File size ─────────────────────────────────────────────────────────
    hr("1  FILE SIZE")
    size_delta = len(built.data) - len(orig.data)
    if size_delta == 0:
        ok(f"Identical: {len(orig.data):,} bytes")
        warn("Same size but checksum fails → content mismatch (code or data value)")
    else:
        err(f"orig={len(orig.data):,}  built={len(built.data):,}  delta={size_delta:+d}")
        if abs(size_delta) % 4 == 0:
            info(f"{abs(size_delta)//4} words — consistent with code/data insertion")
        else:
            info("not a multiple of 4 — likely string or char-array length change")

    # ── 2. ELF header ────────────────────────────────────────────────────────
    hr("2  ELF HEADER")
    checks = [
        ("class",     "64-bit" if orig.is_64 else "32-bit",
                      "64-bit" if built.is_64 else "32-bit"),
        ("endian",    "little" if orig.is_le else "big",
                      "little" if built.is_le else "big"),
        ("e_machine", hex(orig.e_machine), hex(built.e_machine)),
        ("e_type",    hex(orig.e_type),    hex(built.e_type)),
        ("e_entry",   hex(orig.e_entry),   hex(built.e_entry)),
        ("e_phnum",   orig.e_phnum,        built.e_phnum),
        ("e_shnum",   orig.e_shnum,        built.e_shnum),
    ]
    for name, o, b in checks:
        (err if o != b else ok)(
            f"{name}: {o}" if o == b else f"{name}: orig={o}  built={b}")

    # ── 3. LOAD segments ─────────────────────────────────────────────────────
    hr("3  LOAD SEGMENTS")
    if len(orig.segments) != len(built.segments):
        err(f"PT_LOAD count differs: {len(orig.segments)} vs {len(built.segments)}")
    for i, (o, b) in enumerate(zip(orig.segments, built.segments)):
        keys = ("p_vaddr", "p_filesz", "p_memsz")
        same = {k: o[k] == b[k] for k in keys}
        if all(same.values()):
            bss = o["p_memsz"] - o["p_filesz"]
            ok(f"LOAD[{i}] vaddr=0x{o['p_vaddr']:08x} filesz=0x{o['p_filesz']:x} "
               f"memsz=0x{o['p_memsz']:x}  (.bss=0x{bss:x})")
        else:
            err(f"LOAD[{i}] DIFFERS:")
            for k in keys:
                if not same[k]:
                    print(f"    {k}: orig=0x{o[k]:x}  built=0x{b[k]:x}  "
                          f"Δ={b[k]-o[k]:+d}")
            o_bss = o["p_memsz"] - o["p_filesz"]
            b_bss = b["p_memsz"] - b["p_filesz"]
            if o_bss != b_bss:
                print(f"    .bss size: orig=0x{o_bss:x}  built=0x{b_bss:x}  "
                      f"Δ={b_bss-o_bss:+d}")
                info("   → bss_size in splat yaml may need adjusting, "
                     "or a global's size changed")

    # ── 4. Sections summary ──────────────────────────────────────────────────
    hr("4  SECTION SIZES")
    by_name_o = {s["name"]: s for s in orig.sections if s.get("name")}
    by_name_b = {s["name"]: s for s in built.sections if s.get("name")}
    interesting = (".text", ".vutext", ".rodata", ".data", ".bss", ".ctors", ".dtors")
    section_mismatches = []
    for name in interesting:
        if name in by_name_o and name in by_name_b:
            o = by_name_o[name]; b = by_name_b[name]
            if o["sh_size"] != b["sh_size"]:
                err(f"{name:<10} size: 0x{o['sh_size']:x} → 0x{b['sh_size']:x} "
                    f"(Δ={b['sh_size']-o['sh_size']:+d})")
                section_mismatches.append(name)
            else:
                ok(f"{name:<10} size=0x{o['sh_size']:x}")

    # ── 5. Raw byte diff ─────────────────────────────────────────────────────
    hr("5  RAW BYTE DIFF")
    runs = find_diff_runs(orig.data, built.data)

    if not runs:
        if size_delta != 0:
            warn("Common bytes identical — extra bytes only at tail")
            extra = built.data[len(orig.data):] if size_delta > 0 else orig.data[len(built.data):]
            info(f"First 32 extra bytes: {extra[:32].hex(' ')}")
            if all(b == 0 for b in extra):
                info("All-zero tail — linker alignment padding")
        return 1

    total_diff_bytes = sum(e - s + 1 for s, e in runs)
    err(f"{total_diff_bytes:,} bytes differ in {len(runs)} ranges")

    # First diff deep-dive
    fs, fe = runs[0]
    print(f"\n  {C.B}First diff → file offset 0x{fs:08x}{C.X}")
    print(f"    orig : {orig.data [fs:fs+16].hex(' ')}")
    print(f"    built: {built.data[fs:fs+16].hex(' ')}")

    vfs = file_to_vaddr(orig, fs)
    sec = section_at_offset(orig, fs)
    if sec:
        info(f"Section: {sec['name']}  (offset in section: 0x{fs - sec['sh_offset']:x})")

    # Instruction-level classification ONLY if this is in .text / .vutext
    if sec and sec["name"] in (".text", ".vutext") and vfs is not None:
        w_o = read_word(orig.data,  fs, orig.is_le)
        w_b = read_word(built.data, fs, built.is_le)
        if w_o is not None and w_b is not None:
            hint = classify_instruction_diff(w_o, w_b, vfs)
            if hint:
                info(hint)

    # Map symbol attribution (within a reasonable window)
    map_syms = load_map(map_path)
    if vfs is not None and map_syms:
        sb = sym_before(map_syms, vfs)
        if sb:
            sa, sname = sb
            print(f"    {C.CY}↳{C.X} symbol: {C.R}{pretty_name(sname)}{C.X}"
                  f"  @0x{sa:08x}  (+0x{vfs-sa:x})")

    # ── 6. Diff ranges → symbols ─────────────────────────────────────────────
    hr("6  DIFF RANGES → SYMBOLS")
    seen: set[str] = set()
    shown = 0
    non_matching_hits = 0
    for s, e in runs:
        v = file_to_vaddr(orig, s)
        if v is None:
            continue
        sb = sym_before(map_syms, v) if map_syms else None
        label = sb[1] if sb else f"0x{v:08x}"
        sec = section_at_offset(orig, s)
        sec_name = sec["name"] if sec else "?"

        if label.endswith(".NON_MATCHING"):
            non_matching_hits += 1

        if label not in seen:
            seen.add(label)
            tag = f"  {C.R}← FIRST{C.X}" if shown == 0 else ""
            print(f"  0x{s:08x}–0x{e:08x}  {e-s+1:5d}B  "
                  f"[{sec_name:<8}] {C.Y}{pretty_name(label)}{C.X}{tag}")
            shown += 1
            if shown >= args.max_ranges:
                print(f"  ... {len(runs)-shown} more ranges")
                break

    if non_matching_hits:
        warn(f"{non_matching_hits} diff ranges fall inside .NON_MATCHING stubs "
             "(expected — ignore unless nothing else is flagged)")

    # ── 7. Symbol shifts ─────────────────────────────────────────────────────
    hr("7  SYMBOL ADDRESS SHIFTS")
    o_nm = {n: a for a, _, n in nm_defined(orig_path)}
    b_nm = {n: a for a, _, n in nm_defined(built_path)}
    shifted = sorted(
        (o_nm[n], b_nm[n] - o_nm[n], n)
        for n in o_nm if n in b_nm and o_nm[n] != b_nm[n]
    )
    if not shifted:
        warn("No named symbol shifts — divergence is in unnamed/static data or tail padding")
    else:
        fa, fshift, fname = shifted[0]
        err(f"First shift: {pretty_name(fname)} @ 0x{fa:08x}  Δ={fshift:+d}")
        info(f"Something BEFORE 0x{fa:08x} grew by {abs(fshift)} bytes")
        print(f"\n  First 10 shifted symbols:")
        for addr, shift, name in shifted[:10]:
            print(f"    0x{addr:08x}  {shift:+6d}  {pretty_name(name)}")
        if map_syms:
            cands = [(a, n) for a, n in map_syms
                     if a < fa and a > fa - 0x800
                     and not n.endswith(".NON_MATCHING")
                     and not n.startswith("_")]
            if cands:
                print(f"\n  {C.Y}Candidates just before first shift:{C.X}")
                for a, n in cands[-8:]:
                    print(f"    0x{a:08x}  {n}")

    # ── 8. Symbol size changes ───────────────────────────────────────────────
    hr("8  SYMBOL SIZE CHANGES")
    o_sz = nm_sized(orig_path)
    b_sz = nm_sized(built_path)
    changed = sorted(
        (o_sz[n][0], o_sz[n][1], b_sz[n][1], b_sz[n][1] - o_sz[n][1], n)
        for n in o_sz if n in b_sz and o_sz[n][1] != b_sz[n][1]
    )
    if not changed:
        ok("No symbol size changes")
    else:
        err(f"{len(changed)} symbols changed size:")
        for addr, os_, bs_, delta, name in changed[:15]:
            print(f"    0x{addr:08x}  {delta:+6d}B  {pretty_name(name)}  "
                  f"({os_} → {bs_})")

    # ── 9. Tail & padding ────────────────────────────────────────────────────
    hr("9  TAIL & PADDING")
    if size_delta > 0:
        extra = built.data[len(orig.data):]
        if all(b == 0 for b in extra):
            warn(f"{size_delta} NULL bytes appended — linker alignment padding")
            info("Root cause is earlier in the file — see sections 6/7/8")
        else:
            warn(f"{size_delta} non-null bytes appended")
            info(f"Content: {extra[:32].hex(' ')}")
    elif size_delta < 0:
        missing = orig.data[len(built.data):]
        warn(f"{-size_delta} bytes missing from tail")
        if all(b == 0 for b in missing):
            info("All-zero — built ELF is missing linker padding")
    else:
        ok("No tail size issue")

    # ── 10. Diagnosis ────────────────────────────────────────────────────────
    hr("10  DIAGNOSIS & NEXT STEPS")

    abs_delta = abs(size_delta)
    direction = "larger" if size_delta > 0 else ("smaller" if size_delta < 0 else "same size")

    first_sym = None
    if runs:
        v = file_to_vaddr(orig, runs[0][0])
        if v is not None and map_syms:
            sb = sym_before(map_syms, v)
            if sb:
                first_sym = sb[1]

    first_sec = section_at_offset(orig, runs[0][0]) if runs else None
    first_sec_name = first_sec["name"] if first_sec else "?"

    # categorize
    if size_delta == 0:
        err(f"Same size — content mismatch in {first_sec_name}")
        if first_sym:
            if first_sym.endswith(".NON_MATCHING"):
                warn(f"First mismatching symbol is a NON_MATCHING stub — "
                     "this is expected until you write the matching C")
            else:
                print(f"  First mismatching symbol: {C.R}{pretty_name(first_sym)}{C.X}")
        print("\n  Next steps:")
        if first_sec_name == ".text":
            print("    • Run:  ./scripts/diff.sh <mangled_symbol>")
            print("    • Common code-mismatch causes:")
            print("        - wrong struct layout (field offset/size)")
            print("        - signed vs unsigned (char, short)")
            print("        - pointer vs value (missing * or &)")
            print("        - wrong comparison operator (< vs <=, == vs !=)")
            print("        - rodata literals out of order "
                  "(affects `migrate_rodata_to_functions`)")
        elif first_sec_name in (".rodata", ".data"):
            print("    • A global initializer value changed")
            print("    • Check float/string literal ordering")
            print("    • Verify data segment assignment in config/sly1.yaml")
        elif first_sec_name == ".vutext":
            print("    • VU microcode mismatch — check vutext binary")

    elif section_mismatches:
        err(f"ELF is {abs_delta}B {direction} — sections changed size: "
            f"{', '.join(section_mismatches)}")
        print("\n  Next steps:")
        if ".text" in section_mismatches:
            print("    • A function body changed length — check recently edited src/")
        if ".rodata" in section_mismatches:
            print("    • A string literal, float constant, or jumptable changed")
            print("    • Check rodata TU assignments in config/sly1.yaml")
        if ".data" in section_mismatches:
            print("    • A global's size/initializer changed")
            print("    • Check data TU assignments in config/sly1.yaml")
        if ".bss" in section_mismatches:
            print("    • An uninitialized global's size changed")
            print("    • Adjust `bss_size` in config/sly1.yaml if needed")

    elif changed:
        err(f"ELF is {abs_delta}B {direction} — {len(changed)} symbol size change(s)")
        print("\n  Next steps:")
        print("    • Each changed symbol points to a struct/array whose size drifted")
        print("    • Look at the symbol's header file and check field counts/types")

    elif shifted:
        fa, fshift, fname = shifted[0]
        err(f"ELF is {abs_delta}B {direction} — bloat precedes {pretty_name(fname)} "
            f"@ 0x{fa:08x}")
        print("\n  Next steps:")
        print("    • Inspect 'Candidates just before first shift' above")
        print("    • Run:  git diff include/ src/")
        print("    • Check for:")
        print("        - added/removed function")
        print("        - struct field size change")
        print("        - global array resized")

    elif abs_delta > 0:
        err(f"ELF is {abs_delta}B {direction} with no named-symbol changes")
        print("  Likely: unnamed static data or linker padding shift")
        print("\n  Next steps:")
        print("    • Run:  git diff --stat")
        print("    • Look at the first diff's section (see section 5)")

    print()
    return 1

if __name__ == "__main__":
    sys.exit(main())