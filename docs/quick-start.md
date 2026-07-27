---
title: Quick Start
description: Basic configuration and first steps with Elderberry.
---

Now that you have Elderberry installed, let's get your environment set up.

## Initial Setup

1. **Create Configuration Directory**

   Elderberry looks for configuration files in `~/.config/elderberry/`.

   ```bash
   mkdir -p ~/.config/elderberry
   ```

2. **Copy Default Config**

   A default configuration file is provided at `/etc/elderberry/config.conf`. Copy it to your local directory to start customizing.

   ```bash
   cp /etc/elderberry/config.conf ~/.config/elderberry/config.conf
   ```

3. **Launch Elderberry**

   You can now start the compositor from your TTY.

   ```bash
   elderberry
   ```

   Optional: To specify a custom config file path:

   ```bash
   elderberry -c /path/to/your/config.conf
   ```

## Essential Keybindings

Elderberry uses the following keybinds by default:

| Key Combination | Action |
| :--- | :--- |
| `Alt` + `Return` | Open Terminal (defaults to `foot`) |
| `Alt` + `Space` | Open Launcher (defaults to `rofi`) |
| `Alt` + `Q` | Close (Kill) the active window |
| `Super` + `M` | Quit Elderberry |
| `Super` + `F` | Toggle Fullscreen |
| `Alt` + `Arrow Keys` | Move focus (Left, Right, Up, Down) |
| `Ctrl` + `1-9` | Switch to Tag 1-9 |
| `Alt` + `1-9` | Move window to Tag 1-9 |

> **Warning:** Some default bindings rely on specific tools like `foot` (terminal) and `rofi` (launcher). Ensure you have them installed or update your `config.conf` to use your preferred alternatives.

## Recommended Tools

To get a fully functional desktop experience, we recommend installing the following components:

| Category | Recommended Tools |
| :--- | :--- |
| Application Launcher | rofi, bemenu, wmenu, fuzzel |
| Terminal Emulator | foot, wezterm, alacritty, kitty, ghostty |
| Status Bar | waybar, eww, quickshell, ags |
| Desktop Shell | Noctalia, DankMaterialShell |
| Wallpaper Setup | awww(swww), swaybg |
| Notification Daemon | swaync, dunst, mako |
| Desktop Portal | xdg-desktop-portal, xdg-desktop-portal-wlr, xdg-desktop-portal-gtk |
| Clipboard | wl-clipboard, wl-clip-persist, cliphist |
| Gamma Control / Night Light | wlsunset, gammastep |
| Miscellaneous | xfce-polkit, wlogout |

## Example Configuration

The upstream [Mango example configuration](https://github.com/DreamMaoMao/mango-config) is a useful starting point for Elderberry, Waybar, Rofi, and related tools.

```bash
git clone https://github.com/DreamMaoMao/mango-config.git ~/.config/elderberry
```

## Next Steps

Now that you are up and running, dive deeper into customizing Elderberry:

- [Configure Monitors](/docs/configuration/monitors) — Set up resolution, scaling, and multi-monitor layouts.
- [Window Rules](/docs/window-management/rules#window-rules) — Define how specific applications should behave.
- [Appearance](/docs/visuals/theming) — Customize colors, borders, gaps, and effects.
