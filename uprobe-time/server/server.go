package main

import (
	"fmt"
	"math/rand/v2"
	"net/http"
	"strconv"
	"time"
)

//go:noinline
func demo(arg uint32) {
	entryMarker()

	randSleep := rand.IntN(100)
	time.Sleep(time.Duration(randSleep) * time.Millisecond)
	fmt.Println(arg)
	fmt.Println(randSleep)

	exitMarker()
}

//go:noinline
func entryMarker() {}

//go:noinline
func exitMarker() {}

func main() {
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		arg := r.URL.Query().Get("arg")
		i, _ := strconv.Atoi(arg)
		demo(uint32(i))
	})

	_ = http.ListenAndServe(":8080", nil)
}
