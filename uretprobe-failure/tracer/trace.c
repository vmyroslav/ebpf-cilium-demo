#define __TARGET_ARCH_x86

#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

// Define the license for this BPF program
char __license[] SEC("license") = "Dual MIT/GPL";

// Define the structure of the event
struct event {
    u32 arg;
    u64 exec_time_ns;
};

// Define a ring buffer for storing events
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 1 << 24);
} events SEC(".maps");

// Force emitting struct event into the ELF.
const struct event *unused __attribute__((unused));

// Define a hash map for storing start times of function calls
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 1024);
    __type(key, u64);
    __type(value, u64);
} start_times SEC(".maps");

// Define a uprobe for the function "go_demo"
SEC("uprobe/go_demo")
int uprobe_DemoFunc(struct pt_regs *ctx) {
    // Get the current process ID and thread group ID
    u64 pid_tgid = bpf_get_current_pid_tgid();
    // Get the current kernel time
    u64 start_time = bpf_ktime_get_ns();

    // Store the start time in the start_times map
    bpf_map_update_elem(&start_times, &pid_tgid, &start_time, BPF_ANY);
    bpf_printk("uprobe: start_time stored for pid_tgid %llu\n", pid_tgid);

    return 0;
}

// Define a uretprobe for the function "go_demo"
SEC("uretprobe/go_demo")
int uretprobe_DemoFunc(struct pt_regs *ctx) {
    // Get the current process ID and thread group ID
    u64 pid_tgid = bpf_get_current_pid_tgid();
    // Look up the start time in the start_times map
    u64 *start_time = bpf_map_lookup_elem(&start_times, &pid_tgid);
    // Print a debug message
    bpf_printk("uretprobe: function exit for pid_tgid %llu\n", pid_tgid);

    // If the start time was not found, return early
    if (!start_time) {
        bpf_printk("uretprobe: start_time not found for pid_tgid %llu\n", pid_tgid);
        return 0;
    }

    // Reserve space in the ring buffer for an event
    struct event *e;
    e = bpf_ringbuf_reserve(&events, sizeof(struct event), 0);
    if (!e) {
        bpf_printk("uretprobe: ringbuf reserve failed for pid_tgid %llu\n", pid_tgid);
        return 0;
    }

    // Calculate the execution time
    u64 end_time = bpf_ktime_get_ns();
    e->exec_time_ns = end_time - *start_time;

    // Delete the start time from the start_times map
    bpf_map_delete_elem(&start_times, &pid_tgid);

    // Read the return value of the function
    bpf_probe_read(&e->arg, sizeof(e->arg), &PT_REGS_RC(ctx));
    bpf_printk("uretprobe: arg %d, exec_time %llu ns\n", e->arg, e->exec_time_ns);

    // Submit the event to the ring buffer
    bpf_ringbuf_submit(e, 0);

    return 0;
}