#include "client.h"
#include "logger.h"
#include "server.h"

#include "gravioli.pb.h"


Server &Server::getInstance() {
  static Server instance;
  return instance;
}

Server::Server() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
}

Server::~Server() {
}

void Server::stop() {
  google::protobuf::ShutdownProtobufLibrary();
}

void Server::addClient(std::shared_ptr<Client> client) {
  _clients[client->ID] = client;
}

void Server::removeClient(std::size_t id) {
  _clients.erase(id);
}

bool Server::getClient(std::size_t id, std::shared_ptr<Client> &client) {
  if (_clients.count(id) == 0) {
    client == nullptr;
    return false;
  }

  client = _clients[id];
  return true;
}

void Server::msgForClient(const std::vector<std::uint8_t> &data, std::size_t id) {
  std::shared_ptr<Client> client;
  if (getClient(id, client)) {
    client->fillInbox(data);
  }
}

void Server::send(const std::vector<std::uint8_t> &msg, std::function<bool(const Client &)> filter) {
  for (auto kv : _clients) {
    auto client = kv.second;
    if (filter(*client)) {
      client->send(msg);
    }
  }
}
