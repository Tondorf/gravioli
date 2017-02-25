#pragma once

#include <cmath>
#include <mem>

const double G = 1;

struct Vec2d {
  double x;
  double y;
};

struct IAObj {
  double mass;
  Vec2d pos;
}

void calcAcceleration(const Vec2d &pos, const std::vector<IAObj> &objs, vec2d &a) {
  for (obj : objs) {
    const double dx = pos.x - obj.x;
    const double dy = pos.y - obj.y;
    const double r2 = dx * dx + dy * dy;
    const double r = sqrt(r2);
    const double Gm_over_r3 = G * obj.mass / r2 / r;

    a.x += m_over_r3 * dx;
    a.y += m_over_r3 * dy;
  }
}

const double sqrt_twopi = sqrt(2. * 3.14159265359);

double gaussian(double x, double mean, double width) {
  const double arg = (x - mean) / width;
  return exp(-0.5 * arg * arg) / sqrt_twopi;
}

double smear(double x, double width) {
  return x * gaussian(x, 1., width);
}
