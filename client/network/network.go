package network

import "net"
import "strconv"

//import "github.com/golang/protobuf/proto"
import (
	"../api"
	"fmt"
	"github.com/golang/protobuf/proto"
	"log"
)

func Connect(server string, port int, w chan *api.World) error {
    conn, err := net.Dial("tcp", server+":"+strconv.Itoa(port))
    if err != nil {
		fmt.Println("Connection error:", err)
		return err
	}
	defer conn.Close()
	go fetchWorldForever(conn, w)
	sendInputsForever(conn)
	return nil
}

func fetchWorldForever(conn net.Conn, w chan *api.World) {
	world := &api.World{}
	var bytes []byte
	for {
		if _, err := conn.Read(bytes); err != nil {
			log.Fatalln("Failed to Read from con:", err)
		}
		if err := proto.Unmarshal(bytes, world); err != nil {
			log.Fatalln("Failed to Unmarshal:", err)
		}
		w <- world
	}
}

func sendInputsForever(conn net.Conn, act chan proto.Marshaler) {
	for {

	}
}
