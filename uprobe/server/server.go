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
		i, _ := strconv.Atoi(r.URL.Query().Get("arg"))
		demo(uint32(i))
	})

	_ = http.ListenAndServe(":8080", nil)
}
