# About this fork of FreeSynd

![Screenshot](screenshot.png)

## Improvements

Make it playable with modern controls and high-resolution screens.

- Fullscreen by default, preserving aspect ratio as in the original DOS game.
- Configurable scaling. Set for example scale_factor = 3 in user.conf. Default is calculated depending on resolution.
- See more of the map in a zoomed out view, pixel-perfect integer scaled.
- Hold down CTRL or Mouse-wheel to pan the view using the mouse.
- Smooth panning with WASD keys instead of edge-panning.
- Faster menu transitions, without the loud sound from the original.
- Press key 1-4 to select agent. Double press key 1-4 to center on agent.

All credits to the original FreeSynd authors and Bullfrog.

## Developer notes

### Convert data files to lower case
rename.pl -f 'y/A-Z/a-z/' *

### CMake options om macOS
--preset "ninja-clang-arm64-release" -DCONAN_COMMAND=~/Library/Python/3.14/bin/conan

### Go directly to mission with cheats
./FreeSynd -m 8 -c "COOPER TEAM"
