package network

import (
	"net"
	"strconv"
	"github.com/Tondorf/gravioli/client/api"
	"fmt"
	"github.com/golang/protobuf/proto"
	"log"
	"encoding/binary"
)

func Connect(server string, port int, w chan *api.World) error {
	connect := server+":"+strconv.Itoa(port)
	fmt.Println(connect)
	conn, err := net.Dial("tcp", connect)
	if err != nil {
		fmt.Println("Connection error:", err)
		return err
	}
	//defer conn.Close()
	go fetchWorldForever(conn, w)
	//sendInputsForever(conn, )
	return nil
}

func fetchWorldForever(conn net.Conn, w chan<- *api.World) {
	log.Println("fetchWorldForever()")

	for {
		lenbytes := make([]byte, 4)
		n, err := conn.Read(lenbytes)
		if err != nil {
			log.Fatalln("Failed to Read from con:", err)
		}
		if n != 4 {
			log.Fatalln("Expected 4 len bytes, got", n)
		}
		protolen := binary.BigEndian.Uint32(lenbytes)

		protobytes := make([]byte, protolen)
		n, err = conn.Read(protobytes)
		if err != nil {
			log.Fatalln("Failed to Read from con:", err)
		}
		if n != int(protolen) {
			log.Fatalln("Expected", protolen, "bytes, got", n)
		}

		gmsg := &api.GravioliMessage{}
		if err := proto.Unmarshal(protobytes[:n], gmsg); err != nil {
			log.Fatalln("Failed to Unmarshal:", err)
		} else {
			//log.Println("Unmarshalled:", gmsg)
		}
		if gmsg.World != nil {
			w <- gmsg.World
		} else {
			log.Println("No world received with last gravioli message")
		}
	}

}

//func sendInputsForever(conn net.Conn, act chan ...) {
//	for {
//		time.Sleep(1)
//	}
//}
