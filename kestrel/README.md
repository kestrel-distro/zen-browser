# Kestrel integration

Zen is a Firefox overlay: this repository carries the overlay and a `surfer`
config that fetches the Firefox base (`surfer.json` pins Firefox 158.0.1), not
the browser source. The Kestrel per-task scheduling hint is therefore carried as
a patch applied to that base, `kestrel/firefox-kestrel-hint.patch`, generated
against the `FIREFOX_158_0_1_RELEASE` tag.

## What the patch does

`scx_kestrel` classifies tasks by a per-task hint the Kestrel kernel stores on
them (`sched_attr::sched_ext_hint`, `SCHED_FLAG_KESTREL_HINT`). The content
process hints itself: `ContentChild::RecvNotifyProcessPriorityChanged` already
runs there with the foreground/background decision, so the patch sets hint 3 for
foreground and 4 for background. It adds `dom/ipc/kestrel/KestrelHint.h`, a
header-only helper that fails harmlessly on a stock kernel.

The patch covers threads that exist when the priority changes; Firefox re-sends
the priority on tab focus/visibility changes, so render and worker threads
created later are covered on the next transition.

## What it deliberately does not do

The chrome/parent process is left unhinted. It has one long-lived priority and
would otherwise be pinned to browser-active forever; the broker's cgroup
classification (`kestrel-browser.slice`) is the right lever for it. The broker
also moves the browser process tree between the active and background buckets,
which is the slice part of the design.

A same-UID process can set another task's hint (`sched_setattr` keeps the
target's params with `SCHED_FLAG_KEEP_PARAMS`), but hinting from the parent is
still avoided: the hint space is class-valued, and only the process itself knows
which class it is.

An equivalent, Kestrel-owned copy of the patch is committed on the
`kestrel/hint` branch of `kestrel-distro/firefox`.

## Updating

Regenerate the patch against the Firefox tag `surfer.json` pins, then update
`FIREFOX_TAG` in `.github/workflows/kestrel-patch-check.yml`. That workflow
clones the base blobless, sparse-checks out `dom/ipc`, and runs
`git apply --check`, so a base bump that moves the integration point fails
loudly instead of misapplying.
