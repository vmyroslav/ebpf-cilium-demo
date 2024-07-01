package main

import (
	"bytes"
	"encoding/binary"
	"errors"
	"flag"
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/cilium/ebpf/link"
	"github.com/cilium/ebpf/ringbuf"
	"github.com/cilium/ebpf/rlimit"
)

//go:generate go run github.com/cilium/ebpf/cmd/bpf2go -type event bpf trace.c

var (
	binaryProg string
	entryFn    string
	exitFn     string
)

func init() {
	flag.StringVar(&binaryProg, "binary", "./../bin/uprobe_time_app", "The path to the app binary to trace")
	flag.StringVar(&entryFn, "entry", "main.entryMarker", "The entry function to trace")
	flag.StringVar(&exitFn, "exit", "main.exitMarker", "The exit function to trace")
}

func main() {
	stopCh := make(chan os.Signal, 1)
	signal.Notify(stopCh, syscall.SIGKILL, syscall.SIGTERM)

	if err := rlimit.RemoveMemlock(); err != nil {
		log.Fatal(err)
	}

	objs := bpfObjects{}
	if err := loadBpfObjects(&objs, nil); err != nil {
		log.Fatalf("loading objects: %v", err)
	}
	defer objs.Close()

	ex, err := link.OpenExecutable(binaryProg)
	if err != nil {
		log.Fatalf("failed to open uprobe executable: %s", err)
	}

	entryProbe, err := ex.Uprobe(entryFn, objs.UprobeEntryMarker, nil)
	if err != nil {
		log.Fatalf("failed to create entry uprobe: %s", err)
	}
	defer entryProbe.Close()

	exitProbe, err := ex.Uprobe(exitFn, objs.UprobeExitMarker, nil)
	if err != nil {
		log.Fatalf("creating exit uprobe: %s", err)
	}
	defer exitProbe.Close()

	r, err := ringbuf.NewReader(objs.bpfMaps.Events)
	if err != nil {
		log.Fatalf("opening ringbuf reader: %s", err)
	}
	defer r.Close()

	go func() {
		<-stopCh

		if err := r.Close(); err != nil {
			log.Fatalf("closing ringbuf reader: %s", err)
		}
	}()

	log.Println("Waiting for events..")

	var event bpfEvent
	for {
		record, err := r.Read()
		if err != nil {
			if errors.Is(err, ringbuf.ErrClosed) {
				log.Println("Received signal, exiting..")
				return
			}
			log.Printf("reading from reader: %s", err)
			continue
		}

		if err := binary.Read(bytes.NewBuffer(record.RawSample), binary.LittleEndian, &event); err != nil {
			log.Printf("parsing ringbuf event: %s", err)
			continue
		}

		duration := time.Duration(event.EndTime - event.StartTime)
		log.Printf(
			ColorReset+"the function execution time took:"+ColorGreen+"%d"+ColorReset+"\n",
			duration,
		)
	}
}

const (
	ColorReset = "\033[0m"
	ColorGreen = "\033[32m"
)
