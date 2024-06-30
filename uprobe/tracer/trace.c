#define __TARGET_ARCH_x86

#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

char __license[] SEC("license") = "Dual MIT/GPL";

// Define a struct for the event
struct event {
    u32 arg;
};

// Define a BPF ring buffer map to store events
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 1 << 24);
} events SEC(".maps");

// Force emitting struct event into the ELF.
const struct event *unused __attribute__((unused));

// Define the uprobe section
SEC("uprobe/go_demo")
int uprobe_DemoFunc(struct pt_regs *ctx) {
    struct event *e;

    e = bpf_ringbuf_reserve(&events, sizeof(struct event), 0);
    if (!e) {
        return 0;
    }

    // Read the return value of the traced function into the event argument
    bpf_probe_read(&e->arg, sizeof(e->arg), &PT_REGS_RC(ctx));
    // Print the argument value for debugging
    bpf_printk("arg --->>>> %d\n", e->arg);

    // Submit the event to the ring buffer
    bpf_ringbuf_submit(e, 0);

    return 0;
}