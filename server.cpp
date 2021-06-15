#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <string.h>
#include<iostream>
#include <sys/epoll.h>
#include <fcntl.h>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <time.h>
#define PORT 8080
using namespace std;

int total_connection = 0;



class table {
public:
  string read() {
    shared_lock<shared_mutex> r_lock(rw_lock);
    return data;
  }

  void write(string str0) {
    unique_lock<shared_mutex> w_lock(rw_lock);
    data += "o~" + str0;
    cout << "write successed : " << data << endl; 
  }

private:
  string data;
  shared_mutex rw_lock;

};


table table0 = table();

/*
class restfulRequest {
public:
	char method[5];
	char url[100];
private:
  void handle_Reqeust(string restful_request) {
    // parse request url
    return;
  }

};
*/

/*
class task {
	private:
	  string action;

	public:
  task(string action_name) {
    action += action_name;
  }
};
*/

void read_data() {  // read a global memory
    
    thread::id  tid = this_thread::get_id();
    int r;
    if ( (r = (rand()%5) )%2 == 0) {
	    table0.write(to_string(r));
    }
    else { 
             cout << tid << "@ read : " << table0.read() << endl;
	    
    }
}

static void event_add(int epfd, int sockfd, int status);
static void set_nonblocking(int server_fd);
static void event_handler(int epfd, struct epoll_event* events, int num, int client_fd);

int main(int argc, char const *argv[])
{
    srand(time(nullptr));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    char buffer[1024] = {0};
    const char *hello = "Hello from server";
    // create epoll object
    int epfd = epoll_create(10);
    struct epoll_event events[1000];

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    set_nonblocking(server_fd);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
       
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    event_add(epfd, server_fd, EPOLLIN);
    cout << "first socket created and added to epoll" << endl;

    char* buf1 = new char[1024];
    for (;;) {
      int rfds = epoll_wait(epfd, events, 10, -1);
      event_handler(epfd, events, rfds, server_fd);
      memset(buf1, 0, 1024*sizeof(char));
    }
    delete []buf1;
    return 0;
}

static void handle_new_connect(int epfd, int server_fd) {
   struct sockaddr_in address;
   int addrlen = sizeof(address);
   int new_socket;

   if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    { 
      cout << "socket accept failed" << endl;
    } else {

      total_connection++;
      cout << "new connection accepted : " << total_connection << " nc" << endl;
      event_add(epfd, new_socket, EPOLLIN);
    }
    
}


void event_read_action(int epfd, int sockfd, int state) {
	int ret;
	char* buf = new char[1024];
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = sockfd;
        ret = read(sockfd , buf, 1024);
	if (ret <= 0) { 
	  // delete event
	  // close socket
	  epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, &ev);
	  close(sockfd);
	  delete []buf;
	  return;
	}
        printf("%s\n", buf);
        thread t(read_data);
	if (t.joinable() ) {
	  t.join();
	  cout << "joinable" <<endl;
	} else {
	  cout<< "unjoinable" << endl;
        }
	delete []buf;
}

static void event_handler(int epfd, struct epoll_event* events, int num, int server_fd){
  int fd;
  //cout << "readable fd total = " << num << endl;
  for (int i=0; i<num; i++){
      fd = events[i].data.fd;

      if (fd == server_fd && events[i].events & EPOLLIN) {
        handle_new_connect(epfd, fd);
      }

      else if (fd!=server_fd && (events[i].events & EPOLLIN)) {
        event_read_action(epfd, fd, EPOLLIN);
      }
  }
}

static void set_nonblocking(int server_fd) {
    int flags = fcntl(server_fd, F_GETFL);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
}

static void event_add(int epfd, int sockfd, int status) {
        struct epoll_event ev;
	ev.events = status;
	ev.data.fd = sockfd;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
}
