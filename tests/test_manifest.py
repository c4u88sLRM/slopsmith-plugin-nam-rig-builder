from __future__ import annotations

import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def _manifest() -> dict:
    return json.loads((ROOT / "plugin.json").read_text())


def test_manifest_declares_capability_standards():
    manifest = _manifest()

    assert "capability-pipelines.v1" in manifest["standards"]
    assert "plugin-runtime-idempotent.v1" in manifest["standards"]


def test_manifest_declares_playback_observer_for_008():
    playback = _manifest()["capabilities"]["playback"]

    assert playback["roles"] == ["observer"]
    assert playback["kind"] == "lifecycle"
    assert playback["observes"] == ["ready", "stopped", "ended"]
    assert playback["compatibility"] == "shim-allowed"
    assert playback["ownership"] == "observer-only"
    assert playback["safety"] == "safe"
    assert playback["version"] == 1


def test_manifest_keeps_audio_effects_jobs_and_privileged_surfaces():
    capabilities = _manifest()["capabilities"]

    assert capabilities["audio-effects"]["roles"] == ["provider", "requester", "observer"]
    assert "select-chain" in capabilities["audio-effects"]["commands"]
    assert capabilities["jobs"]["roles"] == ["provider", "observer"]
    assert "job.enqueue" in capabilities["jobs"]["operations"]
    assert capabilities["privileged-capabilities"]["roles"] == ["provider", "requester", "observer"]
    assert "check-approval-boundary" in capabilities["privileged-capabilities"]["requests"]
