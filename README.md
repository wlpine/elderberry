<div align="center">
  <h1>Elderberry Wayland Compositor</h1>

  <p>A fast, feature-rich Wayland compositor built on <a href="https://codeberg.org/dwl/dwl">dwl</a></p>

<img src="https://img.shields.io/badge/license-GPL--3.0-blue?style=flat" alt="License"/>

</div>

Elderberry is a fork of [Mango](https://github.com/mangowm/mango). It retains
Mango's lightweight compositor foundation while developing and testing its own
window-management and decoration behavior.

---

https://github.com/user-attachments/assets/bb83004a-0563-4b48-ad89-6461a9b78b1f

> See all layouts in action at [mangowm.github.io](https://mangowm.github.io/)

## Why Elderberry?

Elderberry starts from Mango, which builds on dwl. It keeps the lightweight,
fast-build philosophy while adding practical desktop features.

- **Lightweight & fast** — as lean as dwl, builds in seconds, no functionality compromised
- **Excellent xwayland support** — run X11 apps without friction
- **Tags, not workspaces** — each tag maintains its own independent window layout
- **Smooth animations** — window open/move/close, tag transitions, layer surfaces
- **Flexible layouts** — scroller, master-stack, monocle, dwindle, grid, and more
- **Rich window states** — swallow, minimize, maximize, global, overlay, fakefullscreen
- **Window effects** — blur, shadow, corner radius, opacity (via scenefx)
- **Excellent input method support** — text-input v2/v3
- **Sway-like scratchpad** — named scratchpad support included
- **Hycov-style overview** — see all windows at a glance
- **IPC** — send/receive messages from external programs
- **Hot-reload config** — no restart needed for keybinding changes
- **Zero flickering** — every frame is correct

## Vision

**Testing first.** Elderberry currently prioritizes finding and fixing behavior
that affects daily use.

**Practicality over novelty.** Features get added when they genuinely improve daily workflows — not for the sake of completeness.

**Focused scope.** Niche requests are evaluated by community interest. Significant upvotes move things forward.

## Installation

[![Packaging status](https://repology.org/badge/vertical-allrepos/mangowm.svg)](https://repology.org/project/mangowm/versions)

### Build From Source

```bash
meson setup --wipe build
meson compile -C build
sudo meson install -C build
```

`--wipe` recreates Meson's build metadata, which may otherwise retain the old
Mango checkout path after renaming or moving the source directory.

#### use my config
- install dependencies
```
yay -S rofi foot xdg-desktop-portal-wlr swaybg waybar wl-clip-persist cliphist wl-clipboard wlsunset xfce-polkit swaync pamixer wlr-dpms sway-audio-idle-inhibit-git swayidle dimland-git brightnessctl swayosd wlr-randr grim slurp satty swaylock-effects-git wlogout sox
```
- clone config
```
git clone https://github.com/DreamMaoMao/mango-config.git ~/.config/elderberry
```

The upstream [Mango installation guide](https://mangowm.github.io/docs/installation)
lists the dependencies needed on common distributions.

## Documentation

- **[mangowm.github.io](https://mangowm.github.io/)** — website docs with configuration reference, keybindings, layouts, IPC, and more
- **[GitHub Wiki](https://github.com/mangowm/mango/wiki/)** — community-maintained wiki
- **[Frametail decorations](docs/frametail.md)** — optional Lua-configured server-side decorations

## Community

Join us on **[Discord](https://discord.gg/CPjbDxesh5)**

## Acknowledgements

- [wlroots](https://gitlab.freedesktop.org/wlroots/wlroots) — Wayland protocol implementation
- [Mango](https://github.com/mangowm/mango) — Elderberry's upstream project
- [dwl](https://codeberg.org/dwl/dwl) — Mango's foundation
- [scenefx](https://github.com/wlrfx/scenefx) — window effects library
- [owl](https://github.com/dqrk0jeste/owl) — animation groundwork
- [sway](https://github.com/swaywm/sway) — protocol reference

## Sponsor

If Mango makes your desktop better, consider supporting its development.

Thanks to everyone who has sponsored this project:

<table>
  <tr>
    <!-- add new sponsors here: copy the <td>...</td> block below -->
    <td align="center">
      <a href="https://github.com/dl09r">
        <img src="https://unavatar.io/github/dl09r" width="48" style="border-radius:50%"/><br/>
        <sub>dl09r</sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/tonybanters">
        <img src="https://unavatar.io/github/tonybanters" width="48" style="border-radius:50%"/><br/>
        <sub>tonybanters</sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/vinthara">
        <img src="https://unavatar.io/github/vinthara" width="48" style="border-radius:50%"/><br/>
        <sub>vinthara</sub>
      </a>
    </td>
  </tr>
</table>

Crypto donations accepted:

<table>
  <tr>
    <td valign="middle">
      <strong>Network:</strong> BEP20 (BSC)<br/>
      <strong>Address:</strong> <code>0xf9cda472f2556671d2504afc4c35340ec5615da1</code>
    </td>
    <td valign="middle">
      <img width="120" alt="sponsor QR" src="assets/crypto_sponserme_qrcode.png" />
    </td>
  </tr>
</table>
