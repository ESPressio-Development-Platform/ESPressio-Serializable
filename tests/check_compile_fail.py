"""Verify intentional P3 compile-time diagnostics, rather than accepting arbitrary compiler failure."""
import pathlib
import subprocess
import sys
import tempfile

root = pathlib.Path(__file__).resolve().parents[1]
compiler = sys.argv[1] if len(sys.argv) > 1 else "c++"
cases = {
    "unbounded_field.cpp": "explicit nonzero schema version and a bounded property graph",
    "missing_version.cpp": "explicit nonzero schema version and a bounded property graph",
    "unsupported_format.cpp": "No bounded encoded-size proof for selected format",
}
with tempfile.TemporaryDirectory() as directory:
    for source, diagnostic in cases.items():
        result = subprocess.run(
            [compiler, "-std=c++17", "-I" + str(root / "src"), "-c",
             str(root / "tests" / "compile_fail" / source), "-o", str(pathlib.Path(directory) / "case.o")],
            capture_output=True, text=True,
        )
        if result.returncode == 0 or diagnostic not in result.stderr:
            raise SystemExit(f"Unexpected compile result for {source}:\n{result.stderr}")
        print(f"PASS {source}: intended diagnostic")
