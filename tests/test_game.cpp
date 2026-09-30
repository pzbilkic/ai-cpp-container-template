#include "game.hpp"

#include <array>
#include <cassert>
#include <stdexcept>
#include <string>

using avoider::GameState;
using avoider::StepResult;

int blockedCount(int mask) {
  int count = 0;
  for (int column = 1; column <= 4; ++column) {
    if (mask & (1 << (column - 1))) {
      ++count;
    }
  }
  return count;
}

int main() {
  assert(avoider::kColumns == 4);
  for (int mask : avoider::kObstacleMasks) {
    assert(blockedCount(mask) == 2 || blockedCount(mask) == 3);
    GameState valid = avoider::makeGame({mask, mask, mask});
    assert(valid.obstacles[0] == mask);
  }

  bool rejected = false;
  try {
    avoider::makeGame({0b1111, 0b0011, 0b0111});
  } catch (const std::invalid_argument&) {
    rejected = true;
  }
  assert(rejected);

  const std::array<int, 3> initial{{0b0101, 0b1011, 0b1101}};
  GameState game = avoider::makeGame(initial);
  assert(game.playerColumn == 2);
  assert(game.score == 0);
  assert(game.ticks == 0);
  assert(game.obstacles[0] == initial[0]);
  assert(game.obstacles[4] == initial[1]);
  assert(game.obstacles[8] == initial[2]);
  assert(game.obstacles[14] == 0);

  assert(avoider::movePlayer(game, 4) == StepResult::running);
  assert(game.playerColumn == 4);
  assert(avoider::movePlayer(game, 2) == StepResult::running);

  const GameState beforeInvalid = game;
  assert(avoider::movePlayer(game, 5) == StepResult::invalidInput);
  assert(avoider::advanceTick(game, 0b1111) ==
         StepResult::invalidInput);
  assert(game.obstacles == beforeInvalid.obstacles);
  assert(game.playerColumn == beforeInvalid.playerColumn);
  assert(game.score == beforeInvalid.score);
  assert(game.ticks == beforeInvalid.ticks);

  for (int tick = 1; tick <= 6; ++tick) {
    assert(avoider::advanceTick(game, 0b0110) == StepResult::running);
    assert(game.ticks == tick);
    if (tick == 4) {
      assert(game.obstacles[0] == 0b0110);
    }
  }
  assert(game.obstacles[14] == initial[2]);
  assert(game.score == 0);
  assert(avoider::advanceTick(game, 0b0110) == StepResult::running);
  assert(game.score == 1);
  assert(game.obstacles[14] == 0);

  GameState movedIntoObstacle = avoider::makeGame(initial);
  for (int tick = 0; tick < 6; ++tick) {
    assert(avoider::advanceTick(movedIntoObstacle, 0b0110) ==
           StepResult::running);
  }
  assert(avoider::movePlayer(movedIntoObstacle, 4) ==
         StepResult::collision);
  assert(movedIntoObstacle.gameOver);
  assert(movedIntoObstacle.score == 0);
  assert(avoider::render(movedIntoObstacle).find('!') !=
         std::string::npos);
  assert(avoider::advanceTick(movedIntoObstacle, 0b0110) ==
         StepResult::invalidInput);

  GameState hitByObstacle = avoider::makeGame(initial);
  assert(avoider::movePlayer(hitByObstacle, 1) == StepResult::running);
  for (int tick = 0; tick < 5; ++tick) {
    assert(avoider::advanceTick(hitByObstacle, 0b0110) ==
           StepResult::running);
  }
  assert(avoider::advanceTick(hitByObstacle, 0b0110) ==
         StepResult::collision);
  assert(hitByObstacle.gameOver);
  assert(hitByObstacle.score == 0);

  GameState restarted = avoider::makeGame(initial);
  assert(restarted.score == 0);
  assert(restarted.ticks == 0);
  assert(restarted.playerColumn == 2);
  assert(restarted.obstacles == beforeInvalid.obstacles);
}
