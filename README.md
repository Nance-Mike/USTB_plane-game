# Plane War (飞机大战)

A classic shoot 'em up game implemented in C++ using the Windows Console API. Features an ASCII-art art style with smooth gameplay, dynamic difficulty, and a complete UI system.

## Project Structure

| File | Description |
|------|-------------|
| `plane_war.cpp` | Basic version with core gameplay |
| `enhanced_plane_war.cpp` | Enhanced version with menus, difficulty scaling, pause, and result screen |
| `plane_war.exe` | Compiled basic version |
| `enhanced_plane_war.exe` | Compiled enhanced version |

## Features

### Enhanced Version (`enhanced_plane_war.cpp`)

- **Multi-page UI** — Main menu, help page, pause overlay, and result screen
- **Dynamic difficulty** — Enemies move faster, spawn more frequently, and shoot more often as your score increases
- **Boundary protection** — Player aircraft cannot fly out of the game field
- **Collision detection** — Pixel-level collision between planes, bullets, and the player
- **Pause system** — Press ESC to pause/resume at any time
- **Score system** — Start with 5 points; +1 per kill, -1 when hit; game ends at 0
- **Replay support** — Restart a new round or return to the main menu after each game

### Basic Version (`plane_war.cpp`)

- Core shooting and movement mechanics
- Simple spawn and collision system
- Win/lose conditions

## How to Build

Requires a Windows environment with a C++ compiler (e.g., MSVC, MinGW).

```bash
# Using g++ (MinGW)
g++ -o plane_war.exe plane_war.cpp
g++ -o enhanced_plane_war.exe enhanced_plane_war.cpp

# Using MSVC (Developer Command Prompt)
cl enhanced_plane_war.cpp
```

> **Note:** The game uses Windows-specific headers (`<windows.h>`, `<conio.h>`) and is not cross-platform.

## How to Run

```bash
./enhanced_plane_war.exe
```

A console window (86x48) will open. Use keyboard controls to play.

## Controls

| Key | Action |
|-----|--------|
| W / A / S / D | Move up / left / down / right |
| Space | Shoot |
| ESC | Pause / Resume |
| Q | Quit current round |
| B | Go back (help screen) |
| 1 / 2 / 3 | Menu selection (Start / Help / Exit) |
| Y / N | Play again / Return to menu (result screen) |

## Game Rules

1. Destroy enemy planes to gain score (+1 per kill).
2. When the player is hit (by enemy bullets or collision), score decreases by 1.
3. Game starts with 5 score points. When score reaches 0, the game ends.
4. As score rises, difficulty increases:
   - Enemies fall faster
   - Enemies spawn more frequently
   - Enemies shoot more often
5. Clear all 50 enemies to win.

## Difficulty Levels

| Level | Score Threshold | Enemy Speed | Spawn Interval | Shoot Chance |
|-------|----------------|-------------|----------------|--------------|
| 1 | 0-6 | Slow | 12 frames | 2% |
| 2 | 7-9 | Medium | 10 frames | 3% |
| 3 | 10-14 | Fast | 8 frames | 4% |
| 4 | 15-19 | Faster | 6 frames | 5% |
| 5 | 20+ | Fastest | 4 frames | 6% |

## Technical Details

- **Rendering**: Double-buffered canvas drawn to console via `SetConsoleCursorPosition` for flicker-free output
- **Game loop**: Fixed timestep at ~50ms per frame (20 FPS)
- **Collision**: Per-pixel bounding-box check between entities
- **OOP design**: `Plane` base class with `PlayerPlane` and `EnemyPlane` subclasses; `Game` class manages all state and logic

## License

This project is for educational purposes.
