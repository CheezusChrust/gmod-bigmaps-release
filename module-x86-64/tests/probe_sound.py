import hashlib
import struct

from unicorn.x86_const import (UC_X86_REG_R8, UC_X86_REG_R9, UC_X86_REG_RBX, UC_X86_REG_RCX,
                               UC_X86_REG_RDX, UC_X86_REG_R13, UC_X86_REG_R14, UC_X86_REG_R15,
                               UC_X86_REG_RDI, UC_X86_REG_RSI, UC_X86_REG_RSP)

import probe_coord as probe


START = 0x180211C49
END = 0x180212084
STUB = 0x30001000
SOUND_ENTRY = bytes.fromhex("f30f100d3bb418004585c90f8413010000")
DEST = probe.codec.PROP + 0x100
PREVIOUS = probe.codec.PROP + 0x200
WRITER_START = 0x18014F5C5
WRITER_END = 0x18014F776
WRITER_SITES = (
    (0x18014F63A, "41b80c000000", "41b80d000000"),
    (0x18014F6C9, "41b80c000000", "41b80d000000"),
    (0x18014F758, "41b80c000000", "41b80d000000"),
)
SITES = (
    (0x180211C5F, "8d41f4", "8d41f3"),
    (0x180211C65, "44230df45a1f00", "44230df85a1f00"),
    (0x180211C74, "c1ea0c", "c1ea0d"),
    (0x180211D57, "41c1e114", "41c1e113"),
    (0x180211D5B, "41c1f914", "41c1f913"),
    (0x180211DE5, "8d41f4", "8d41f3"),
    (0x180211DEB, "44230d6e591f00", "44230d72591f00"),
    (0x180211DFA, "c1ea0c", "c1ea0d"),
    (0x180211EDD, "41c1e114", "41c1e113"),
    (0x180211EE1, "41c1f914", "41c1f913"),
    (0x180211F6B, "8d41f4", "8d41f3"),
    (0x180211F71, "44230de8571f00", "44230dec571f00"),
    (0x180211F80, "c1ea0c", "c1ea0d"),
    (0x180212062, "41c1e114", "41c1e113"),
    (0x180212066, "41c1f914", "41c1f913"),
)


def check_writer(width):
    machine = probe.codec.load(str(probe.ENGINE))
    for offset in range(32):
        for mask_width in range(33):
            mask = ~(((1 << mask_width) - 1) << offset) & 0xFFFFFFFF
            machine.mem_write(probe.WRITE_MASKS + 4 * (offset * 33 + mask_width), struct.pack("<I", mask))
    if width == 13:
        for address, original, replacement in WRITER_SITES:
            original_bytes = bytes.fromhex(original)
            if bytes(machine.mem_read(address, len(original_bytes))) != original_bytes:
                raise RuntimeError(f"Unexpected instruction at {address:#x}")
            machine.mem_write(address, bytes.fromhex(replacement))
    previous = (111.0, 222.0, 333.0)
    samples = ((0, 1, -1), (2047, -2048, 2048), (4095, -4096, 3000), (-3000, 123, -456))
    cases = 0
    for values in samples:
        for flags in range(8):
            for offset in (0, 1, 7, 18, 19, 30, 31, 32, 63):
                bits = []
                for axis, value in enumerate(values):
                    present = (flags >> axis) & 1
                    bits.append(present)
                    if present:
                        bits.extend((value >> bit) & 1 for bit in range(width - 1))
                        bits.append(int(value < 0))
                expected = sum(bit << (offset + index) for index, bit in enumerate(bits))
                machine.mem_write(probe.codec.BUF, b"\0" * 64)
                probe.codec.reset_bf(machine, offset)
                machine.mem_write(PREVIOUS + 0x14, struct.pack("<fff", *previous))
                current = tuple(values[axis] * 8.0 if flags & (1 << axis) else previous[axis] for axis in range(3))
                machine.mem_write(DEST + 0x14, struct.pack("<fff", *current))
                for register, value in ((UC_X86_REG_RBX, probe.codec.BF), (UC_X86_REG_RSI, PREVIOUS),
                                        (UC_X86_REG_RDI, DEST), (UC_X86_REG_RSP, probe.codec.STACK + 0xF0000)):
                    machine.reg_write(register, value)
                machine.emu_start(WRITER_START, WRITER_END, count=100000)
                actual = int.from_bytes(machine.mem_read(probe.codec.BUF, 64), "little")
                checked = (1 << (offset + len(bits))) - 1
                if actual & checked != expected or probe.codec.cur_bit(machine) != offset + len(bits):
                    raise AssertionError(f"sound writer mismatch: {width=} {values=} {flags=} {offset=}")
                cases += 1
    print(f"{width}-bit sound-coordinate writer block: {cases} independent-model cases passed")


def check(width):
    machine = probe.codec.load(str(probe.ENGINE))
    if width == 13:
        if bytes(machine.mem_read(START, len(SOUND_ENTRY))) != SOUND_ENTRY:
            raise RuntimeError("Unexpected sound-reader entry instructions")
        machine.mem_map(STUB, 0x1000)
        jump = bytes.fromhex("ff2500000000")
        stub = (bytes.fromhex("41ffc55048b8") + struct.pack("<Q", 0x18039D08C) +
                bytes.fromhex("f30f1008584585c9750e") +
            jump + struct.pack("<Q", 0x180211D6D) + jump + struct.pack("<Q", 0x180211C5A))
        machine.mem_write(STUB, stub)
        machine.mem_write(START, jump + struct.pack("<Q", STUB) + b"\x90" * 3)
        for address, original, replacement in SITES:
            original_bytes = bytes.fromhex(original)
            if bytes(machine.mem_read(address, len(original_bytes))) != original_bytes:
                raise RuntimeError(f"Unexpected instruction at {address:#x}")
            machine.mem_write(address, bytes.fromhex(replacement))
    previous = (120.0, -240.0, 360.0)
    samples = ((0, 1, -1), (2047, -2048, 2048), (4095, -4096, 3000), (-3000, 123, -456))
    cases = 0
    for values in samples:
        for flags in range(8):
            for offset in (0, 1, 7, 18, 19, 30, 31, 32, 63):
                bits = []
                for axis, value in enumerate(values):
                    present = (flags >> axis) & 1
                    bits.append(present)
                    if present:
                        bits.extend((value >> bit) & 1 for bit in range(width))
                packed = sum(bit << (offset + index) for index, bit in enumerate(bits))
                machine.mem_write(probe.codec.BUF, packed.to_bytes(64, "little"))
                machine.mem_write(DEST, b"\0" * 0x40)
                machine.mem_write(PREVIOUS, b"\0" * 0x40)
                machine.mem_write(PREVIOUS + 0x14, struct.pack("<fff", *previous))
                probe.reset_reader(machine, offset + 1)
                current, remaining = struct.unpack("<II", machine.mem_read(probe.READER + 0x18, 8))
                for register, value in ((UC_X86_REG_R8, probe.READER), (UC_X86_REG_R9, flags & 1),
                                        (UC_X86_REG_RBX, 32), (UC_X86_REG_RCX, remaining),
                                        (UC_X86_REG_RDX, current), (UC_X86_REG_R13, 12),
                                        (UC_X86_REG_R14, DEST), (UC_X86_REG_R15, PREVIOUS),
                                        (UC_X86_REG_RSP, probe.codec.STACK + 0xF0000)):
                    machine.reg_write(register, value)
                machine.emu_start(START, END, count=100000)
                actual = struct.unpack("<fff", machine.mem_read(DEST + 0x14, 12))
                limit = 1 << (width - 1)
                expected = tuple((((values[axis] + limit) % (2 * limit)) - limit) * 8.0
                                 if flags & (1 << axis) else previous[axis] for axis in range(3))
                if actual != expected or probe.reader_bit(machine) != offset + len(bits):
                    raise AssertionError(f"sound reader mismatch: {width=} {values=} {flags=} {offset=} "
                                         f"{actual=} {expected=} end={probe.reader_bit(machine)} "
                                         f"expected_end={offset + len(bits)}")
                cases += 1
    print(f"{width}-bit sound-coordinate reader block: {cases} independent-model cases passed")


def main():
    digest = hashlib.sha256(probe.ENGINE.read_bytes()).hexdigest().upper()
    if digest != probe.SHA256:
        raise RuntimeError(f"Unexpected engine hash: {digest}")
    check_writer(12)
    check_writer(13)
    check(12)
    check(13)


if __name__ == "__main__":
    main()