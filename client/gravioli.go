package main

import (
	"github.com/Tondorf/gravioli/client/api"
	"github.com/Tondorf/gravioli/client/network"
	"github.com/Tondorf/gravioli/client/visualizer"
	"time"
	"os"
	"strconv"
)

var RUNNING = true

const DEFAULT_PORT = 2017

//go:generate protoc -I=.. ../gravioli.proto --go_out=api
func main() {
	if len(os.Args) < 2 {
		os.Exit(1)
	}
	SERVER := os.Args[1]
	PORT := DEFAULT_PORT
	if len(os.Args) > 2 {
		PORT, _ = strconv.Atoi(os.Args[2])
	}
	worldChan := make(chan *api.World)

	go network.Connect(SERVER, PORT, worldChan)
	go visualizer.Init(worldChan)

	for {
		time.Sleep(1)
	}

}
