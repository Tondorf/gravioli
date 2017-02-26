#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include <event2/bufferevent.h>

class Client {
private:
  static std::atomic<std::size_t> _nextID;

  bufferevent *_bufev;
  std::vector<std::uint8_t> _inbox;

public:
  const std::size_t ID;

  static std::shared_ptr<Client> create(bufferevent *);

  Client(std::size_t id, bufferevent *);
  virtual ~Client();

  void fillInbox(const std::vector<std::uint8_t> &);
  void send(const std::vector<std::uint8_t> &);
};
