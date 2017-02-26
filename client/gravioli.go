package main

import (
	//"github.com/Tondorf/gravioli/client/network"
	  "github.com/Tondorf/gravioli/client/visualizer"
	)

//go:generate protoc -I=.. ../gravioli.proto --go_out=api

func main() {
    visualizer.WindowTest()

}
