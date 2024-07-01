# Cilium eBPF Demos

This repository contains multiple demos showcasing the usage of eBPF with Cilium for Go applications. Each demo is designed to highlight different aspects of eBPF and its interaction with Go applications, including uProbes, uRetprobes, and function execution time tracking.

## Table of Contents
1. [Overview](#overview)
2. [Navigating the Demos](#navigating-the-demos)
3. [Useful Resources](#useful-resources)

## Overview
Each folder within this repository contains a specific demo illustrating a unique feature of eBPF and its integration with Go applications. The demos include:
- [Using uProbes with eBPF](uprobe/)
- [Tracking Function Execution Time with eBPF](uprobe-time/)
- [Understanding Potential Crashes Caused by uRetprobes in Go Applications](uretprobe-failure/)

For more details on each demo, please refer to the README file within the respective demo folder.

## Useful Resources
Here are some useful resources to understand eBPF, Go, and their integration:
- [Cilium eBPF Documentation](https://docs.cilium.io/en/stable/bpf/)
- [eBPF Tracing and uProbes](https://www.kernel.org/doc/html/latest/bpf/bpf_devel_QA.html#why-my-uretprobe-program-crashes)
- [Introduction to eBPF](https://ebpf.io/what-is-ebpf/)

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.


Links:

- [Great intro](https://medium.com/bumble-tech/bpf-and-go-modern-forms-of-introspection-in-linux-6b9802682223)
- [Talk from Liz Rice](https://www.youtube.com/watch?v=Hed2DOrk_kk)
- [Pixie demo](https://github.com/pixie-io/pixie-demos/tree/main/simple-gotracing)
- [Cilium eBPF](https://github.com/cilium/ebpf)