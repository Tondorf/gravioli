package main

import (
	//"github.com/Tondorf/gravioli/client/network"
	  "github.com/Tondorf/gravioli/client/visualizer"
	)

//go:generate mkdir api ; protoc -I=.. ../gravioli.proto --go_out=api

func main() {
    visualizer.WindowTest()

}

