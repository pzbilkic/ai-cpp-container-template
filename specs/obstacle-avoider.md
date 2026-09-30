# Spec: Four-Column CLI Obstacle Avoider

**Status:** Updated for four lanes

## Objective

Build a four-column obstacle avoider for a WSL terminal. Obstacle rows descend automatically while the player changes columns by pressing 1, 2, 3, or 4 without Enter. Redraw the board in place at about 30 frames per second.

## Game Rules

- The board has four columns and 15 text rows. The player starts in column 2 of the bottom row.
- Start with obstacle rows at positions 0, 4, and 8. Each obstacle row blocks either two or three columns, leaving exactly one or two open columns.
- Every 160 milliseconds, move each obstacle row down one text row. Spawn a new obstacle row at the top every fourth movement tick. Empty rows between obstacles remain open.
- A collision occurs when a descending obstacle blocks the player's bottom cell or the player moves into a blocked cell already there. Freeze play and show the final score.
- Score one point when an obstacle row safely passes the bottom. There is no fixed win condition.
- During play, keys 1 through 4 switch directly to that column. q quits. Enter and unrelated keys do nothing. After collision, r restarts and q quits.
- A seeded run with --seed N repeats the same initial obstacle rows and later spawns on the same toolchain. Restart with a seed replays that sequence; unseeded restart uses a fresh seed.
- EOF exits cleanly. Invalid command-line arguments exit nonzero with usage.

## Obstacle Representation

Represent each occupied row with a four-bit mask. Bit 0 blocks column 1 and bit 3 blocks column 4. Valid spawn masks have exactly two or three set bits; zero denotes an empty row. Choose uniformly from the ten valid masks: six with two blocked columns and four with three blocked columns. A choice of any one of the open lanes is always possible because the player can switch directly to any column.

## Terminal Behavior

Use POSIX termios for immediate key input and restore original settings on exit and Ctrl+C. Poll stdin without blocking the timer. Use ANSI cursor positioning only for terminal output; restore the cursor on exit. Nonterminal output remains readable for diagnostics.

## Tech Stack and Commands

C++17, Bash, and g++ in WSL. Preserve the existing Docker and test_runner.sh workflows.

    ./run-game.sh
    ./run-game.sh --seed 42
    ./build/obstacle-avoider

    mkdir -p build
    g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 main.cpp game.cpp -o build/obstacle-avoider
    g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -I. tests/test_game.cpp game.cpp -o build/test_game
    ./build/test_game

The executable run-game.sh builds and runs from any working directory. Generated binaries stay in ignored build/.

## Project Structure

    main.cpp                    Terminal mode, input polling, timer, random masks
    game.hpp                    State and rule declarations
    game.cpp                    Movement, collision, score, rendering
    run-game.sh                 WSL build-and-run launcher
    tests/test_game.cpp         Deterministic game-rule tests
    README.md                   Player controls and launch instructions
    specs/obstacle-avoider.md   This specification

## Code Style

Use two-space indentation, braces, PascalCase types, and camelCase functions and variables. Keep game rules independent of terminal I/O and random number generation.

    StepResult movePlayer(GameState& game, int column) {
      if (column < 1 || column > kColumns) {
        return StepResult::invalidInput;
      }
      // Check the selected column against the bottom row's blocked mask.
    }

## Testing Strategy

Use the dependency-free test executable in tests/test_game.cpp. Inject obstacle masks into movement ticks. Check initial mask placement, every generated mask's one-or-two-open-lane property, spawn cadence, score on a safe pass, collision on descent and lane switch, valid movement to column 4, and invalid columns. Compile with warning flags and play in a WSL pseudo-terminal to verify immediate keys, in-place redraw, and terminal restoration.

## Boundaries

- **Always:** Restore terminal settings and cursor on exit; keep input responsive; generate only valid obstacle masks; preserve repeatable seeded play; run build and tests.
- **Ask first:** Add third-party libraries, replace Docker or test_runner.sh, or change the four-column rule.
- **Never:** Spawn a fully blocked row, commit binaries or secrets, modify .agents skill files for this feature, or let input block obstacle movement.

## Success Criteria

1. ./run-game.sh starts a playable WSL terminal game with four labeled columns and no Enter required.
2. Every occupied obstacle row has one or two open lanes, including the initial rows and later spawns.
3. Obstacles move independently of input, and the board redraws in place.
4. Collision, scoring, restart, quit, EOF, and seed behavior match the rules above.
5. Terminal settings and cursor return to normal after quitting or Ctrl+C.
6. The warning-enabled build and deterministic tests pass.

## Open Questions

None. The user's request to use four columns and block two or three lanes per obstacle row replaces the previous three-column rule.
