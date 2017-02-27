package network

import "net"
import "strconv"

//import "github.com/golang/protobuf/proto"
import (
	"github.com/Tondorf/gravioli/client/api"
	"fmt"
	"github.com/golang/protobuf/proto"
	"log"
	//"io/ioutil"
	"time"
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
	gmsg := &api.GravioliMessage{}
	for {
		bytes := make([]byte, 1024)
		var err error
		//bytes, err = ioutil.ReadAll(conn)
		n, err := conn.Read(bytes)
		if err != nil {
			log.Fatalln("Failed to ReadAll from con:", err)
		} else {
			log.Println("Received", n, "bytes:", bytes)
		}
		if err := proto.Unmarshal(bytes, gmsg); err != nil {
			log.Fatalln("Failed to Unmarshal:", err)
		} else {
			log.Println("Unmarshalled:", gmsg)
		}
		if gmsg.World != nil {
			w <- gmsg.World
		} else {
			log.Println("No world received with gravioli message")
		}
		time.Sleep(time.Second)
	}
}

//func sendInputsForever(conn net.Conn, act chan ...) {
//	for {
//		time.Sleep(1)
//	}
//}
