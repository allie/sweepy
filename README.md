# Sweepy

<p align="center">
  <img width="388" src="https://i.imgur.com/q6I7MzC.png" alt="Screenshot of Sweepy" />
</p>

An accurate, lightweight, cross-platform, open-source re-implementation of Windows Minesweeper, written in C and built on SDL.

## Building

SDL2 must be installed on your system first before building. Then just run `make` in the root directory. Use `make debug` to compile with debug symbols, or `make prod` to compile with optimization.

## Controls
Mouse:
```
Left: Reveal a tile
Right: Place a flag or (?) marker
Middle / Shift+Left / Left+Right: Clear surrounding tiles
```
Keyboard:
```
r: Reset the game
b: Choose beginner difficulty (before first click)
i: Choose intermediate difficulty (before first click)
e: Choose expert difficulty (before first click)
?: Toggle (?) marks
-: Scale window down
+: Scale window up
Esc: Stop timer
```

## To do
* Allow custom field size and mine count
* Persist options
* Add persistent high scores
* Add WRAPFIELD mode
* Use lower bit depth for textures
* Replace rand() with a better RNG to avoid board cycling
