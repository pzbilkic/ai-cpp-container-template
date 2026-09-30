#ifndef OBSTACLE_AVOIDER_GAME_HPP
#define OBSTACLE_AVOIDER_GAME_HPP

#include <array>
#include <string>

namespace avoider {

constexpr int kRows = 15;
constexpr int kColumns = 4;
constexpr int kSpawnEvery = 4;
constexpr std::array<int, 10> kObstacleMasks{{
    0b0011, 0b0101, 0b0110, 0b1001, 0b1010, 0b1100,
    0b0111, 0b1011, 0b1101, 0b1110,
}};

struct GameState {
  std::array<int, kRows> obstacles{};
  int playerColumn = 2;
  int score = 0;
  int ticks = 0;
  bool gameOver = false;
};

enum class StepResult { running, collision, invalidInput };

GameState makeGame(const std::array<int, 3>& initialMasks);
StepResult movePlayer(GameState& game, int column);
StepResult advanceTick(GameState& game, int spawnMask);
std::string render(const GameState& game);

}  // namespace avoider

#endif
