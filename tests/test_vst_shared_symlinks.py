from __future__ import annotations

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
VST_SRC = ROOT / "vst" / "src"


def test_vst_categories_link_shared_headers():
    for category in ("racks", "pedals", "amps"):
        shared = VST_SRC / category / "_shared"

        assert shared.is_symlink()
        assert shared.readlink() == Path("../_shared")
        assert shared.resolve() == VST_SRC / "_shared"
