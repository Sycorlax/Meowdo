Here's the updated README with both badges:

```markdown
# meowdo – a cat todo list for your terminal

[![CI](https://github.com/Sycorlax/Meowdo/actions/workflows/build.yml/badge.svg)](https://github.com/Sycorlax/Meowdo/actions/workflows/build.yml)
[![License: GPL-3.0](https://img.shields.io/badge/License-GPL%203.0-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

meowdo is a cute, keyboard-driven todo list with a cat sidekick.  
It runs in your terminal, supports tags, search, pinning, and saves everything in `~/.local/share/meowdo/todos.txt`.

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
- **persistent** – data saved to `~/.local/share/meowdo/todos.txt`
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

### Build

```bash
git clone https://github.com/Sycorlax/Meowdo.git
cd Meowdo
make
```

If using the Nix flake, you will first need to [install Nix](https://nixos.org/download/) and enable Nix flakes.

If not on NixOS, add this to `~/.config/nix/nix.conf`:

```nix
experimental-features = nix-command flakes
```

And then run `nix build .` in the project folder.

If on NixOS, add this to your configuration:

```nix
nix.settings.experimental-features = [
    "nix-command"
    "flakes"
];
```

And rebuild, then run `nix build .` in the project folder.

This will compile the program using the provided `Makefile`.  
**Do not run as root** – the program writes to `~/.local/share/meowdo/`.

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

- **Directory:** `~/.local/share/meowdo/`
- **File:** `todos.txt` – plain text, editable (but careful with the format)

### Line format:

```
<P|->|<x| >|<tag>|<text>|<created_ts>|<done_ts>
```

Example:
```
P|x|work|review PR|1678896000|1678899600
-| |home|buy cat food|1678896000|0
```

## Notes

- Tags are case-insensitive (stored lowercased)
- Maximum 1024 todos (increase `MAX_TODOS` if needed)

## Troubleshooting

### 🐱 I don't see the cat, just weird characters!

If your cat looks like `\xe2\xa0\x80\xe2\xa0\x80...` or random symbols, your terminal isn't set to UTF-8 mode. The cat art uses Unicode braille characters that require UTF-8 encoding.

**Quick fix:**

```bash
# Set UTF-8 locale for this session
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8

# Run meowdo again
./meowdo
```

**Permanent fix** (add to your `~/.bashrc` or `~/.zshrc`):

```bash
echo 'export LANG=en_US.UTF-8' >> ~/.bashrc
echo 'export LC_ALL=en_US.UTF-8' >> ~/.bashrc
source ~/.bashrc
```

**If locale is missing** (Debian/Ubuntu):

```bash
sudo dpkg-reconfigure locales
# Select en_US.UTF-8 from the list
```

**Test if UTF-8 is working:**

```bash
echo "🐱"
```

If you see a cat emoji, you're good! If you see `\xf0\x9f\x90\xb1`, UTF-8 is not enabled.

### 🖥️ Terminal still not showing the cat?

Some minimal terminal emulators don't support Unicode braille characters. Try using:
- **Linux:** GNOME Terminal, Konsole, Alacritty, Kitty
- **macOS:** Terminal.app, iTerm2
- **Windows:** Windows Terminal, WSL with Windows Terminal

### ❌ "ncurses.h: No such file or directory"

You're missing the ncurses development libraries. See the [Dependencies](#dependencies) section above for installation commands.

### ⚠️ "_XOPEN_SOURCE" redefined warning

This is harmless! The program will still work fine. It's just a compiler warning about a duplicate definition.

## Contributing

Pull requests are welcome! Please ensure:
- Code compiles without warnings (`-Wall -Wextra`)
- The GitHub Actions build passes

## License

[GPL-3.0](LICENSE) – see LICENSE file for details.

---

Made with 💖 and 😺
```
