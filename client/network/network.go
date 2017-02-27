package network

import "net"
import "strconv"

//import "github.com/golang/protobuf/proto"
import (
	"github.com/Tondorf/gravioli/client/api"
	"fmt"
	"github.com/golang/protobuf/proto"
	"log"
	//"time"
	"io/ioutil"
	"time"
)

func Connect(server string, port int, w chan *api.World) error {
	conn, err := net.Dial("tcp", server+":"+strconv.Itoa(port))
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
	gmsg := &api.GravioliMessage{}
	for {
		var bytes []byte
		var err error
		if bytes, err = ioutil.ReadAll(conn); err != nil {
			log.Fatalln("Failed to ReadAll from con:", err)
		}
		if err := proto.Unmarshal(bytes, gmsg); err != nil {
			log.Fatalln("Failed to Unmarshal:", err)
		}
		if gmsg.World != nil {
			w <- gmsg.World
		} else {
			log.Println("No world received with gravioli message")
		}
		time.Sleep(1)
	}
}

//func sendInputsForever(conn net.Conn, act chan ...) {
//	for {
//		time.Sleep(1)
//	}
//}
