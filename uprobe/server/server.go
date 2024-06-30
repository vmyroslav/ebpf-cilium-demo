package main

import (
	"fmt"
	"net/http"
	"strconv"
)

//go:noinline
func demo(arg uint32) {
	fmt.Println(arg)
}

func main() {
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		arg := r.URL.Query().Get("arg")
		i, _ := strconv.Atoi(arg)
		demo(uint32(i))
	})

	_ = http.ListenAndServe(":8080", nil)
}