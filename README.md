# Clido

Command line todo application written in C++.

## Build

```bash
g++ -std=c++17 -O2 -o todo src/todo.cpp
```

## Usage

```bash
./todo add "Buy milk"
./todo list
./todo done 1
./todo remove 1
./todo clear
./todo help
```

Tasks are stored in `~/.clido/tasks.db` so they persist between runs.
