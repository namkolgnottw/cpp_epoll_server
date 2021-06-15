#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
//#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include <time.h>

#define PORT 8080
using namespace std;

void send_restful_request(int sockfd, string message, int t_index) {
	string context;
	context += to_string(t_index);
	context += ": ";
	context += message;
	context += "\n";
	send(sockfd , context.c_str(), context.length() , 0);
	cout <<"Hello message sent" << endl;

  
}

void thread_act(int t_index) {
	thread::id tid = this_thread::get_id();
	cout << tid << "  thread is start." << endl;
	int sock = 0;
	struct sockaddr_in serv_addr;
	string hello = "Hello from client";
	char buffer[1024] = {0};

	  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	  {
	  	printf("\n Socket creation error \n");
	  	return;
	  }

	  serv_addr.sin_family = AF_INET;
	  serv_addr.sin_port = htons(PORT);
	
  	// Convert IPv4 and IPv6 addresses from text to binary form
         	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	  {
	   	  printf("\nInvalid address/ Address not supported \n");
		  return;
	  }

	  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	  {
	  	  printf("\nConnection Failed \n");
		  return;
	  }

	  for (int i=0; i<3; i++) {
            send_restful_request(sock, hello, t_index);
	    cout << "sleep" << endl;
	    this_thread::sleep_for(chrono::seconds(rand()%8+1));
	  }
	  close(sock);
}

int main(int argc, char const *argv[])
{

	unsigned seed;
	seed = (unsigned)time(NULL);
	srand(seed);
	vector<thread> thread_arr;
	for (int i=0; i<100; i++) {
	  thread_arr.push_back(thread(thread_act, i));
	}
	for (int i=0; i<100; i++)
	  thread_arr[i].join();

	return 0;
}
