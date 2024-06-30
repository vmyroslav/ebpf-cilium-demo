all: clean download generate build

generate:
	go generate

download:
	go mod download

build:
	go build -o trace main.go bpf_bpfel.go

clean:
	rm -f app

run:
	sudo ./trace

# read debug
debug:
	sudo cat /sys/kernel/debug/tracing/trace_pipe