import numpy as np
import cv2
import socket

UDP_IP = "127.0.0.1"
UDP_PORT = 8888
BUF_LEN=65540
PACK_SIZE=4096


sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
sock.bind((UDP_IP,UDP_PORT))
while True:

    data,client_address= sock.recvfrom(512)
    num = ord(data[0])
    tmp=""
    for i in range(num):
        data,_ = sock.recvfrom(BUF_LEN)
        tmp+=data
        if(len(data)!=PACK_SIZE):
            print("Received unexpected size pack:"+str(len(data)))
            continue
    file_bytes = np.asarray(bytearray(tmp), dtype=np.uint8)
    image = cv2.imdecode(file_bytes, 0)
    cv2.imshow("recv",image)
    cv2.waitKey(10)
    r = 'Receieve'
    sock.sendto(r.encode(),client_address)