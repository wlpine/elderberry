# Frametail decorations

Elderberry can use Frametail Lua themes for compositor-owned
titlebars and arbitrary decorations on any window edge. Support is optional and
does not change a normal Elderberry build.

## Build

Install Frametail with its Lua component, then enable the Elderberry feature:

```sh
meson setup build -Dframetail=enabled
meson compile -C build
```

Elderberry discovers `frametail-lua` through pkg-config. Frametail and Elderberry must use
compatible SceneFX and wlroots versions.

## Configure

Add these options to `~/.config/elderberry/config.conf`:

```ini
frametail=1
frametail_theme=/etc/elderberry/frametail-theme.lua
```

`frametail_theme` should be an absolute path. The sample theme is installed with
Elderberry when Frametail support is enabled. Calling `reload_config` loads
the Lua file transactionally and rebuilds decorations for mapped clients. A
theme error leaves the currently loaded theme and decorations active.

The Lua `window` object exposes the client ID, title, app ID, focus, maximized,
and fullscreen state. Elderberry handles these action names from bars and elements:

- `move`
- `resize`
- `minimize`
- `maximize`
- `fullscreen`
- `close`

Frametail replaces Elderberry's normal client border, shadow, and blur while active.
Fullscreen clients hide the decoration. Elderberry's existing group bar is not drawn
for Frametail-decorated clients.

Applications that insist on client-side decorations can still draw their own
header inside the compositor decoration. Use Elderberry window rules to control CSD
policy as needed.
