// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_PLANET_GAME_H_
#define FLATBUFFERS_GENERATED_PLANET_GAME_H_

#include "flatbuffers/flatbuffers.h"

namespace game {

struct Planet;

struct Planet FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_MASS = 4,
    VT_RADIUS = 6
  };
  float mass() const {
    return GetField<float>(VT_MASS, 0.0f);
  }
  float radius() const {
    return GetField<float>(VT_RADIUS, 0.0f);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<float>(verifier, VT_MASS) &&
           VerifyField<float>(verifier, VT_RADIUS) &&
           verifier.EndTable();
  }
};

struct PlanetBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_mass(float mass) {
    fbb_.AddElement<float>(Planet::VT_MASS, mass, 0.0f);
  }
  void add_radius(float radius) {
    fbb_.AddElement<float>(Planet::VT_RADIUS, radius, 0.0f);
  }
  PlanetBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  PlanetBuilder &operator=(const PlanetBuilder &);
  flatbuffers::Offset<Planet> Finish() {
    const auto end = fbb_.EndTable(start_, 2);
    auto o = flatbuffers::Offset<Planet>(end);
    return o;
  }
};

inline flatbuffers::Offset<Planet> CreatePlanet(
    flatbuffers::FlatBufferBuilder &_fbb,
    float mass = 0.0f,
    float radius = 0.0f) {
  PlanetBuilder builder_(_fbb);
  builder_.add_radius(radius);
  builder_.add_mass(mass);
  return builder_.Finish();
}

inline const game::Planet *GetPlanet(const void *buf) {
  return flatbuffers::GetRoot<game::Planet>(buf);
}

inline bool VerifyPlanetBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<game::Planet>(nullptr);
}

inline void FinishPlanetBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<game::Planet> root) {
  fbb.Finish(root);
}

}  // namespace game

#endif  // FLATBUFFERS_GENERATED_PLANET_GAME_H_
