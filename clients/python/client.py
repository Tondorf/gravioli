import signal
import time
import zmq

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend

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

    key = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f'
    backend = default_backend()

    while running:
        try:
            iv, cipher = socket.recv_multipart(zmq.NOBLOCK)
        except zmq.ZMQError:
            time.sleep(0.1)
        else:
            aes = Cipher(algorithms.AES(key), modes.CFB(iv), backend=backend)
            decryptor = aes.decryptor()
            print(['{:2x}'.format(b) for b in cipher])
            msg = decryptor.update(cipher) + decryptor.finalize()
            print(['{:2x}'.format(b) for b in msg])

    socket.close()


if __name__ == "__main__":
    client(8888)
