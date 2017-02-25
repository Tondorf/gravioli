#include <cstdlib>
#include <iostream>

#include "physics.h"

using std::cout;
using std::endl;

bool testSmearing(double smearThis, double smearWidth) {
  const std::size_t N = 1e6;

  std::size_t below = 0;
  std::size_t above = 0;

  const double threshold_low = smearThis * (1. - smearWidth);
  const double threshold_up = smearThis * (1. + smearWidth);
  for (std::size_t i = 0; i < N; i++) {
    const double x = smear(smearThis, smearWidth);
    if (x > threshold_up) {
      above++;
    } else if (x < threshold_low) {
      below++;
    }
  }

  const double dBelow = (double) below;
  const double dAbove = (double) above;

  if (std::abs(dBelow / dAbove - 1.) > .01) {
    cout << "#below and #above should be equal (at a 1\% level)" << endl;
    return false;
  }

  const double oneSigma = erf(1./sqrt(2.));
  if (std::abs((dBelow + dAbove) / ((double) N) - (1. - oneSigma)) > .01) {
    cout << "Inner interval should contain 0.68\% of all entries (at a 1\% level)" << endl;
    return false;
  }

  return true;
}

bool testAccelerationCalculation() {
  std::vector<IAObj> objs;

  auto createObj = [](double mass, double x, double y) {
    Vec2d pos;
    pos.x = x;
    pos.y = y;

    IAObj obj;
    obj.mass = mass;
    obj.pos = pos;

    return obj;
  };

  objs.push_back(createObj(2., 3., 4.));

  Vec2d affObj;
  affObj.x = -5.;
  affObj.y = -6.;

  Vec2d acc;
  calcAcceleration(affObj, objs, acc, 1.);
  
  double r = sqrt(8.*8. + 10.*10.);
  double gm_over_r3 = 2. / r/r/r;
  Vec2d ref;
  ref.x = gm_over_r3 * 8.;
  ref.y = gm_over_r3 * 10.;

  if (std::abs(acc.x / ref.x - 1.) > 1e-3 || std::abs(acc.y / ref.y - 1.) > 1e-3) {
    cout << "The calculation should be correct at a 1 permille level" << endl;
    return false;
  }

  affObj.x = 0.;
  affObj.y = 0.;

  objs.clear();
  objs.push_back(createObj(1., 1., 1.));
  objs.push_back(createObj(1., 1., -1.));
  objs.push_back(createObj(1., -1., 1.));
  objs.push_back(createObj(1., -1., -1.));
  calcAcceleration(affObj, objs, acc, 1.);
  if (std::abs(acc.x) > 1e-10 || std::abs(acc.y) > 1e-10) {
    cout << "At the center of a square, there should be zero-g at a 1e-10 level" << endl;
    return false;
  }

  return true;
}

bool testCaching() {
  distributions.clear();
  smear(10., .1);
  smear(20., .1);
  if (distributions.size() != 1) {
    cout << "different means should not increase caching pool" << endl;
    return false;
  }

  distributions.clear();
  smear(10., .1);
  smear(20., .1);
  smear(30., .101);
  if (distributions.size() != 2) {
    cout << "generators with equal widths should not increase caching pool" << endl;
    return false;
  }

  return true;
}

bool test() {
  cout << "+ testing: smearing" << endl;
  if (!testSmearing(10., .1)) {
    return false;
  }

  cout << "+ testing: caching" << endl;
  if (!testCaching()) {
    return false;
  }

  cout << "+ testing: acceleration calculation" << endl;
  if (!testAccelerationCalculation()) {
    return false;
  }

  return true;
}

int main(int, char **) {
  if (test()) {
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

