#include <cassert>
#include <chrono>
#include <sstream>

#include "client.h"
#include "logger.h"
#include "server.h"
#include "world.h"

Server &Server::getInstance() {
  static Server instance;
  return instance;
}

Server::Server() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // generate world
  world::World::getInstance();

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

bool Server::addClient(std::shared_ptr<Client> client) {
  if (!world::World::getInstance().addPlayer(client->ID)) {
    return false;
  }

  _clients[client->ID] = client;

  auto acc = new api::Acceptance;
  acc->set_id(client->ID);

  auto msg = new api::GravioliMessage;
  msg->set_allocated_accept(acc);
  assert(msg->IsInitialized());
  client->send(serializeMessage(msg));

  return true;
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
  
  auto all =  [](const Client &)->bool { return true; };

  while (_running) {
    auto start = std::chrono::system_clock::now();

    if (_clients.size() > 0) {
      auto msg = dumpWorld();
      auto serializedWorld = serializeMessage(msg);

      forAllClients([this](Client &client) {
        if (!client.isInboxEmpty()) {
          auto msg = client.handOverInbox();
          processMsgFromClient(client.ID, msg);
        }
      });

      send(serializedWorld, all);
    }

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

std::vector<std::uint8_t> Server::serializeMessage(api::GravioliMessage *msg) {
  std::stringstream ss;
  msg->SerializeToOstream(&ss);

  std::string datastring = ss.str();
  return std::vector<std::uint8_t>(datastring.begin(), datastring.end());
}

api::GravioliMessage *Server::dumpWorld() {
  auto world = new api::World;

  for (auto kv : world::World::getInstance().getPlanets()) {
    auto id = kv.first;
    auto planet = kv.second;

    auto planet_pb = world->add_planets();
    auto sphere_pb = new api::Sphere;

    sphere_pb->set_x(planet->x);
    sphere_pb->set_y(planet->y);
    sphere_pb->set_mass(planet->mass);
    sphere_pb->set_radius(planet->radius);
    assert(sphere_pb->IsInitialized());

    planet_pb->set_id(id);
    planet_pb->set_allocated_sphere(sphere_pb);
    planet_pb->set_health(planet->health);
    assert(planet_pb->IsInitialized());
  }

  for (auto kv : world::World::getInstance().getPlayers()) {
    auto id = kv.first;
    auto player = kv.second;

    auto player_pb = world->add_players();

    player_pb->set_id(id);
    player_pb->set_name(player->name);
    player_pb->set_skillpoints(player->skillpoints);
    player_pb->set_homebaseid(player->homebase->id);
    player_pb->set_aim(player->aim);
    player_pb->set_gunpower(player->gunpower);
    player_pb->set_gunlength(player->gunlength);
    player_pb->set_cooldown(player->cooldown);
    assert(player_pb->IsInitialized());
  }

  assert(world->IsInitialized());
  auto message = new api::GravioliMessage;
  message->set_allocated_world(world);
  assert(message->IsInitialized());

  return message;
}
