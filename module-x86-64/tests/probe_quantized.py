import hashlib
import random
import struct

import probe_coord as probe


SERVER = probe.ENGINE.parents[2] / "garrysmod/bin/win64/server.dll"
SERVER_SHA256 = "FFFF73124BF46ADD691459A030C3426B940787229D1F3EF9640CE7DFE9BB1F96"
HIGH_LOW_MUL = 0x18032D020
ENCODE = 0x1801A0E10
DECODE = 0x1801A09D0


def main():
    for path, digest in ((probe.ENGINE, probe.SHA256), (SERVER, SERVER_SHA256)):
        if hashlib.sha256(path.read_bytes()).hexdigest().upper() != digest:
            raise RuntimeError(f"Unexpected image hash: {path}")
    engine = probe.codec.load(str(probe.ENGINE))
    server = probe.codec.load(str(SERVER))
    for offset in range(32):
        for width in range(33):
            mask = ~(((1 << width) - 1) << offset) & 0xFFFFFFFF
            engine.mem_write(probe.WRITE_MASKS + 4 * (offset * 33 + width), struct.pack("<I", mask))
    rng = random.Random(26439081)
    for name, bits, limit, flags in (("stock beam", 19, 16384, 0x400), ("wide beam", 20, 32768, 0x400),
                                     ("stock effect", 17, 16384, 0), ("wide effect", 18, 32768, 0)):
        probe.codec.call(server, HIGH_LOW_MUL, bits, xmm1=float(2 * limit))
        multiplier = probe.codec.xmm0(server)
        expected_multiplier = probe.codec.f32(((1 << bits) - 1) / (2 * limit))
        if multiplier != expected_multiplier:
            raise AssertionError(f"{name}: multiplier {multiplier} != {expected_multiplier}")
        engine.mem_write(probe.codec.PROP, b"\0" * 0x80)
        engine.mem_write(probe.codec.PROP + 0x14, struct.pack("<Iff", bits, -float(limit), float(limit)))
        engine.mem_write(probe.codec.PROP + 0x50, struct.pack("<fI", multiplier, flags))
        values = [0.0, -float(limit), float(limit), limit - 0.125, -limit + 0.125, 1.0, -1.0]
        values.extend(probe.codec.f32(rng.uniform(-limit, limit)) for _ in range(250))
        step = 2 * limit / ((1 << bits) - 1)
        cases = 0
        for value in values:
            for offset in (0, 7, 31):
                engine.mem_write(probe.codec.BUF, b"\0" * 64)
                probe.codec.reset_bf(engine, offset)
                probe.codec.call(engine, ENCODE, probe.codec.PROP, r8=probe.codec.BF, xmm1=value)
                if probe.codec.cur_bit(engine) != offset + bits or probe.codec.overflow(engine):
                    raise AssertionError(f"{name}: writer position mismatch at {value=} {offset=}")
                probe.reset_reader(engine, offset)
                probe.codec.call(engine, DECODE, probe.codec.PROP, rdx=probe.READER)
                result = probe.codec.xmm0(engine)
                if probe.reader_bit(engine) != offset + bits or abs(result - value) > step:
                    raise AssertionError(f"{name}: decode mismatch {value=} {result=} {offset=}")
                probe.reset_reader(engine, offset)
                probe.codec.call(engine, probe.FLOAT_SKIP, probe.codec.PROP, rdx=probe.READER)
                if probe.reader_bit(engine) != offset + bits:
                    raise AssertionError(f"{name}: skip mismatch at {value=} {offset=}")
                cases += 1
        print(f"{name}: {cases} multiplier, writer, decoder and skip cases passed")


if __name__ == "__main__":
    main()