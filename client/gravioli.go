package main

import "./network"
import "./visualizer"

//go:generate protoc -I=.. ../gravioli.proto --go_out=api
func main() {
    visualizer.WindowTest()

}
