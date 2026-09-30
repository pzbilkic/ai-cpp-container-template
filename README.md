#Agentic Running Game
## Petar Bilkic
### Run "./run-game.sh" in CLI for game to start.









# cpp-container-template

## Getting Started

This repository is compatible with [cpp-container](https://github.com/ChicoState/cpp-container). If not already built on your machine, clone and build it.

Run the container:

```bash
docker run -v "$(pwd)":/usr/src -it cpp-container
```

Run the application interactively in a shell:

```bash
docker run -v "$(pwd)":/usr/src -it cpp-container sh
```

## Structure

* `.agents` - AI agent configurations and skills (in `/skills` subdirectory) for this project
* `.` - The root directory contains the C++ code for the application as well as necessary scripts
* `specs` - Specification documentation
* `tests` - Test code

## Four-Column Obstacle Avoider

Open a WSL terminal in this repository and run:

    ./run-game.sh

The script builds the game with g++ and launches it. The compiled executable is
saved at build/obstacle-avoider, so you can run it again without rebuilding:

    ./build/obstacle-avoider

If g++ is missing on Ubuntu, install it with sudo apt-get install g++.
For a repeatable obstacle sequence, run ./run-game.sh --seed 42.

Press 1, 2, 3, or 4 to change columns immediately; do not press Enter.
Obstacles move down automatically while the display redraws in place. Each
obstacle row blocks two or three columns, leaving one or two safe lanes. Earn
a point for each row that passes you. Press q to quit, or r after a collision
to restart. Ctrl+C also exits and restores the terminal.

To compile and run the focused tests:

    mkdir -p build
    g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -I. tests/test_game.cpp game.cpp -o build/test_game
    ./build/test_game
