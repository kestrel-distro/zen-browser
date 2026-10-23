# Kestrel integration

Zen is a Firefox overlay: this repository carries the overlay and a `surfer`
config that fetches the Firefox base (`surfer.json` pins Firefox 158.0.1), not
the browser source. The Kestrel per-task scheduling hint is therefore carried as
a patch applied to that base, `kestrel/firefox-kestrel-hint.patch`, generated
against the `FIREFOX_158_0_1_RELEASE` tag.

## What the patch does

`scx_kestrel` classifies tasks by a per-task hint the Kestrel kernel stores on
them (`sched_attr::sched_ext_hint`, `SCHED_FLAG_KESTREL_HINT`). A content
process has to hint itself: the parent cannot set another process's hint
without `CAP_SYS_NICE`, and the session broker does not run inside the content
process.

The patch hooks `ContentChild::RecvNotifyProcessPriorityChanged`, which already
runs in the content process with the foreground/background decision, and sets
hint 3 for foreground and 4 for background. It adds
`dom/ipc/kestrel/KestrelHint.h`, a header-only helper that fails harmlessly on a
stock kernel, where the Kestrel broker's cgroup classification remains the only
path.

An equivalent, Kestrel-owned copy of the patch is committed on the
`kestrel/hint` branch of `kestrel-distro/firefox`.

## Updating

Regenerate the patch against the Firefox tag `surfer.json` pins, then update
`FIREFOX_TAG` in `.github/workflows/kestrel-patch-check.yml`. That workflow
clones the base blobless, sparse-checks out `dom/ipc`, and runs
`git apply --check`, so a base bump that moves the integration point fails
loudly instead of misapplying.
