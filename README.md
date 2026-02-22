# ofxFilters

Real-time webcam effects built with [openFrameworks](https://openframeworks.cc). Effects work as **coordinate remappers** — like analog modular synthesis. Patching a set of transformations together generates a visual result. 

```
webcam ──▶ frame buffer ──▶ effect chain ──▶ [ texture renderer ]
           (60 frames,       remap src              +
            ring buffer)     coords only      [ ascii renderer  ]
                                                     │
                                              on-screen HUD
```

## Effects

| Key | Effect | Params |
|-----|--------|--------|
| `1` | **Wave** — sinusoidal pixel displacement | — |
| `2` | **RGB Split** — per-channel horizontal offset (chromatic aberration) | — |
| `3` | **Slitscan** — each column samples a different past frame | `q`/`a` depth |
| `4` | **Block Displace** — grid-based spatial distortion | `w`/`s` size · `e`/`d` amount |

Multiple effects can be active at once and apply in order.

## ASCII Renderer

| Key | Action |
|-----|--------|
| `5` | Toggle ASCII renderer |
| `6` | Cycle color mode: mono (green) → row-tinted → per-char |
| `m` / `n` | Cell size smaller / larger |
| `,` / `.` | Cycle char sets: standard · sparse · dense · organic |

The ASCII renderer reads from the same processed buffer as the texture renderer and draws on top of it.

## Build

Requires openFrameworks 0.12.x on macOS.

```sh
make RunRelease
```

## License

MIT
