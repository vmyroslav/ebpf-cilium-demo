package main

import (
	"bytes"
	"encoding/binary"
	"errors"
	"flag"
	"fmt"
	"log"
	"os"
	"os/signal"
	"syscall"

	"github.com/cilium/ebpf/link"
	"github.com/cilium/ebpf/ringbuf"
	"github.com/cilium/ebpf/rlimit"
)

// Generate BPF code from trace.c
//go:generate go run github.com/cilium/ebpf/cmd/bpf2go -type event bpf trace.c

var (
	binaryProg string
	traceFn    string
)

// Initialize command line flags
func init() {
	flag.StringVar(&binaryProg, "binary", "./../bin/uprobe_app", "The path to the app binary to trace")
	flag.StringVar(&traceFn, "func", "main.demo", "The function to trace")
}

// Main function
func main() {
	// Create a channel to receive OS signals
	stopCh := make(chan os.Signal, 1)
	signal.Notify(stopCh, syscall.SIGKILL, syscall.SIGTERM)

	// Remove memory lock limit
	if err := rlimit.RemoveMemlock(); err != nil {
		log.Fatal(err)
	}

	pwd, err := os.Getwd()
	if err != nil {
		fmt.Println("Error:", err)
		return
	}
	fmt.Println("Current working directory:", pwd)

	// Load BPF objects
	objs := bpfObjects{}
	if err := loadBpfObjects(&objs, nil); err != nil {
		log.Fatalf("loading objects: %v", err)
	}
	defer objs.Close()

	// Open executable for uprobe
	ex, err := link.OpenExecutable(binaryProg)
	if err != nil {
		log.Fatalf("failed to open executable: %s", err)
	}

	// Create uprobe
	uprobe, err := ex.Uprobe(traceFn, objs.UprobeDemoFunc, nil)
	if err != nil {
		log.Fatalf("failed to create uprobe: %s", err)
	}
	defer uprobe.Close()

	// Create ringbuf reader
	r, err := ringbuf.NewReader(objs.bpfMaps.Events)
	if err != nil {
		log.Fatalf("failed to create ringbuf reader: %s", err)
	}
	defer r.Close()

	// Wait for signals to stop the program
	go func() {
		<-stopCh

		if err = r.Close(); err != nil {
			log.Fatalf("closing ringbuf reader: %s", err)
		}
	}()

	log.Println("Waiting for BPF events...")

	var event bpfEvent

	// Read events from ringbuf
	for {
		record, err := r.Read()
		if err != nil {
			if errors.Is(err, ringbuf.ErrClosed) {
				log.Println("ringbuf closed")
				return
			}

			log.Printf("error reading ringbuf: %s", err)
			continue
		}

		// Parse the ringbuf event entry into a bpfEvent structure
		if err := binary.Read(bytes.NewBuffer(record.RawSample), binary.LittleEndian, &event); err != nil {
			log.Printf("failed to parse event: %s", err)
			continue
		}

		log.Printf(
			ColorGreen+"the trace function %s"+ColorReset+" was called with "+ColorYellow+"%d"+ColorReset+" argument\n",
			traceFn,
			event.Arg,
		)
	}
}

const (
	ColorReset  = "\033[0m"
	ColorGreen  = "\033[32m"
	ColorYellow = "\033[33m"
)
