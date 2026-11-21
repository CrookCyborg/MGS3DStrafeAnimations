# mgs3danim

3DS-style over-the-shoulder aiming for **METAL GEAR SOLID 3** (Master Collection, PC), as an ASI plugin.

- the camera close behind Snake's shoulder while a weapon is raised; Snake faces where the camera looks and strafes
- walking, crouch walking and crawling animations on his legs while strafing; a crosshair that shots follow
- crouch walking outside the shoulder view, a slow crawl, first-person hand-over with a pad, the classic camera left alone

## Files
- `mgs3danim.c` - the whole mod (one C file, no C runtime)
- `all_settings_reference.ini` - every setting with its built-in value (none of them is needed)
- `build/` - `build.sh` and the import definitions for kernel32 / user32 / gdi32
- `tools/` - tests against a model of the game, and a check of the game exe (see `tools/README.txt`)

## Build
Needs LLVM (clang, lld-link, llvm-dlltool):

    sh build/build.sh

This writes `build/mgs3danim.asi`. Put it and `mgs3danim.ini` into the game's `plugins` folder (an ASI loader is
needed).

