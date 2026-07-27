# Elderberry Contributor Notes

## Project Scope

Elderberry is a private, personal fork of Mango/dwl. Optimize changes for the
owner's day-to-day workflow and preferences rather than for broad public API or
downstream compatibility. Keep changes small and avoid carrying compatibility
layers without a concrete local need.

## Codebase

- The compositor is primarily a single C translation unit rooted at
  `src/mango.c`; feature headers under `src/` are included into it.
- Wayland protocol XML files live in `protocols/` and are generated through
  `protocols/meson.build`.
- The optional Lua-configured server-side decorations use Frametail from
  `~/Development/wlpine/frametail` and are integrated by
  `src/frametail-integration.h`.
- The default runtime configuration and Frametail theme live in `assets/`.
- Follow the existing C style and run `./format.sh` on touched C sources when
  practical. Do not reformat unrelated code.

## Build And Verification

Use a fresh Meson build directory when changing build options or protocols:

```sh
meson setup build-dev -Dframetail=enabled
meson compile -C build-dev
```

When Frametail itself changes, build and test
`~/Development/wlpine/frametail` first, then configure Elderberry against its
updated dependency. Exercise decoration changes with multiple outputs, floating
and tiled windows, the scroller layout, and both maximized and fullscreen
states.

## Git Rules

- Never push commits or branches. Commit your changes often.
- Preserve unrelated worktree changes; this fork is commonly developed and
  tested from a dirty tree.
- Do not rewrite or amend existing commits unless explicitly requested.
