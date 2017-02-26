#include <csignal>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>

#include <arpa/inet.h>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "logger.h"
#include "server.h"


void onRead(bufferevent *bufev, void *arg) {
  std::vector<std::uint8_t> data;

  std::size_t n = 0;
  do {
    std::uint8_t chunk[1024];
    n = bufferevent_read(bufev, chunk, sizeof(chunk));

    std::size_t size = data.size();
    data.resize(size + n);
    memcpy(&data[size], chunk, n);
  } while(n > 0);

  auto id = static_cast<int *>(arg);
  Server::getInstance().msgForClient(data, *id);

  Log::debug("Received data from client %d.", *id);
}

void onEvent(bufferevent *bufev, short what, void *arg) {
  auto id = static_cast<int *>(arg);
  std::shared_ptr<Client> client;
  if (!Server::getInstance().getClient(*id, client)) {
    Log::error("Could not find Client %d", *id);
    return;
  }

  if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
    bufferevent_free(bufev);

    Log::info("Client %d disconnected.", client->ID);
  }

  Server::getInstance().removeClient(*id);
  delete id;
}

void onAccept(evconnlistener *listener, evutil_socket_t fd, sockaddr *, int, void *) {
  event_base *base = evconnlistener_get_base(listener);
  bufferevent *bufev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

  auto client = Client::create(bufev);
  auto id = new std::size_t;
  *id = client->ID;
  Server::getInstance().addClient(client);

  bufferevent_setcb(bufev, onRead, nullptr, onEvent, static_cast<void *>(id));
  bufferevent_enable(bufev, EV_READ | EV_WRITE);

  Log::info("Accepting new Client and assigning ID %d.", client->ID);
}

void onError(evconnlistener *listener, void *) {
  event_base *base = evconnlistener_get_base(listener);
  int err = EVUTIL_SOCKET_ERROR();

  Log::error("Got an error %d (%s) on the listener. Shutting down.", err, evutil_socket_error_to_string(err));

  event_base_loopexit(base, nullptr);
}

void onSignal(evutil_socket_t, short, void *arg) {
	event_base *base = static_cast<event_base *>(arg);
	timeval delay = { 2, 0 };

  Log::info("Caught an interrupt signal; exiting cleanly in two seconds.");

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

  Log::SimpleLogger::getInstance().setLogLevel(Log::LogLevel::DEBUG);
  Log::debug("Setting log level to DEBUG");

  std::uint16_t port = atoi(argv[1]);
  Log::info("Starting server on port %d.", port);
	
  auto base = event_base_new();

  sockaddr_in sin;
  std::memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(0x7f000001);
  sin.sin_port = htons(port);

  auto flags = LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE;
  auto listener = evconnlistener_new_bind(base, onAccept, nullptr, flags, -1, (sockaddr*)&sin, sizeof(sin));
  if (!listener) {
    Log::error("Could not create listener");
    return EXIT_FAILURE;
  }

  evconnlistener_set_error_cb(listener, onError);

  auto evsig = evsignal_new(base, SIGINT, onSignal, (void *)base);
  event_add(evsig, nullptr);

  event_base_dispatch(base);

  evconnlistener_free(listener);
	event_free(evsig);

  Log::info("Done.");

  return EXIT_SUCCESS;
}
