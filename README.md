# Sweepy

An accurate, lightweight, cross-platform, open-source re-implementation of Windows Minesweeper, written in C and built on SDL.

## Building

SDL2 must be installed on your system first before building. Then just run `make` in the root directory.

## Hotkeys
```
r: Reset the game
b: Choose beginner difficulty (before first click)
i: Choose intermediate difficulty (before first click)
e: Choose expert difficulty (before first click)
-: Scale down
=: Scale up
```

## To do
* Display negative numbers in the mine counter
* Make the timer work
* Fix flood_fill bug with flags that overwrites mines and numbers
* Hotkey to toggle (?) markers
* Allow custom field size and mine count
* Double check multiselect behaviour accuracy
* Add some status text when changing options
* Persist options
* Add persistent high scores
* Find and fix rare segfault
