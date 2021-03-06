#include"stdafx.h"
#include<stdio.h>
#include<winsock2.h>
#include "opencv2/opencv.hpp"
#include "config.h"
#include <ctime>
#pragma warning(disable:4996) 
#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define SERVER "127.0.0.1"  //ip address of udp server
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data
typedef char raw_type;

using namespace cv;
using namespace std;


void Video()
{
	Mat frame, send;
	cv::VideoCapture cap(0); // Grab the camera
	namedWindow("send", CV_WINDOW_AUTOSIZE);
	while (1) {
		cap >> frame;
		if (frame.size().width == 0)continue;//simple integrity check; skip erroneous data...
		resize(frame, send, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
		cv::cvtColor(send, send, CV_BGR2GRAY);
		imshow("send", send);
		waitKey(FRAME_INTERVAL);
	}

}

int main(void)
{
	int jpegqual = ENCODE_QUALITY; // Compression Parameter
	cv::Mat image;
	image = cv::imread("test.JPG",0);
	resize(image, image, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
	//imshow("send", image);
	waitKey(FRAME_INTERVAL);

	vector < int > compression_params;
	vector < uchar > encoded;
	compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
	compression_params.push_back(jpegqual);
	imencode(".jpg", image, encoded, compression_params);

	int total_pack = 1 + (encoded.size() - 1) / PACK_SIZE;
	int ibuf[1];
	ibuf[0] = total_pack;

	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];
	WSADATA wsa;

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//create socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//setup address structure
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);

	//start communication
	while (1)
	{



		printf("Enter message : ");
		std::clock_t start;
		double duration;
		start = std::clock();
		//gets_s(message);
		sendto(s, (raw_type *)ibuf, sizeof(int), 0, (struct sockaddr *) &si_other, slen);
		for (int i = 0; i < total_pack; i++)
		{
			sendto(s, (raw_type *)(&encoded[i * PACK_SIZE]), PACK_SIZE, 0, (struct sockaddr *) &si_other, slen);
		}

		////send the message
		//if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
		//{
		//	printf("sendto() failed with error code : %d", WSAGetLastError());
		//	exit(EXIT_FAILURE);
		//}

		////receive a reply and print it
		////clear the buffer by filling null, it might have previously received data

		//try to receive some data, this is a blocking call

		memset(buf, '\0', BUFLEN);
		recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen);
		printf("recvfrom() failed with error code : %d", WSAGetLastError());
		//if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR)
		//{
		//	
		//	exit(EXIT_FAILURE);
		//}

		puts(buf);
		duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
		std::cout << "printf: " << duration << '\n';
	}

	closesocket(s);
	WSACleanup();

	return 0;
}