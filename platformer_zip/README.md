# Platform Quest

A terminal-based 2D platformer game written in C++.

## Description
Platform Quest is a side-scrolling platformer where the player navigates three
increasingly difficult levels, collecting all coins to advance. Enemies can be
defeated by jumping on them (stomping). The game tracks a persistent leaderboard
saved to `scores.txt`.

## How to Compile and Run
```bash
make
./platformer
```
Requires a C++17-capable compiler (g++ or clang++).

## Controls
| Key | Action |
|-----|--------|
| A / D | Move left / right |
| W or Space | Jump |
| P | Pause |
| Q | Quit |

## Features
- 3 levels with increasing difficulty
- 3 platform types: solid, moving, breakable
- 2 enemy types: walker, jumper
- 3 coin tiers: bronze (10), silver (25), gold (50)
- Persistent top-10 leaderboard (scores.txt)
- ASCII renderer with HUD popup messages
- Lives and health system

## Data Structures
| Structure | Where Used | Reason |
|-----------|-----------|--------|
| `std::vector<Platform>` | Level | Dynamic list of platforms; O(1) append |
| `std::vector<Enemy>` | Level | Enemies removed at runtime via `erase-remove` |
| `std::vector<Coin>` | Level | Iterated every frame; random access needed |
| `std::queue<string>` | Level | FIFO event messages for HUD popups |
| `std::vector<int>` | Player | Logs coin values collected this run |

## Algorithms
1. **AABB Collision Detection** — O(1) per pair, used in Platform and Enemy
2. **AABB Collision Resolution** — O(1), resolves minimum-penetration axis
3. **Insertion Sort** — O(n) per score insert, keeps leaderboard sorted descending
4. **2D Grid Rasterizer** — O(W×H + N), maps world coords to ASCII screen buffer

## Classes
| Class | Files |
|-------|-------|
| Player | Player.h / Player.cpp |
| Platform | Platform.h / Platform.cpp |
| Enemy | Enemy.h / Enemy.cpp |
| Coin | Coin.h / Coin.cpp |
| Level | Level.h / Level.cpp |
| Leaderboard | Leaderboard.h / Leaderboard.cpp |
| Game | Game.h / Game.cpp |

## Contributors
- [Your Name]
