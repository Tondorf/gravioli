#include <cassert>
#include <chrono>
#include <sstream>

#include "client.h"
#include "logger.h"
#include "server.h"

Server &Server::getInstance() {
  static Server instance;
  return instance;
}

Server::Server() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  _thread = std::make_shared<std::thread>(&Server::loop, this);
  _running = true;
}

Server::~Server() {
}

void Server::stop() {
  Log::info("Stopping game loop.");
  _running = false;
  _thread->join();

  google::protobuf::ShutdownProtobufLibrary();
}

void Server::addClient(std::shared_ptr<Client> client) {
  _clients[client->ID] = client;

  auto acc = new api::Accept;
  acc->set_id(client->ID);

  auto msg = new api::Message;
  msg->set_allocated_accept(acc);
  assert(msg->IsInitialized());
  client->send(serializeMessage(msg));
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

void Server::msgFromClientToServer(const std::vector<std::uint8_t> &data, std::size_t id) {
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

void Server::loop() {
  Log::debug("Starting game loop.");
  
  auto world = new api::World;
  auto planet = world->add_planets();
  auto shot = world->add_shots();
  auto player = world->add_players();

  assert(planet != nullptr);
  assert(shot != nullptr);
  assert(player != nullptr);

  auto sphere1 = new api::Sphere;
  sphere1->set_x(0.);
  sphere1->set_y(0.);
  sphere1->set_mass(1e3);
  sphere1->set_radius(10);
  assert(sphere1->IsInitialized());

  planet->set_id(0);
  planet->set_allocated_sphere(sphere1);
  planet->set_health(10.);
  assert(planet->IsInitialized());

  auto sphere2 = new api::Sphere;
  sphere2->set_x(20.);
  sphere2->set_y(30.);
  sphere2->set_mass(.1);
  sphere2->set_radius(.5);
  assert(sphere2->IsInitialized());

  shot->set_id(0);
  shot->set_origin(0);
  shot->set_allocated_sphere(sphere2);
  shot->set_dir(3.14159/2.);
  shot->set_speed(11.);
  shot->set_ttl(2.);
  shot->set_dmg(1.);
  assert(shot->IsInitialized());

  player->set_id(0);
  player->set_name("Muttermundkontaktbolzen");
  player->set_skillpoints(0);
  player->set_homebaseid(0);
  player->set_aim(3.14159);
  player->set_cooldown(1.);
  assert(player->IsInitialized());

  assert(world->IsInitialized());
  auto message = new api::Message;
  message->set_allocated_world(world);
  assert(message->IsInitialized());
  auto serializedWorld = serializeMessage(message);

  auto all =  [](const Client &)->bool { return true; };

  while (_running) {
    auto start = std::chrono::system_clock::now();

    forAllClients([this](Client &client) {
      if (!client.isInboxEmpty()) {
        auto msg = client.handOverInbox();
        processMsgFromClient(client.ID, msg);
      }
    });

    send(serializedWorld, all);

    auto stop = std::chrono::system_clock::now();
    auto dt = static_cast<std::size_t>(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());
    auto timeleft = 10 - dt;
    if (timeleft > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(timeleft));
    } else {
      Log::warning("Calculation in game loop took more time than allowed.");
    }
  }
}

void Server::processMsgFromClient(std::size_t, const std::vector<std::uint8_t> &) {
}

void Server::forAllClients(std::function<void(Client &)> f) {
  for (auto kv : _clients) {
    f(*(kv.second));
  }
}

std::vector<std::uint8_t> Server::serializeMessage(api::Message *msg) {
  std::stringstream ss;
  msg->SerializeToOstream(&ss);

  std::string datastring = ss.str();
  return std::vector<std::uint8_t>(datastring.begin(), datastring.end());
}
