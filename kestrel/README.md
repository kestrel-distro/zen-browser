# Kestrel integration

Zen is a Firefox overlay: this repository carries the overlay and a `surfer`
config that fetches the Firefox base (see `surfer.json`), not the browser
source. The Kestrel per-task scheduling hint therefore has to be carried as a
patch applied to the Firefox base at build time, the way Zen carries its own
Firefox changes.

## What the patch does

`scx_kestrel` classifies tasks by a per-task hint the Kestrel kernel stores on
them, set through `sched_setattr(2)` with `SCHED_FLAG_KESTREL_HINT`. Firefox
content processes are separate processes and the session broker cannot set
another process's hint without `CAP_SYS_NICE`, so each process has to hint
itself.

The patch sets:

- hint 3 on the process hosting the focused tab, and
- hint 4 on content processes whose tab is hidden or in the background,

at the same place Firefox already tracks render-layer visibility and process
priority (`ContentParent::SetPriority` and the tab visibility notifications
around `PresShell`/`nsITabParent`). The helper it calls,
`kestrel/kestrel_hint.h`, is header-only and fails harmlessly on a stock
kernel, where the Kestrel broker's cgroup classification remains the only
path.

## Status

The helper and this design are landed. The Firefox patch itself is not yet
authored: it has to be generated against the exact Firefox base pinned in
`surfer.json`, which is not checked out here. `.github/workflows/kestrel-patch-check.yml`
runs `git apply --check` once the patch lands, so a base bump that moves the
integration points fails the build instead of silently misapplying.
