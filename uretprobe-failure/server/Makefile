all: build

build:
	go build -gcflags="all=-N -l"  -o app server.go

clean:
	rm -f trace


#nm -n path/to/go/binary | grep demo
