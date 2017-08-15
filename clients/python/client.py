import signal
import time
import zmq

from multiprocessing import Process


def client(port):
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.connect('tcp://127.0.0.1:{}'.format(port))
    socket.setsockopt_string(zmq.SUBSCRIBE, '')

    running = True

    def stopReading(signal, frame): 
        nonlocal running
        running = False
    signal.signal(signal.SIGINT, stopReading)

    while running:
        try:
            msg = socket.recv(zmq.NOBLOCK)
        except zmq.ZMQError:
            time.sleep(0.1)
        else:
            print(msg)

    socket.close()


if __name__ == "__main__":
    client(8888)
