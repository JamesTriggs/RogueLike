#pragma once
#include <array>
#include <cstdint>
#include <deque>
#include <random>
#include <string>
#include <vector>

namespace choir {
constexpr int width = 66;
constexpr int height = 38;
struct Pos {
    int x = 0, y = 0;
    bool operator==(Pos p) const { return x == p.x && y == p.y; }
    bool operator!=(Pos p) const { return !(*this == p); }
};
int distance(Pos a, Pos b);
enum class Tile { Wall, Floor, Door, Lift, Seal, Med, Ammo, Ration, Scrap, Shrine, Survivor, Warp, Machinery };
enum class Kind { Thrall, Chorister, Legionary };
enum class Outcome { Alive, Dead, Possessed, Escaped };
enum class Action { North, South, West, East, Wait, Interact, Heal, Eat, Fire, Throw, Sneak };
struct Cell { Tile tile = Tile::Wall; bool seen = false, visible = false; };
struct Enemy {
    Pos pos;
    Kind kind = Kind::Thrall;
    int hp = 8;
    Pos target;
    int alert = 0;
};
struct Player {
    Pos pos;
    Pos facing{1, 0};
    int hp = 24, resolve = 100, corruption = 0;
    int ammo = 5, meds = 2, rations = 2, scrap = 3, rescued = 0;
    bool seal = false, sneaking = true;
};
struct Message { std::string text; bool danger = false; };
// State is explicit so tests and future save formats can inspect it without SDL.
class Game {
public:
    explicit Game(std::uint32_t seed);
    std::uint32_t seed;
    int deck = 0, turn = 0, deckTurns = 0;
    Player player;
    Outcome outcome = Outcome::Alive;
    std::array<Cell, width * height> cells{};
    std::vector<Enemy> enemies;
    std::deque<Message> messages;
    Pos entry, exit, sealPos;
    Pos noise;
    int noiseLife = 0, noiseRadius = 0;
    bool act(Action action);
    bool inside(Pos p) const;
    Cell& at(Pos p);
    const Cell& at(Pos p) const;
    bool walkable(Pos p) const;
    bool lineOfSight(Pos a, Pos b) const;
    void refreshVision();
    void generateDeck();
    void log(const std::string& text, bool danger = false);
    int choirStage() const;
    std::string objective() const;
    static std::string deckName(int deck);
    static std::string enemyName(Kind kind);
private:
    std::mt19937 rng;
    int random(int low, int high);
    void emitNoise(Pos p, int radius);
    void advance();
    void enemyTurns();
    void pickup();
    bool interact();
    int enemyAt(Pos p) const;
};
} // namespace choir
