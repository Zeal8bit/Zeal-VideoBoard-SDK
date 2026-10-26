"""Build checks run inside ZDE: pytest tests/test_build.py -v --junitxml=junit/build.xml."""

from pathlib import Path
import os
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[1]
LIBRARIES = sorted(
    subprocess.check_output(
        ["git", "ls-files", "lib/*.lib"], cwd=ROOT, text=True
    ).splitlines()
)
EXAMPLES = sorted(path for path in (ROOT / "examples").iterdir() if path.is_dir())


def run(*args, cwd=ROOT):
    subprocess.run(args, cwd=cwd, check=True, timeout=300)


@pytest.fixture(scope="session")
def sdk(tmp_path_factory):
    """Snapshot committed archives, then force a fresh build of every library."""
    assert LIBRARIES, "No committed SDK libraries found"
    assert EXAMPLES, "No examples found"
    os.environ["ZVB_SDK_PATH"] = str(ROOT)
    baseline = tmp_path_factory.mktemp("committed-libraries")
    for name in LIBRARIES:
        data = subprocess.check_output(["git", "show", f"HEAD:{name}"], cwd=ROOT)
        (baseline / Path(name).name).write_bytes(data)

    # The SDK writes archives to lib/ regardless of its CMake build directory.
    # Remove old archives so a removed target cannot pass using a stale file.
    for archive in (ROOT / "lib").glob("*.lib"):
        archive.unlink()
    build = ROOT / "build" / "ci-sdk"
    run("cmake", "-S", str(ROOT), "-B", str(build))
    run("cmake", "--build", str(build), "--clean-first")
    return baseline


def archive_contents(path):
    """Compare ar entry names/order/bytes, including duplicates and indexes.

    Only header timestamps, UID/GID and file modes are ignored. GNU long-name
    tables and symbol indexes remain part of the comparison.
    """
    data = path.read_bytes()
    assert data.startswith(b"!<arch>\n"), f"Invalid archive: {path}"
    offset = 8
    members = []
    while offset < len(data):
        header = data[offset:offset + 60]
        assert len(header) == 60 and header[58:] == b"`\n", f"Invalid header: {path}"
        size = int(header[48:58])
        offset += 60
        assert 0 <= size <= len(data) - offset, f"Invalid member size: {path}"
        members.append((header[:16].rstrip(), data[offset:offset + size]))
        offset += size + size % 2
    assert members, f"Empty archive: {path}"
    return members


def test_sdk_build(sdk):
    expected = {Path(name).name for name in LIBRARIES}
    generated = {path.name for path in (ROOT / "lib").glob("*.lib")}
    assert generated == expected, f"Library set changed: {generated ^ expected}"


@pytest.mark.parametrize("library", LIBRARIES, ids=lambda name: Path(name).name)
def test_committed_library_matches_build(sdk, library):
    rebuilt = ROOT / library
    assert rebuilt.is_file(), f"Library was not generated: {library}"
    if archive_contents(rebuilt) != archive_contents(sdk / rebuilt.name):
        pytest.fail(
            f"{library} differs from the fresh build. Rebuild the SDK using the CI "
            "toolchain and commit the updated lib/*.lib files (git add -f if needed)."
        )


@pytest.mark.parametrize("example", EXAMPLES, ids=lambda path: path.name)
def test_example_build(sdk, example):
    # Use each example's supported build system. New examples are discovered
    # automatically, and directories without a build definition fail visibly.
    if (example / "CMakeLists.txt").is_file():
        build = ROOT / "build" / "ci-examples" / example.name
        run("cmake", "-S", str(example), "-B", str(build))
        run("cmake", "--build", str(build), "--clean-first")
    else:
        assert (example / "Makefile").is_file(), f"No build definition: {example}"
        run("make", "-B", cwd=example)
