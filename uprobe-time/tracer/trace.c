#define __TARGET_ARCH_x86

#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

char __license[] SEC("license") = "Dual MIT/GPL";

struct event {
    u32 pid;
    u64 start_time;
    u64 end_time;
};

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 4096); // Increased map size
    __type(key, u32);
    __type(value, u64);
} start_times SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 1 << 24);
} events SEC(".maps");

// Force emitting struct event into the ELF.
const struct event *unused __attribute__((unused));

SEC("uprobe/entryMarker")
int uprobe_entryMarker(struct pt_regs *ctx) {
    u32 pid = bpf_get_current_pid_tgid() >> 32; // Extract the PID
    u64 start_time = bpf_ktime_get_ns();
    int ret = bpf_map_update_elem(&start_times, &pid, &start_time, BPF_ANY);
    if (ret != 0) {
        bpf_printk("uprobe_entryMarker: failed to store start time for pid %u\n", pid);
    } else {
        bpf_printk("uprobe_entryMarker: start time stored for pid %u\n", pid);
    }
    return 0;
}

SEC("uprobe/exitMarker")
int uprobe_exitMarker(struct pt_regs *ctx) {
    u32 pid = bpf_get_current_pid_tgid() >> 32; // Extract the PID
    u64 *start_time = bpf_map_lookup_elem(&start_times, &pid);
    if (!start_time) {
        bpf_printk("uprobe_exitMarker: start time not found for pid %u\n", pid);
        return 0;
    }

    struct event *e = bpf_ringbuf_reserve(&events, sizeof(*e), 0);
    if (!e) {
        bpf_printk("uprobe_exitMarker: failed to reserve ringbuf space for pid %u\n", pid);
        return 0;
    }

    e->pid = pid;
    e->start_time = *start_time;
    e->end_time = bpf_ktime_get_ns();
    bpf_ringbuf_submit(e, 0);
    bpf_map_delete_elem(&start_times, &pid);
    bpf_printk("uprobe_exitMarker: event submitted for pid %u\n", pid);
    return 0;
}
