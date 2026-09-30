#include "game.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cctype>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <limits>
#include <poll.h>
#include <random>
#include <stdexcept>
#include <string>
#include <termios.h>
#include <unistd.h>

namespace {

volatile std::sig_atomic_t stopRequested = 0;
constexpr auto kStepTime = std::chrono::milliseconds(160);
constexpr int kFrameTimeMs = 33;

void requestStop(int) {
  stopRequested = 1;
}

class Terminal {
 public:
  Terminal() : hasInputTerminal_(isatty(STDIN_FILENO)),
               hasOutputTerminal_(isatty(STDOUT_FILENO)) {
    if (hasInputTerminal_) {
      if (tcgetattr(STDIN_FILENO, &original_) != 0) {
        throw std::runtime_error("Could not read terminal settings");
      }
      termios immediate = original_;
      immediate.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
      immediate.c_cc[VMIN] = 0;
      immediate.c_cc[VTIME] = 0;
      if (tcsetattr(STDIN_FILENO, TCSANOW, &immediate) != 0) {
        throw std::runtime_error("Could not enable immediate keys");
      }
    }
    if (hasOutputTerminal_) {
      std::cout << "\x1b[?25l\x1b[2J" << std::flush;
    }
  }

  ~Terminal() {
    if (hasInputTerminal_) {
      tcsetattr(STDIN_FILENO, TCSANOW, &original_);
    }
    if (hasOutputTerminal_) {
      std::cout << "\x1b[?25h\n" << std::flush;
    }
  }

  bool redrawInPlace() const {
    return hasOutputTerminal_;
  }

 private:
  bool hasInputTerminal_;
  bool hasOutputTerminal_;
  termios original_{};
};

bool parseSeed(int argc, char* argv[], bool& seeded, std::uint32_t& seed) {
  if (argc == 1) {
    return true;
  }
  if (argc != 3 || std::string(argv[1]) != "--seed") {
    return false;
  }
  const std::string value = argv[2];
  if (value.empty() ||
      !std::all_of(value.begin(), value.end(), [](unsigned char digit) {
        return std::isdigit(digit) != 0;
      })) {
    return false;
  }
  try {
    const unsigned long long parsed = std::stoull(value);
    if (parsed > std::numeric_limits<std::uint32_t>::max()) {
      return false;
    }
    seed = static_cast<std::uint32_t>(parsed);
    seeded = true;
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

void draw(const avoider::GameState& game, bool inPlace) {
  if (inPlace) {
    std::cout << "\x1b[H" << avoider::render(game) << "\x1b[J"
              << std::flush;
  } else {
    std::cout << '\n' << avoider::render(game) << std::flush;
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  bool seeded = false;
  std::uint32_t seed = 0;
  if (!parseSeed(argc, argv, seeded, seed)) {
    std::cerr << "Usage: " << argv[0] << " [--seed N]\n";
    return 2;
  }

  std::signal(SIGINT, requestStop);
  std::signal(SIGTERM, requestStop);

  try {
    Terminal terminal;
    std::random_device randomDevice;
    std::mt19937 generator;
    std::uniform_int_distribution<int> obstacleChoice(
      0, static_cast<int>(avoider::kObstacleMasks.size()) - 1);
  const auto randomMask = [&]() {
    return avoider::kObstacleMasks[obstacleChoice(generator)];
  };
    const auto newGame = [&]() {
      generator.seed(seeded ? seed : randomDevice());
      std::array<int, 3> initial{};
      for (int& mask : initial) {
        mask = randomMask();
      }
      return avoider::makeGame(initial);
    };

    avoider::GameState game = newGame();
    auto nextTick = std::chrono::steady_clock::now() + kStepTime;
    bool needsDraw = true;
    bool running = true;

    while (running && !stopRequested) {
      if (needsDraw || (terminal.redrawInPlace() && !game.gameOver)) {
        draw(game, terminal.redrawInPlace());
        needsDraw = false;
      }

      pollfd input{STDIN_FILENO, POLLIN, 0};
      const int ready = poll(&input, 1, kFrameTimeMs);
      if (ready < 0 && errno != EINTR) {
        std::cerr << "Input polling failed\n";
        return 1;
      }
      if (ready > 0 && (input.revents & (POLLIN | POLLHUP))) {
        char keys[64];
        const ssize_t count = read(STDIN_FILENO, keys, sizeof(keys));
        if (count == 0) {
          break;
        }
        if (count < 0 && errno != EINTR && errno != EAGAIN) {
          std::cerr << "Input reading failed\n";
          return 1;
        }
        for (ssize_t index = 0; index < count && running; ++index) {
          const char key = keys[index];
          if (key == 'q') {
            running = false;
          } else if (game.gameOver && key == 'r') {
            game = newGame();
            nextTick = std::chrono::steady_clock::now() + kStepTime;
            needsDraw = true;
          } else if (!game.gameOver && key >= '1' && key <= '4') {
            avoider::movePlayer(game, key - '0');
            needsDraw = true;
          }
        }
      }

      if (!game.gameOver && running && !stopRequested &&
          std::chrono::steady_clock::now() >= nextTick) {
        const bool willCollide =
            (game.obstacles[avoider::kRows - 2] &
             (1 << (game.playerColumn - 1))) != 0;
        int spawnMask = avoider::kObstacleMasks[0];
        if (!willCollide &&
            (game.ticks + 1) % avoider::kSpawnEvery == 0) {
          spawnMask = randomMask();
        }
        avoider::advanceTick(game, spawnMask);
        nextTick = std::chrono::steady_clock::now() + kStepTime;
        needsDraw = true;
      }
    }
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
