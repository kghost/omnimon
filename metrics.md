## Source of Metrics

Overall uptime, /proc/uptime
Overall load, /proc/loadavg
Overall scheduler stat, /proc/schedstat
Overall CPU stat, /proc/stat
Overall memory, /proc/meminfo
Overall memory, /proc/vmstat
Overall pressure, /proc/pressure/*
Overall Slab stat, /proc/slabinfo, need root
Overall disk stat, /proc/diskstats

Overall interrupt stat, /proc/interrupts
Overall SoftIRQ stat, /proc/softirqs

Overall network stat, /proc/net/netstat

Per-process stat, /proc/[pid]
Per-process stat alternative, netlink taskstats
Per-process/Per-thread stat, /proc/[pid]/task/[tid]
Per-process file descriptors, /proc/[pid]/fd/

Per-CGroup stat, /sys/fs/cgroup/[tree]

Network interfaces, RTNETLINK

Network connection tracking, RTNETLINK netfilter nf_conntrack, /proc/sys/net/netfilter/nf_conntrack_*
