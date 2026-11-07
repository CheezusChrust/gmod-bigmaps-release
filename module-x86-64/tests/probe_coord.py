import hashlib
import math
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import emulate_codec as codec


ENGINE = ROOT / "work/analysis/26439081-x86-64/bin/win64/engine.dll"
SHA256 = "4525B25701DD14EB7C2D4EA49A559797A41BBC07E8D73F631C711F1B809DBDBA"
WRITE_COORD = 0x1802B1FD0
READ_COORD = 0x1802B3180
WRITE_MP = 0x1802B2120
READ_MP = 0x1802B3510
FLOAT_SKIP = 0x18019E7C0
FLOAT_COMPARE = 0x18019E480
WRITE_MASKS = 0x180DD9C90
READER = codec.BF + 0x100
OTHER_READER = codec.BF + 0x180
WIDTH_SITES = (
    (0x18019E8E3, "b80f000000", "b810000000"),
    (0x1802B20E7, "41b80e000000", "41b80f000000"),
    (0x1802B32CE, "83f90e", "83f90f"),
    (0x1802B32D3, "448d49f2", "448d49f1"),
    (0x1802B32DA, "81e3ff3f0000", "81e3ff7f0000"),
    (0x1802B32E9, "41c1e80e", "41c1e80f"),
    (0x1802B3342, "be0e000000", "be0f000000"),
)
MP_WIDTH_SITES = (
    (0x1802B228D, "41b80e000000", "41b80f000000"),
    (0x1802B2336, "41b80e000000", "41b80f000000"),
    (0x1802B3691, "83f90e", "83f90f"),
    (0x1802B369A, "8d41f2", "8d41f1"),
    (0x1802B36A0, "4181e1ff3f0000", "4181e1ff7f0000"),
    (0x1802B36AF, "c1ea0e", "c1ea0f"),
    (0x1802B373B, "bb0e000000", "bb0f000000"),
    (0x1802B3918, "83f90e", "83f90f"),
    (0x1802B391D, "8d41f2", "8d41f1"),
    (0x1802B3923, "4181e1ff3f0000", "4181e1ff7f0000"),
    (0x1802B3932, "c1ea0e", "c1ea0f"),
    (0x1802B393A, "bb0e000000", "bb0f000000"),
)


def coord_bits(value, width):
    value = codec.f32(value)
    integer = math.floor(abs(value))
    fraction = math.floor(abs(value) * 32) & 31
    bits = [int(integer != 0), int(fraction != 0)]
    if integer or fraction:
        bits.append(int(value < 0))
        if integer:
            bits.extend(((integer - 1) >> bit) & 1 for bit in range(width))
        if fraction:
            bits.extend((fraction >> bit) & 1 for bit in range(5))
    return bits


def coord_value(value, width):
    value = codec.f32(value)
    integer = math.floor(abs(value))
    fraction = math.floor(abs(value) * 32) & 31
    decoded = (((integer - 1) & ((1 << width) - 1)) + 1 if integer else 0) + fraction / 32
    return -decoded if value < 0 else decoded


def mp_bits(value, width, mode):
    value = codec.f32(value)
    integer = math.floor(abs(value))
    fraction_width = 3 if mode == 1 else 5
    fraction = math.floor(abs(value) * (1 << fraction_width)) & ((1 << fraction_width) - 1)
    integer_width = 11 if integer < 2048 else width
    bits = [int(integer < 2048), int(integer != 0)]
    if mode != 2 or integer:
        bits.append(int(value < 0))
        if integer:
            bits.extend(((integer - 1) >> bit) & 1 for bit in range(integer_width))
        if mode != 2:
            bits.extend((fraction >> bit) & 1 for bit in range(fraction_width))
    return bits


def mp_value(value, width, mode):
    value = codec.f32(value)
    integer = math.floor(abs(value))
    integer_width = 11 if integer < 2048 else width
    decoded = ((integer - 1) & ((1 << integer_width) - 1)) + 1 if integer else 0
    if mode != 2:
        fraction_width = 3 if mode == 1 else 5
        decoded += (math.floor(abs(value) * (1 << fraction_width)) & ((1 << fraction_width) - 1)) / (1 << fraction_width)
    return -decoded if value < 0 else decoded


def reset_reader(machine, offset, reader=READER):
    word = codec.BUF + (offset // 32) * 4
    shift = offset % 32
    initial = struct.unpack("<I", machine.mem_read(word, 4))[0] >> shift
    machine.mem_write(reader, b"\0" * 0x40)
    machine.mem_write(reader + 0x0C, struct.pack("<IQ", 512, 64))
    machine.mem_write(reader + 0x18, struct.pack("<IIQQQ", initial, 32 - shift, word + 4, codec.BUF + 64, codec.BUF))


def reader_bit(machine, reader=READER):
    remaining, pointer = struct.unpack("<IQ", machine.mem_read(reader + 0x1C, 12))
    return (pointer - codec.BUF) * 8 - remaining


def check_sequence(machine, width, mode):
    values = (0, -0.5, 1, 2047.5, -2048, 16383, -16384, 16385.5, 32767.96875, -32768.96875)
    if width == 14:
        values = values[:7]
    flags = 2 if mode is None else (0x2000, 0x4000, 0x8000)[mode]
    machine.mem_write(codec.PROP + 0x54, struct.pack("<I", flags))
    for offset in (0, 7, 31):
        original = bytes([0xA5] * 64)
        machine.mem_write(codec.BUF, original)
        codec.reset_bf(machine, offset)
        expected = int.from_bytes(original, "little")
        end = offset
        for value in values:
            codec.call(machine, WRITE_COORD if mode is None else WRITE_MP, codec.BF,
                       r8=0 if mode is None else mode, xmm1=value)
            bits = coord_bits(value, width) if mode is None else mp_bits(value, width, mode)
            for index, bit in enumerate(bits, end):
                expected = (expected & ~(1 << index)) | (bit << index)
            end += len(bits)
        if bytes(machine.mem_read(codec.BUF, 64)) != expected.to_bytes(64, "little") or codec.cur_bit(machine) != end:
            raise AssertionError(f"packed writer mismatch: {width=} {mode=} {offset=}")
        for action in ("read", "skip", "compare"):
            reset_reader(machine, offset)
            reset_reader(machine, offset, OTHER_READER)
            end = offset
            for value in values:
                bits = coord_bits(value, width) if mode is None else mp_bits(value, width, mode)
                end += len(bits)
                if action == "read":
                    codec.call(machine, READ_COORD if mode is None else READ_MP, READER,
                               rdx=0 if mode is None else mode)
                    expected_value = coord_value(value, width) if mode is None else mp_value(value, width, mode)
                    if codec.xmm0(machine) != expected_value:
                        raise AssertionError(f"packed reader value mismatch: {value=} {width=} {mode=} {offset=}")
                elif action == "skip":
                    codec.call(machine, FLOAT_SKIP, codec.PROP, rdx=READER)
                else:
                    codec.call(machine, FLOAT_COMPARE, codec.PROP, rdx=READER, r8=OTHER_READER)
                    if machine.reg_read(codec.UC_X86_REG_RAX) != 0 or reader_bit(machine, OTHER_READER) != end:
                        raise AssertionError(f"packed compare mismatch: {value=} {width=} {mode=} {offset=}")
                if reader_bit(machine) != end:
                    raise AssertionError(f"packed {action} position mismatch: {value=} {width=} {mode=} {offset=}")


def check(width, mode=None):
    machine = codec.load(str(ENGINE))
    for offset in range(32):
        for mask_width in range(33):
            mask = ~(((1 << mask_width) - 1) << offset) & 0xFFFFFFFF
            machine.mem_write(WRITE_MASKS + 4 * (offset * 33 + mask_width), struct.pack("<I", mask))
    if width == 15:
        for address, original, replacement in WIDTH_SITES if mode is None else MP_WIDTH_SITES:
            original_bytes = bytes.fromhex(original)
            if bytes(machine.mem_read(address, len(original_bytes))) != original_bytes:
                raise RuntimeError(f"Unexpected instruction at {address:#x}")
            machine.mem_write(address, bytes.fromhex(replacement))
    values = (0, 0.03125, -0.5, 1, -1.25, 2047.96875, 2048, 16383, 16384, 16385.5,
              32767.96875, -32768, -32768.96875)
    cases = 0
    for value in values:
        for offset in (0, 1, 7, 31, 32, 63):
            for byte in (0, 0xA5, 0xFF):
                original = bytes([byte] * 64)
                machine.mem_write(codec.BUF, original)
                codec.reset_bf(machine, offset)
                codec.call(machine, WRITE_COORD if mode is None else WRITE_MP, codec.BF,
                           r8=0 if mode is None else mode, xmm1=value)
                bits = coord_bits(value, width) if mode is None else mp_bits(value, width, mode)
                expected = int.from_bytes(original, "little")
                for index, bit in enumerate(bits, offset):
                    expected = (expected & ~(1 << index)) | (bit << index)
                actual = bytes(machine.mem_read(codec.BUF, len(original)))
                checked = (1 << (offset + len(bits))) - 1
                if (int.from_bytes(actual, "little") & checked != expected & checked or
                        codec.cur_bit(machine) != offset + len(bits)):
                    raise AssertionError(f"writer mismatch: {value=} {offset=} {byte=:#x} "
                                         f"actual={actual[:8].hex()} expected={expected.to_bytes(64, 'little')[:8].hex()} "
                                         f"end={codec.cur_bit(machine)} expected_end={offset + len(bits)}")
                reset_reader(machine, offset)
                codec.call(machine, READ_COORD if mode is None else READ_MP, READER,
                           rdx=0 if mode is None else mode)
                actual_value = codec.xmm0(machine)
                expected_value = coord_value(value, width) if mode is None else mp_value(value, width, mode)
                if actual_value != expected_value or reader_bit(machine) != offset + len(bits):
                    raise AssertionError(f"reader mismatch: {value=} {offset=} {byte=:#x} "
                                         f"actual={actual_value} expected={expected_value} "
                                         f"end={reader_bit(machine)} expected_end={offset + len(bits)}")
                flags = 2 if mode is None else (0x2000, 0x4000, 0x8000)[mode]
                machine.mem_write(codec.PROP + 0x54, struct.pack("<I", flags))
                reset_reader(machine, offset)
                codec.call(machine, FLOAT_SKIP, codec.PROP, rdx=READER)
                if reader_bit(machine) != offset + len(bits):
                    raise AssertionError(f"skip mismatch: {value=} {offset=} {mode=} {width=}")
                reset_reader(machine, offset)
                reset_reader(machine, offset, OTHER_READER)
                codec.call(machine, FLOAT_COMPARE, codec.PROP, rdx=READER, r8=OTHER_READER)
                if (machine.reg_read(codec.UC_X86_REG_RAX) != 0 or
                    reader_bit(machine) != offset + len(bits) or
                    reader_bit(machine, OTHER_READER) != offset + len(bits)):
                    raise AssertionError(f"compare mismatch: {value=} {offset=} {mode=} {width=}")
                cases += 1
    check_sequence(machine, width, mode)
    label = "standard" if mode is None else ("MP", "MP-low", "MP-integer")[mode]
    print(f"{width}-bit {label} writer, reader, skip and compare: {cases} cases and 3 packed sequences passed")


def main():
    digest = hashlib.sha256(ENGINE.read_bytes()).hexdigest().upper()
    if digest != SHA256:
        raise RuntimeError(f"Unexpected engine hash: {digest}")
    check(14)
    check(15)
    for mode in range(3):
        check(14, mode)
        check(15, mode)


if __name__ == "__main__":
    main()