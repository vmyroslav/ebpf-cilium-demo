package main

import (
	"flag"
	"fmt"
	"log"
	"net/http"
	"strconv"
)

var (
	port string
)

func init() {
	flag.StringVar(&port, port, "8090", "The port to listen on")
}

func main() {
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		i, _ := strconv.Atoi(r.URL.Query().Get("arg"))
		demo(uint32(i))
	})

	log.Println("Starting server on :" + port)
	if err := http.ListenAndServe(fmt.Sprintf(":%s", port), nil); err != nil {
		log.Fatal(fmt.Errorf("failed to start server: %w", err))
	}
}

//go:noinline
func demo(arg uint32) {
	fmt.Println("call received with an argument:", arg)
}
