#include "stdafx.h"
#include<stdio.h>
#include<winsock2.h>
#include <iostream>
#include"config.h"
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#define BUF_LEN 65540 // Larger than maximum UDP packet size
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data
#pragma warning( disable : 4996)
#include "opencv2/opencv.hpp"
using namespace std;
using namespace cv;
int main()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUF_LEN];
	WSADATA wsa;

	slen = sizeof(si_other);

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	//keep listening for data
	while (1)
	{

		printf("Waiting for data...");
		fflush(stdout);
		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUF_LEN);
		
		do {
			recv_len = recvfrom(s, buf, BUF_LEN, 0, (struct sockaddr *) &si_other, &slen);
		} while (recv_len > sizeof(int));
		int total_pack = ((int *)buf)[0];

		std::cout << "expecting length of packs:" << total_pack << std::endl;
		char * longbuf = new char[PACK_SIZE * total_pack];
		for (int i = 0; i < total_pack; i++) {
			recv_len = recvfrom(s, buf, BUF_LEN, 0, (struct sockaddr *) &si_other, &slen);
			if (recv_len != PACK_SIZE) {
				cerr << "Received unexpected size pack:" << recv_len << endl;
				continue;
			}
			memcpy(&longbuf[i * PACK_SIZE], buf, PACK_SIZE);
		}

		Mat rawData = Mat(1, PACK_SIZE * total_pack, CV_8UC1, longbuf);
		Mat frame = imdecode(rawData, CV_LOAD_IMAGE_COLOR);
		if (frame.size().width == 0) {
			cerr << "decode failure!" << endl;
			continue;
		}
		imshow("recv", frame);
		free(longbuf);

		waitKey(1);
		////try to receive some data, this is a blocking call
		//if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		//{
		//	printf("recvfrom() failed with error code : %d", WSAGetLastError());
		//	exit(EXIT_FAILURE);
		//}

		////print details of the client/peer and the data received
		//printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		//printf("Data: %s\n", buf);

		////now reply the client with the same data
		//if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
		//{
		//	printf("sendto() failed with error code : %d", WSAGetLastError());
		//	exit(EXIT_FAILURE);
		//}
	}

	closesocket(s);
	WSACleanup();

	return 0;
}
