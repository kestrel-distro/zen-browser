#pragma once

/*
 * Kestrel per-task scheduling hint, header-only. Sets the hint the Kestrel
 * kernel stores on a task (sched_attr::sched_ext_hint, SCHED_FLAG_KESTREL_HINT)
 * so scx_kestrel can classify it. A stock kernel rejects the flag, so every
 * call fails harmlessly and reports unsupported.
 *
 * Keep the ABI in sync with kestrel-distro/kestrel
 * install/helpers/libkestrel-hint/kestrel_hint.h and the kernel patch.
 */

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <dirent.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

namespace KestrelHint {

inline constexpr uint64_t FLAG        = 1ULL << 40; // SCHED_FLAG_KESTREL_HINT
inline constexpr uint64_t KEEP_POLICY = 0x08ULL;
inline constexpr uint64_t KEEP_PARAMS = 0x10ULL;

struct SchedAttrExt {
    uint32_t size;
    uint32_t sched_policy;
    uint64_t sched_flags;
    int32_t  sched_nice;
    uint32_t sched_priority;
    uint64_t sched_runtime;
    uint64_t sched_deadline;
    uint64_t sched_period;
    uint32_t sched_util_min;
    uint32_t sched_util_max;
    uint32_t sched_ext_hint;
};

inline int setTid(pid_t tid, uint32_t hint) {
    SchedAttrExt a{};
    a.size           = sizeof(a);
    a.sched_flags    = FLAG | KEEP_POLICY | KEEP_PARAMS;
    a.sched_ext_hint = hint;
    return static_cast<int>(syscall(SYS_sched_setattr, tid, &a, 0u));
}

inline int setProcess(uint32_t hint) {
    DIR* dir = opendir("/proc/self/task");
    if (!dir)
        return -1;
    int rc = 0;
    while (dirent* e = readdir(dir)) {
        if (e->d_name[0] == '.')
            continue;
        pid_t tid = static_cast<pid_t>(atoi(e->d_name));
        if (tid > 0 && setTid(tid, hint) != 0)
            rc = -1;
    }
    closedir(dir);
    return rc;
}

} // namespace KestrelHint
