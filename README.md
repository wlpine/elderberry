<div align="center">
  <h1>Elderberry Wayland Compositor</h1>

  <p>A fast, feature-rich Wayland compositor built on <a href="https://github.com/mangowm/mango">Mango</a></p>
  <p>...which itself was built on <a href="https://codeberg.org/dwl/dwl">dwl</a></p>

<img src="https://img.shields.io/badge/license-GPL--3.0-blue?style=flat" alt="License"/>

</div>

Elderberry is a fork of [Mango](https://github.com/mangowm/mango). It retains
Mango's lightweight compositor foundation while developing and testing its own
window-management and decoration behavior.

This project entirely started as a pet project of mine paired with [frametail](https://github.com/wlpine/frametail) to mess around with AI-powered development and add some niche features that I don't think anybody else would appreciate.

---

### Build From Source

```bash
meson setup --wipe build
meson compile -C build
sudo meson install -C build
```

`--wipe` recreates Meson's build metadata, which may otherwise retain the old
Mango checkout path after renaming or moving the source directory.

The upstream [Mango installation guide](https://mangowm.github.io/docs/installation)
lists the dependencies needed on common distributions.

## Acknowledgements

- [wlroots](https://gitlab.freedesktop.org/wlroots/wlroots) — Wayland protocol implementation
- [Mango](https://github.com/mangowm/mango) — Elderberry's upstream project
- [dwl](https://codeberg.org/dwl/dwl) — Mango's foundation
- [scenefx](https://github.com/wlrfx/scenefx) — window effects library
- [owl](https://github.com/dqrk0jeste/owl) — animation groundwork
- [sway](https://github.com/swaywm/sway) — protocol reference
