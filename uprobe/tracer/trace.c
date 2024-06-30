#define __TARGET_ARCH_x86

#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

char __license[] SEC("license") = "Dual MIT/GPL";

struct event {
    u32 arg;
};

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 1 << 24);
} events SEC(".maps");

// Force emitting struct event into the ELF.
const struct event *unused __attribute__((unused));

SEC("uprobe/go_demo")
int uprobe_DemoFunc(struct pt_regs *ctx) {
    struct event *e;

    e = bpf_ringbuf_reserve(&events, sizeof(struct event), 0);
    if (!e) {
        bpf_printk("params --->>>> %d\n", 22);

        return 0;
    }

    bpf_probe_read(&e->arg, sizeof(e->arg), &PT_REGS_RC(ctx));
    bpf_printk("param --->>>> %d\n", e->arg);

    bpf_ringbuf_submit(e, 0);

    return 0;
}