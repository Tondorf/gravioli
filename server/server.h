#pragma once

#include <functional>
#include <map>
#include <memory>

#include "client.h"

class Server {
private:
  std::map<std::size_t, std::shared_ptr<Client>> _clients;

  Server();

public:
  ~Server();
  static Server &getInstance();
  Server(Server const &) = delete;
  void operator=(Server const &) = delete;

  void stop();

  void addClient(std::shared_ptr<Client>);
  void removeClient(std::size_t);
  bool getClient(std::size_t key, std::shared_ptr<Client> &);
  void send(const std::vector<std::uint8_t> &, std::function<bool(const Client &)>);
  void msgForClient(const std::vector<std::uint8_t> &, std::size_t);
};
