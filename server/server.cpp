#include <arpa/inet.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <memory>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

event_base *evbase;

struct Client {
  int fd;
  bufferevent *bufev;
};

int setnonblock(int fd) {
  int flags = fcntl(fd, F_GETFL);
  if (flags < 0) {
    return flags;
  }

  flags |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) < 0) {
    return -1;
  }

  return 0;
}

void onRead(bufferevent *bufev, void *) {
  std::uint8_t chunk[8192]; // read 8k at a time
  std::size_t n = 0;
  do {
    n = bufferevent_read(bufev, chunk, sizeof(chunk));
  } while(n > 0);
}

void onError(bufferevent *, short what, void *arg) {
  if (what & BEV_EVENT_EOF) {
    std::cout << "[LOG] client disconnected" << std::endl;
  } else {
    std::cout << "[LOG] client socket error, disconnecting" << std::endl;
  }

  auto client = static_cast<Client *>(arg);
  bufferevent_free(client->bufev);
  close(client->fd);
  delete client;
}

void onAccept(int fd, short, void *) {
  sockaddr_in addr;
  socklen_t len = sizeof(addr);

  int clientfd = accept(fd, (sockaddr *)&addr, &len);
  if (clientfd < 0) {
    std::cerr << "[LOG] accept failed" << std::endl;
    return;
  }

  if (setnonblock(clientfd) < 0) {
    std::cerr << "[LOG] failed to set client socket non-blocking" << std::endl;
    return;
  }

  auto client = new Client;
  client->fd = clientfd;
  client->bufev = bufferevent_socket_new(evbase, clientfd, 0);
  bufferevent_setcb(client->bufev, onRead, nullptr, onError, client);
  bufferevent_enable(client->bufev, EV_READ);

  std::cout << "[LOG] accepted connection from " << inet_ntoa(addr.sin_addr) << std::endl;
}


void onSignal(evutil_socket_t, short, void *arg) {
	event_base *base = static_cast<event_base *>(arg);
	timeval delay = { 2, 0 };

	std::cout << "[LOG] caught an interrupt signal; exiting cleanly in two seconds" << std::endl;

	event_base_loopexit(base, &delay);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [port]" << std::endl;
    return EXIT_FAILURE;
  }

  std::ifstream splashfile("splash.txt"); 
  if (splashfile.is_open()) {
    std::string line;
    while (getline(splashfile, line)) {
      std::cout << line << std::endl;
    }
    splashfile.close();
  }
  std::cout << std::endl;

  std::uint16_t port = atoi(argv[1]);
  std::cout << "starting server on port " << port << std::endl;
	
  evbase = event_base_new();
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    std::cerr << "[LOG] listen failed" << std::endl;
    return EXIT_FAILURE;
  }

  sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if (bind(listenfd, (sockaddr *)&addr, sizeof(addr)) < 0) {
    std::cerr << "[LOG] bind failed" << std::endl;
    return EXIT_FAILURE;
  }

  if (listen(listenfd, 5) < 0) {
    std::cerr << "[LOG] listen failed" << std::endl;
    return EXIT_FAILURE;
  }

  int reuseaddr_on = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on));

  if (setnonblock(listenfd) < 0) {
    std::cerr << "failed to set server socket to non-blocking" << std::endl;
    return EXIT_FAILURE;
  }

  event evacc;
  event_assign(&evacc, evbase, listenfd, EV_READ | EV_PERSIST, onAccept, nullptr);
  event_add(&evacc, nullptr);

  event *evsig = evsignal_new(evbase, SIGINT, onSignal, (void *)evbase);
  event_add(evsig, nullptr);

  event_base_dispatch(evbase);

	event_free(evsig);
	event_base_free(evbase);

	std::cout << "[LOG] done" << std::endl;

  return EXIT_SUCCESS;
}
