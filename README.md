# meowdo – a cat todo list for your terminal

meowdo is a cute, keyboard-driven todo list with a cat sidekick.  
It runs in your terminal, supports tags, search, pinning, and saves everything in `~/.meowdo/todos.txt`.

![SmartStatus screenshot](Meowdo.png)
*(A colorful ncurses UI with a cat and tasks)*

## Features

- **tasks** – add, edit, delete, mark done
- **pinning** – keep important tasks on top
- **tags** – assign tags, filter by 1–6, color-coded
- **search** – live filtering with `/`
- **progress bar** – shows how many tasks are done
- **freaky cat** – changes mood depending on pending tasks
- **celebration** – all tasks done? kitty party!
- **persistent** – data saved to `~/.meowdo/todos.txt`
- **vim-like keys** – `j`/`k`, `g`/`G`, `PgUp`/`PgDn`

## Dependencies

- `ncurses` (development libraries)
- C compiler (`gcc`, `clang`, …)
- `make`

### Install ncurses (examples)

| OS | Command |
|----|---------|
| Arch Linux | `sudo pacman -S ncurses` |
| Debian/Ubuntu | `sudo apt install libncurses-dev` |
| macOS (Homebrew) | `brew install ncurses` |
| Fedora | `sudo dnf install ncurses-devel` |

## Compilation

Simply run `make` in the project folder:

```bash
make
```

If using the Nix flake, run:

```bash
nix build .
```

This will compile the program using the provided `Makefile`.  
**Do not run as root** – the program writes to `~/.meowdo/`.

## Usage

```bash
./meowdo
```

## Default Key Bindings

| Key | Action |
|-----|--------|
| `j` / `↓` | move down |
| `k` / `↑` | move up |
| `g` / `G` | jump to top / bottom |
| `PgUp` / `PgDn` | page up / down |
| `n` | new task |
| `e` | edit selected task |
| `Space` | toggle done |
| `p` | toggle pin |
| `t` | set / clear tag |
| `d` | delete selected task |
| `D` | delete ALL tasks |
| `/` | search |
| `Esc` | clear search + tag filter |
| `1` … `6` | filter by tag (color-coded) |
| `0` | show all tasks |
| `q` | quit |

## Data Storage

- **Directory:** `~/.meowdo/`
- **File:** `todos.txt` – plain text, editable (but careful with the format)

### Line format:

```
<P|-|<x| >|<tag>|<text>|<created_ts>|<done_ts>
```

Example:
```
P|x|work|review PR|1678896000|1678899600
-| |home|buy cat food|1678896000|0
```

## Notes

- Tags are case-insensitive (stored lowercased)
- Maximum 1024 todos (increase `MAX_TODOS` if needed)

## About

meowdo is a cute, keyboard-driven todo list with a cat sidekick.  
It lives in your terminal, supports tags, search, pinning, and remembers everything in `~/.meowdo/todos.txt`.
