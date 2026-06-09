# Bug report — Slopsmith 0.2.9 crashes when loading any bundled VST3 (sandbox subsystem)

**Severity:** critical — every bundled VST3 (amps/pedals/racks) crashes the whole app on load, so no Rig Builder tone using a bundled VST plays.

**Build:** Slopsmith 0.2.9 (osx-arm64-stable), Apple Silicon, macOS (Darwin 25.3).
**Regression:** worked on the previous version (VSTs loaded in-process). Reproduces on a clean Developer-ID 0.2.9 install — **independent of code signing** (confirmed: happens before/after an ad-hoc re-sign, and with the sandbox child signed Developer-ID or ad-hoc).

## Symptom
Loading any bundled VST3 via `LoadVST` aborts the process:
```
libc++abi: terminating due to uncaught exception of type
  std::__1::system_error: mutex lock failed: Invalid argument
  (also seen: thread::join failed: Invalid argument)
```
SIGABRT, exit 134. The whole app dies (in-process abort), so the per-plugin crash guard never gets to record it.

## Crash backtrace (thread #1, CrBrowserMain)
```
__clang_call_terminate
slopsmith::sandbox::ControlChannel::~ControlChannel() + 348
slopsmith::sandbox::SandboxedProcessor::~SandboxedProcessor() + 228
slopsmith::sandbox::SandboxedProcessor::spawn(SpawnConfig const&, juce::String&) + 564
slopsmith::sandbox::tryLoadSandboxed(juce::PluginDescription const&, double, int, juce::String&) + 444
loadVstSandboxAware(juce::String const&, double, int, juce::String&, bool&) + 340
LoadVST(Napi::CallbackInfo const&) + 544
```

## What SLOPSMITH_SANDBOX_DEBUG=1 shows
```
[vst-trace] shouldSandbox: <plugin>.vst3 — default policy (every VST3 sandboxes)
[vst-trace] SubprocessHandle.startPosix: posix_spawn '.../slopsmith-vst-host' (3 inherited fds)
[vst-trace] SubprocessHandle.startPosix: spawned pid=35858        <- spawn SUCCEEDS
[vst-trace] [ctrl] readFrame: len read failed rc=-3 errno=0
[vst-trace] [ctrl] readFrame failed; exiting loop (peerClosed=1 err=0)
# => "sandbox handshake failed before ready (subprocess exit or control-pipe disconnect)"
```

## Root cause — three compounding bugs
1. **Policy: every VST3 is force-sandboxed** (`shouldSandbox … default policy (every VST3 sandboxes)`). There is no in-process path and no env/setting to opt out. So a broken sandbox = nothing loads. The previous version loaded in-process and worked.
2. **The parent does NOT pass the audio-shm fd to the sandbox child** — this is the concrete failure. Instrumented by replacing `slopsmith-vst-host` with a wrapper that `stat`s each inherited fd. The child is spawned with args `--control-fd 3 --audio-evt-fd 4 --audio-shm-fd 5`, but only fds 3 and 4 actually arrive:
   ```
   fd3 OPEN type=Socket     (control)   OK
   fd4 OPEN type=Socket     (evt)       OK
   fd5 MISSING/bad          (audio-shm) <-- never inherited
   ```
   The child loads the plugin fine (`createPluginInstance … instance=OK`), enters the control loop, and reads the 85-byte config frame — then can't map the audio shared memory (fd 5 is closed), never signals `ready`, so the parent's spawn fails. The parent log says "(3 inherited fds)" but only 2 reach the child → an off-by-one / missing `posix_spawn_file_actions_adddup2` (or a CLOEXEC not cleared) for the shm fd in `slopsmith_audio.node`'s spawn setup. **Fix: ensure the audio-shm fd is actually inherited at fd 5 (add the dup2 file-action / clear FD_CLOEXEC).** No client-side workaround is possible — a fd the parent never hands off cannot be recovered downstream.
3. **The failure path crashes the whole app.** When `spawn` fails, `~SandboxedProcessor`/`~ControlChannel` run on a partially-initialized object and call `std::thread::join()` / `std::mutex::lock()` on members that were never started/initialized → `std::system_error (EINVAL)` thrown from a destructor → uncaught → `std::terminate` → abort.

## Suggested fixes (any one stops the app-kill; all three for a real fix)
- **Bug 3 (quick):** make `~ControlChannel` / `~SandboxedProcessor` tolerate a never-started state — guard `thread.join()` with `joinable()`, don't lock/destroy an uninitialized mutex, and never throw from a destructor. This alone turns the crash into a graceful "sandbox load failed".
- **Bug 2 (real fix):** debug why `slopsmith-vst-host` exits before `ready` in full host mode on this build/OS (control-fd handshake, shm magic/size, or inherited-fd numbering). `[ctrl] protocol version mismatch` / `audio shm magic mismatch` paths are the suspects.
- **Bug 1 (resilience):** if the sandbox spawn fails, fall back to in-process load (the old behavior) instead of dead-ending, or make the "every VST3 sandboxes" policy configurable.

## Repro
1. Open Rig Builder, audition/edit any bundled VST3 (e.g. `vst/amps/AidenGT300.vst3`).
2. App aborts with the trace above. `SLOPSMITH_SANDBOX_DEBUG=1` prints the sandbox trace.

## Note (separate issue, already worked around locally)
0.2.9's app bundle is missing `com.apple.security.device.audio-input` in its macOS entitlements, so mic/instrument input is dead until the app is re-signed with it. Please add it to the build's `mac.entitlements` (and `mac.entitlementsInherit` for the Renderer helper). See `reference_mic_entitlement_fix`.
