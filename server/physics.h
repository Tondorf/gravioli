#pragma once

#include <cmath>
#include <map>
#include <memory>
#include <random>
#include <vector>

const double G = 1.;

struct Vec2d {
  double x;
  double y;
};

struct IAObj {
  double mass;
  Vec2d pos;
};

void calcAcceleration(const Vec2d &pos, const std::vector<IAObj> &objs, Vec2d &a, const double g = G) {
  a.x = 0.;
  a.y = 0.;

  for (auto obj : objs) {
    double dx = obj.pos.x - pos.x;
    double dy = obj.pos.y - pos.y;
    double r2 = dx * dx + dy * dy;
    double r = sqrt(r2);
    double gm_over_r3 = g * obj.mass / r2 / r;

    a.x += gm_over_r3 * dx;
    a.y += gm_over_r3 * dy;
  }
}

/*
 * ATTENTION
 *
 * this will map numbers < 1e-5 to the same ID.
 * This is sensible for this special case of caching
 * widths of random distributions, ie:
 * - small differents are imperceptible
 * - width > 1 is uncommon
 */
int width2ID(double width) {
  return static_cast<int>(1e5 * width);
}

std::default_random_engine generator;
std::map<int, std::shared_ptr<std::normal_distribution<double>>> distributions;

double smear(double x, double width) {
  int id = width2ID(width);

  auto elem = distributions.find(id);
  if (elem == distributions.end()) {
    auto newDist = std::make_shared<std::normal_distribution<double>>(1., width);
    distributions.insert({id, newDist});

    return smear(x, width);
  }
  
  auto dist = (*elem).second;
  std::normal_distribution<double> z(1., width);

  double smearFactor = dist->operator()(generator);
  return x * smearFactor;
}
