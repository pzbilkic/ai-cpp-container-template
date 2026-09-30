#!/usr/bin/env bash
set -eu

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

if ! command -v g++ >/dev/null 2>&1; then
  echo "g++ is required. On Ubuntu, install it with: sudo apt-get install g++" >&2
  exit 1
fi

mkdir -p "$script_dir/build"
g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 \
  "$script_dir/main.cpp" "$script_dir/game.cpp" \
  -o "$script_dir/build/obstacle-avoider"

exec "$script_dir/build/obstacle-avoider" "$@"

