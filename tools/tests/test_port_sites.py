"""Regression tests for tools/port_sites.py find-time behaviour.

These exercise the porting tool against the preserved analysis copies under work\\ (git-ignored), so
they skip automatically on a fresh clone that lacks the binaries. They lock in the behaviour changed
for the dev-build (26444477) regression case:

  * confidence classification (VERIFIED vs REVIEW) and the new exit codes,
  * occurrence-order (CHECK) and changed-reader matches being flagged REVIEW, not approved,
  * codec data tables recovered by unique content when register reallocation moves the signature,
  * register-moved immediate codec constants recovered inside a content-anchored sibling's function,
  * the refusal path: a changed original byte is reported MISMATCH and fails the run.

Run with either `python -m pytest tools/tests` or `python tools/tests/test_port_sites.py`.
"""
import contextlib
import io
import json
import os
import re
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
TOOLS = os.path.dirname(HERE)
REPO = os.path.dirname(TOOLS)
sys.path.insert(0, TOOLS)
import port_sites as ps  # noqa: E402

X64 = os.path.join(TOOLS, "sites_25375506.json")
X86 = os.path.join(TOOLS, "sites_25375506-x86.json")
VBSP = os.path.join(REPO, "compiler", "sites_vbsppp-80289A94.json")
ANALYSIS = os.environ.get("BIGMAPS_ANALYSIS_ROOT", os.path.join(REPO, "work", "analysis"))
A = lambda *p: os.path.join(ANALYSIS, *p)  # noqa: E731

LINE = re.compile(r"^(VERIFIED|REVIEW|MISMATCH|NOT FOUND)\s+(\S+)\s+\S+\s+.*?via\s+(.*?)(?:\s+in fn.*)?$")


def run_find(root, sites, json_out=None):
    """Run find() capturing stdout. Returns (exit_code, {id: (status, method)}, full_text)."""
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        code = ps.find(root, sites, json_out)
    out = buf.getvalue()
    per = {}
    for ln in out.splitlines():
        m = LINE.match(ln)
        if m:
            per[m.group(2)] = (m.group(1), m.group(3).strip())
    return code, per, out


def have(root, sites):
    return os.path.isdir(root) and os.path.isfile(sites)


class Baseline(unittest.TestCase):
    def test_bundled_reference_metadata(self):
        for target, path in (("gmod", X64), ("gmod-x86", X86), ("vbsppp", VBSP)):
            with self.subTest(target=target), open(path, encoding="utf-8") as f:
                db = json.load(f)
                build, modules, sites = ps.target_set(target)
                self.assertEqual(build, db["build"])
                self.assertEqual(modules, db["modules"])
                self.assertEqual(len(sites), len(db["sites"]))

    @unittest.skipUnless(have(A("25375506"), X64), "x64 analysis copy missing")
    def test_regeneration_preserves_signatures(self):
        references = (("gmod", A("25375506"), X64),
                      ("gmod-x86", A("25375506-x86"), X86),
                      ("vbsppp", A("vbsppp-80289A94"), VBSP))
        with tempfile.TemporaryDirectory() as d:
            for target, root, source in references:
                with self.subTest(target=target):
                    if not have(root, source):
                        continue
                    output = os.path.join(d, target + ".json")
                    with contextlib.redirect_stdout(io.StringIO()):
                        ps.gen(root, output, target)
                    with open(source) as f, open(output) as g:
                        self.assertEqual(json.load(f), json.load(g))

    @unittest.skipUnless(have(A("25375506"), X64), "x64 analysis copy missing")
    def test_x64_self_all_verified(self):
        code, per, out = run_find(A("25375506"), X64)
        self.assertEqual(code, 0)  # clean control: every site strict-verified
        self.assertTrue(all(s == "VERIFIED" for s, _ in per.values()), per)
        self.assertIn("57 verified, 0 need review, 0 not resolved", out)

    @unittest.skipUnless(have(A("25375506-x86"), X86), "x86 analysis copy missing")
    def test_x86_occurrence_is_review(self):
        code, per, out = run_find(A("25375506-x86"), X86)
        # P4-11b/c resolve only by occurrence order; they must be flagged REVIEW, never approved.
        self.assertEqual(per["P4-11b"][0], "REVIEW")
        self.assertEqual(per["P4-11c"][0], "REVIEW")
        self.assertIn("occ", per["P4-11b"][1])
        self.assertEqual(code, 2)  # located but not all verified -> review exit
        verified_block = out.split("REVIEW REQUIRED")[0]
        self.assertNotIn('("P4-11b"', verified_block)  # weak match kept out of the safe block

    @unittest.skipUnless(have(A("vbsppp-80289A94"), VBSP), "vbsppp analysis copy missing")
    def test_vbsppp_all_verified(self):
        code, per, out = run_find(A("vbsppp-80289A94"), VBSP)
        self.assertEqual(code, 0)
        self.assertIn("34 verified, 0 need review, 0 not resolved", out)


class DevRegression(unittest.TestCase):
    @unittest.skipUnless(have(A("26444477"), X64), "dev x64 analysis copy missing")
    def test_x64_recovered_and_reviewed(self):
        code, per, out = run_find(A("26444477"), X64)
        self.assertEqual(code, 2)  # all located, none approved automatically
        self.assertIn("0 not resolved", out)
        for pid in ("P5-04", "P5-06b", "P5-07b"):
            self.assertEqual(per[pid], ("REVIEW", "content"), pid)
        self.assertEqual(per["P5-07a"][0], "REVIEW")
        self.assertIn("sibling", per["P5-07a"][1])
        # shared-literal readers changed -> REVIEW with a changed-reader diagnostic
        for pid in ("P4-03", "P4-04", "P4-13", "P4-14"):
            self.assertEqual(per[pid][0], "REVIEW", pid)
        self.assertIn("reader count", out)
        self.assertIn("added:", out)

    @unittest.skipUnless(have(A("26444477-x86"), X86), "dev x86 analysis copy missing")
    def test_x86_sibling_immediate_recovery(self):
        code, per, out = run_find(A("26444477-x86"), X86)
        self.assertIn("0 not resolved", out)
        for pid in ("P5-06a", "P5-07a"):
            self.assertEqual(per[pid][0], "REVIEW", pid)
            self.assertIn("sibling", per[pid][1])
        self.assertEqual(code, 2)

    @unittest.skipUnless(have(A("26444477"), X64), "dev x64 analysis copy missing")
    def test_json_report(self):
        with tempfile.TemporaryDirectory() as d:
            out_json = os.path.join(d, "r.json")
            run_find(A("26444477"), X64, out_json)
            rep = json.load(open(out_json))
        self.assertEqual(rep["summary"]["failed"], 0)
        self.assertEqual(rep["summary"]["verified"] + rep["summary"]["review"], rep["summary"]["total"])
        p504 = next(s for s in rep["sites"] if s["id"] == "P5-04")
        self.assertEqual(p504["status"], "REVIEW")
        self.assertEqual(p504["method"], "content")
        self.assertTrue(p504["proposed_entry"])


class Refusal(unittest.TestCase):
    @unittest.skipUnless(have(A("25375506"), X64), "x64 analysis copy missing")
    def test_changed_original_byte_is_mismatch(self):
        db = json.load(open(X64))
        for e in db["sites"]:
            if e["id"] == "P5-03":  # a simple code immediate, mov r32, 0xF
                e["orig"] = e["orig"].replace("0F", "EE", 1)  # pretend the original byte changed
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "mut.json")
            json.dump(db, open(path, "w"))
            code, per, out = run_find(A("25375506"), path)
        self.assertEqual(per["P5-03"][0], "MISMATCH")
        self.assertEqual(code, 1)  # a hard failure, not a review
        self.assertNotIn('("P5-03"', out.split("REVIEW REQUIRED")[0])


if __name__ == "__main__":
    unittest.main(verbosity=2)
