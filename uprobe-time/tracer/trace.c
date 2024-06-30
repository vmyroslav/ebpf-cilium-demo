#define __TARGET_ARCH_x86

#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

char __license[] SEC("license") = "Dual MIT/GPL";

// Define the event structure
struct event {
    u32 pid;          // Process ID
    u64 start_time;   // Start time of the event
    u64 end_time;     // End time of the event
};

// Define a hash map to store the start times of events
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 4096);
    __type(key, u32);
    __type(value, u64);
} start_times SEC(".maps");

// Define a ring buffer to store the events
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 1 << 24);
} events SEC(".maps");

// Force emitting struct event into the ELF.
const struct event *unused __attribute__((unused));

// Define the function to be called at the entry of the target function
SEC("uprobe/entryMarker")
int uprobe_entryMarker(struct pt_regs *ctx) {
    u32 pid = bpf_get_current_pid_tgid() >> 32; // Extract the PID
    u64 start_time = bpf_ktime_get_ns();        // Get the current time

    // Store the start time in the start_times map
    int ret = bpf_map_update_elem(&start_times, &pid, &start_time, BPF_ANY);
    if (ret != 0) {
        bpf_printk("uprobe_entryMarker: failed to store start time for pid %u\n", pid);
    } else {
        bpf_printk("uprobe_entryMarker: start time stored for pid %u\n", pid);
    }
    return 0;
}

// Define the function to be called at the exit of the target function
SEC("uprobe/exitMarker")
int uprobe_exitMarker(struct pt_regs *ctx) {
    u32 pid = bpf_get_current_pid_tgid() >> 32; // Extract the PID
    // Retrieve the start time from the start_times map
    u64 *start_time = bpf_map_lookup_elem(&start_times, &pid);
    if (!start_time) {
        bpf_printk("uprobe_exitMarker: start time not found for pid %u\n", pid);
        return 0;
    }

    // Reserve space in the ring buffer for the event
    struct event *e = bpf_ringbuf_reserve(&events, sizeof(*e), 0);
    if (!e) {
        bpf_printk("uprobe_exitMarker: failed to reserve ringbuf space for pid %u\n", pid);
        return 0;
    }

    // Fill the event structure
    e->pid = pid;
    e->start_time = *start_time;
    e->end_time = bpf_ktime_get_ns();

    // Submit the event to the ring buffer
    bpf_ringbuf_submit(e, 0);

    // Delete the start time from the start_times map
    bpf_map_delete_elem(&start_times, &pid);
    bpf_printk("uprobe_exitMarker: event submitted for pid %u\n", pid);

    return 0;
 }