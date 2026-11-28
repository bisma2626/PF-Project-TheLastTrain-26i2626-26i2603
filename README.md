# The Last Train

CS-1002 Programming Fundamentals (AI & DS), Fall 2026 - Semester Project
Group: 26i2626 and 26i2603

A deterministic, text-based rescue game in C++17. There are no classes: only functions, arrays and `int` / `char` / `bool` variables. The player explores a 5-floor, 20-room building, rescues a passenger and a cat, finds a 3-digit cabinet code and prepares the train before Energy or Time runs out.

## Folder structure

```
include/    headers (config, state, level, simulator, physics, switches, logger, util, viewer_iface)
src/core/   source files (main_core, level, simulator, physics, switches, logger, util, viewer_iface_core)
data/       building.txt, initial-state.txt and the test input files
out/        savegame.txt and game_summary.txt (written by the winning test run, test_win.txt)
docs/       Project_Diagram.jpg, Flowchart.png, ConsoleUI.png
```

## Build and run

Run everything from the project root: the program uses the relative paths `data/...` and `out/...`, and the `out` folder must exist.

Linux / macOS / WSL:

```
g++ -std=c++17 -Wall -Wextra -Iinclude src/core/*.cpp -o last_train
./last_train
```

Windows PowerShell (g++ on PATH):

```
g++ -std=c++17 -Wall -Wextra -Iinclude src/core/*.cpp -o last_train.exe
.\last_train.exe
```

The project compiles with no warnings under these flags.

## How to play

First enter a starting floor and room (for example `1 1`). Then type a command letter and press Enter:

| Command | Action |
|---|---|
| `M` | Move: then enter destination floor and room |
| `I` | Inspect the room (lists items, finds clue digits) |
| `C` | Collect an item: then enter its symbol |
| `U` | Use an item: then enter its symbol |
| `K` | Enter the cabinet code (only in F5-R1) |
| `S` / `L` | Save / load `out/savegame.txt` |
| `Q` | Quit (asks for confirmation) |

Invalid or rejected actions cost nothing.

### Rules in short

- Energy starts at 100 and Time at 50. Both are clamped between 0 and their maximum. If either reaches 0 the mission fails.
- Costs (Energy / Time): door or stairs -2/-1, window -3/-2, lift -4/-3, inspect, pickup and use -1/-1.
- Energy Pack +25 Energy, Emergency Toolkit +15 Energy and Observation Token +15 Time apply as soon as they are collected.
- The locked door F3-R1 to F3-R2 needs the Master Key. Lift 1 joins floors 1 and 2 (rooms 2 and 3). Lift 2 joins F2-R4, F4-R4 and F5-R4: it works once the Fuse is installed (F3-R4) and the Lift Pass is collected, or with 3 emergency trips from the Emergency Access Card.
- Clue digits 4, 2 and 7 are found by inspecting F3-R2, F4-R3 and F5-R1. The code `427` opens the cabinet in F5-R1 and releases the Train Key.
- Rescue the passenger (Medical Kit used in F4-R1), secure the cat (Cat Carrier used in F3-R3), then use the Train Key in F5-R2 to prepare the train and win.
- Score: Master Key 10, each clue 10, cabinet 30, Train Key 10, passenger 100, cat 100, fuse 20, lift pass 20, train prepared 100, victory bonus 100 (maximum 520).

### Building map symbols

| Symbol | Meaning | Symbol | Meaning |
|---|---|---|---|
| `D` | Ordinary door | `<` | Master Key |
| `W` | Window | `)` | Energy Pack |
| `L` | Locked door (Master Key) | `\` | Emergency Toolkit |
| `#` | Solid wall | `4` `2` `7` | Cabinet clue digits |
| `S1` `S2` `S3` | Staircases | `*` | Fuse |
| `L1` | Lift 1 (lift shaft, not a walkable door) | `~` | Cat |
| `L2` | Lift 2 | `;` | Control Panel |
| `^` | Cabinet / Train Key | `(` | Lift Pass |
| `!` | Passenger | `%` | Medical Kit |
| `$` | Emergency Access Card | `@` | Cat Carrier |
| `&` | Observation Token | | |

## Test files and expected results

Each test file lists the inputs of a whole session. Run one in PowerShell like this (Linux / macOS: `./last_train < data/test_win.txt`):

```
Get-Content data\test_win.txt | .\last_train.exe
```

| File | What it checks | Expected end of run |
|---|---|---|
| `test_win.txt` | A full winning route (also saves once) | MISSION COMPLETE, score 480, energy 12, time 3 |
| `test_loss.txt` | Running out of Time with repeated lift rides | MISSION FAILED, score 0, energy 32, time 0 |
| `test_save.txt` | Move, save, quit | `out/savegame.txt` starts with `1 2 97 48 0 0 0` |
| `test_load.txt` | Load the save (run after `test_save.txt`) | Floor 1, Room 2, energy 97, time 48 |

## Verifying determinism

The same inputs always give the same output. Run a test twice and compare:

```
Get-Content data\test_win.txt | .\last_train.exe | Out-File out\run1.txt -Encoding ascii
Get-Content data\test_win.txt | .\last_train.exe | Out-File out\run2.txt -Encoding ascii
Compare-Object (Get-Content out\run1.txt) (Get-Content out\run2.txt)
```

No output from `Compare-Object` means the two runs are identical. On Linux / macOS use `diff out/run1.txt out/run2.txt`.

## File formats

- `data/building.txt`: ASCII map, read at start and never overwritten. Each floor has a row of room labels (`F3-R2`) with connection symbols between them and a row below with the item symbols.
- `data/initial-state.txt`: `KEY: VALUE` lines (start values and objective flags). `START_ROOM: PLAYER_SELECTS` means the player picks the start room.
- `out/savegame.txt`: line 1 is floor, room, energy, time, score, inventory count, emergency trips. Line 2 is the inventory symbols. Line 3 is the 16 objective flags (1 or 0). The next 20 lines are one per room (F1-R1 to F5-R4): item count followed by the item symbols still in the room. Loading reads everything into temporary variables first, so a damaged file never changes the current game.
- `out/game_summary.txt`: result (victory, failed or quit), score, remaining Energy and Time, the objective checklist and a session audit.

## Architecture and flow diagrams

- `docs/Project_Diagram.jpg`: input files, source modules and output files.
- `docs/Flowchart.png`: startup, starting-room validation, game loop, command handling, save/load, quit confirmation and final report.

## Changes to the skeleton

`include/state.h`: added three connection constants `CONN_STAIRS`, `CONN_LIFT1` and `CONN_LIFT2`. No function declarations were changed.

## Assumptions

- "Train prepared" needs the Train Key, the rescued passenger and the secured cat. The Fuse and Lift Pass are optional.
- `building.txt` shows one Energy Pack in F1-R4, so the game has one.
- The -1/-1 pickup cost also applies to Energy Packs, the Toolkit and the Token.
- Maximum Time is 50 (as in `initial-state.txt`).
- Lift 1 joins rooms 2 and 3 on floor 1 with rooms 2 and 3 on floor 2. The `L1` marks in the map are lift shafts, not walkable doors.
- The Emergency Access Card gives its 3 trips when collected.