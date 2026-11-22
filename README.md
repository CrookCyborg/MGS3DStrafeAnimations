# MGS3DStrafeAnimations

3DS-style over-the-shoulder aiming and strafe animations for **METAL GEAR SOLID 3** (Master Collection, PC), as an ASI plugin.

- the camera close behind Snake's shoulder while a weapon is raised; Snake faces where the camera looks and strafes
- walking, crouch walking and crawling animations on his legs while strafing; a crosshair that shots follow
- crouch walking outside the shoulder view, a slow crawl, first-person hand-over with a pad, the classic camera left alone

## Files
- `mgs3danim.c` - the whole mod (one C file, no C runtime)
- `build/` - `build.sh` and the import definitions for kernel32 / user32 / gdi32
- `tools/` - tests against a model of the game, and a check of the game exe (see `tools/README.txt`)

## Build
Needs LLVM (clang, lld-link, llvm-dlltool):

    sh build/build.sh

This writes `build/mgs3danim.asi`. Put it and `mgs3danim.ini` into the game's `plugins` folder (an ASI loader is
needed).

## CREDITS
- cipherxof - MGS3CrouchWalk. The crouch-walk feature in this mod is based on
  MGS3CrouchWalk's design and on the game-code offsets documented in it.
  https://github.com/cipherxof/MGS3CrouchWalk
- Zoft - this mod's strafe, crouch-walk and aiming animations depend on the research done by Zoft into the MTAR animation archive format and the original port
  of the Snake Eater 3D animations to the Master Collection.
- ThirteenAG - Ultimate ASI Loader.
