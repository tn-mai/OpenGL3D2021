/**
* @file GameEngine.h
*/
#include "Primitive.h"
#include "Texture.h"
#include "Actor.h"
#include <unordered_map>
#include <random>

using ActorList = std::vector<std::shared_ptr<Actor>>;
using TextureBuffer = std::unordered_map<std::string, std::shared_ptr<Texture>>;

/**
* ÉQÅ[ÉÄÉGÉìÉWÉì
*/
class GameEngine
{
public:
  static bool Initialize();
  static void Finalize();
  static GameEngine& Get();

  ActorList& GetActors() { return actors; }
  void AddActor(std::shared_ptr<Actor> actor) { newActors.push_back(actor); }
  void UpdateActors();

  PrimitiveBuffer& GetPrimitiveBuffer() { return *primitiveBuffer; }
  const Primitive& GetPrimitive(int n) { return primitiveBuffer->Get(n); }

  bool LoadPrimitive(const char* filename);
  const Primitive& GetPrimitive(const char* filename) const;

  std::shared_ptr<Texture> LoadTexture(const char* filename);
  std::shared_ptr<Texture> GetTexture(const char* filename) const;

  unsigned int GetRandom();

private:
  GameEngine() = default;
  ~GameEngine() = default;
  GameEngine(const GameEngine&) = delete;
  GameEngine& operator=(const GameEngine&) = delete;

  ActorList actors;
  ActorList newActors;
  std::shared_ptr<PrimitiveBuffer> primitiveBuffer;
  TextureBuffer textureBuffer;
  std::mt19937 rg;
};