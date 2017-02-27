#include "client.h"

std::atomic<std::size_t> Client::_nextID(0); 

std::shared_ptr<Client> Client::create(bufferevent *bufev) {
  std::size_t id = _nextID;
  _nextID = id + 1;

  return std::make_shared<Client>(id, bufev);
}

Client::Client(std::size_t id, bufferevent *bufev): _bufev(bufev), ID(id) {
}

Client::~Client() {
}

void Client::fillInbox(const std::vector<std::uint8_t> &data) {
  _inbox = data;
}

bool Client::isInboxEmpty() const {
  return _inbox.size() == 0;
}

std::vector<std::uint8_t> Client::handOverInbox() {
  std::vector<std::uint8_t> inbox;
  inbox.swap(_inbox);

  return inbox;
}

void Client::send(const std::vector<std::uint8_t> &data) {
  std::uint32_t length = data.size();
  
  // converting e.g. 256 -> 0010
  std::uint8_t arrayOfBytes[4];
  for (int i = 0; i < 4; i++) {
    arrayOfBytes[3 - i] = (length >> (i * 8));
  }

  bufferevent_write(_bufev, arrayOfBytes, 4);
  bufferevent_write(_bufev, &data[0], data.size());
}
