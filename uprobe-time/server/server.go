package main

import (
	"flag"
	"fmt"
	"log"
	"math/rand/v2"
	"net/http"
	"strconv"
	"time"
)

var (
	port string
)

func init() {
	flag.StringVar(&port, port, "8091", "The port to listen on")
}

//go:noinline
func demo(arg uint32) {
	entryMarker()

	randSleep := rand.IntN(100)
	time.Sleep(time.Duration(randSleep) * time.Millisecond)

	fmt.Printf("the function was called with arg: %d and took %d ms to complete\n", arg, randSleep)

	exitMarker()
}

//go:noinline
func entryMarker() {}

//go:noinline
func exitMarker() {}

func main() {
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		i, _ := strconv.Atoi(r.URL.Query().Get("arg"))
		demo(uint32(i))
	})

	log.Println("Starting server on :" + port)
	if err := http.ListenAndServe(port, nil); err != nil {
		log.Fatal(fmt.Errorf("failed to start server: %w", err))
	}
}
