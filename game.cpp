#include "game.hpp"

#include <sstream>
#include <stdexcept>

namespace avoider {
namespace {

bool validColumn(int column) {
  return column >= 1 && column <= kColumns;
}

bool validObstacleMask(int mask) {
  if (mask < 0 || mask >= (1 << kColumns)) {
    return false;
  }
  int blocked = 0;
  for (int column = 1; column <= kColumns; ++column) {
    if (mask & (1 << (column - 1))) {
      ++blocked;
    }
  }
  return blocked == 2 || blocked == 3;
}

bool blocksColumn(int mask, int column) {
  return (mask & (1 << (column - 1))) != 0;
}

}  // namespace

GameState makeGame(const std::array<int, 3>& initialMasks) {
  GameState game;
  for (int index = 0; index < 3; ++index) {
    if (!validObstacleMask(initialMasks[index])) {
      throw std::invalid_argument("Obstacle row must leave 1 or 2 open lanes");
    }
    game.obstacles[index * kSpawnEvery] = initialMasks[index];
  }
  return game;
}

StepResult movePlayer(GameState& game, int column) {
  if (game.gameOver || !validColumn(column)) {
    return StepResult::invalidInput;
  }
  game.playerColumn = column;
  if (blocksColumn(game.obstacles[kRows - 1], column)) {
    game.gameOver = true;
    return StepResult::collision;
  }
  return StepResult::running;
}

StepResult advanceTick(GameState& game, int spawnMask) {
  if (game.gameOver || !validObstacleMask(spawnMask)) {
    return StepResult::invalidInput;
  }

  const bool passedObstacle = game.obstacles[kRows - 1] != 0;
  for (int row = kRows - 1; row > 0; --row) {
    game.obstacles[row] = game.obstacles[row - 1];
  }
  game.obstacles[0] = 0;
  ++game.ticks;

  if (blocksColumn(game.obstacles[kRows - 1], game.playerColumn)) {
    game.gameOver = true;
    return StepResult::collision;
  }

  if (passedObstacle) {
    ++game.score;
  }
  if (game.ticks % kSpawnEvery == 0) {
    game.obstacles[0] = spawnMask;
  }
  return StepResult::running;
}

std::string render(const GameState& game) {
  std::ostringstream out;
  out << "Four-Column Obstacle Avoider    Score: " << game.score << '\n';
  out << "      1   2   3   4\n";
  out << "    +---+---+---+---+\n";
  for (int row = 0; row < kRows; ++row) {
    out << "    |";
    for (int column = 1; column <= kColumns; ++column) {
      const bool obstacle = blocksColumn(game.obstacles[row], column);
      const bool player = row == kRows - 1 && game.playerColumn == column;
      char symbol = ' ';
      if (obstacle && player) {
        symbol = '!';
      } else if (player) {
        symbol = 'P';
      } else if (obstacle) {
        symbol = 'X';
      }
      out << ' ' << symbol << " |";
    }
    out << '\n';
  }
  out << "    +---+---+---+---+\n";
  if (game.gameOver) {
    out << "Collision! Final score: " << game.score << '\n';
    out << "Press r to restart or q to quit.\n";
  } else {
    out << "Press 1-4 to move. Press q to quit.\n";
    out << "No Enter needed. Each obstacle row leaves 1-2 lanes open.\n";
  }
  return out.str();
}

}  // namespace avoider
