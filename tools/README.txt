mgs3danim - test tools

sim.c        runs the mod's code (../mgs3danim.c) against a small model of the game: camera, Snake, pad, keyboard,
             motions, shots. Each part prints what the mod did next to what is wanted ("want ...").
chk.c        the same tests, plus "install": finds every place in the real game exe that the mod hooks or reads and
             checks that the code there is what the mod expects (run it after a game update).
tests/       the test parts included by sim.c.
motion_info.py  lists the motions in a motion archive (frames, root movement).

Both need two files from your own game installation (they are not included):
  006891cc.mtar           Snake's motion archive
  METAL GEAR SOLID3.exe   the game exe (chk only)

Build and run (Linux or WSL, clang):
  clang -std=gnu2x -O1 -w -fno-builtin sim.c -lm -o sim
  ./sim path/to/006891cc.mtar > sim.out
  clang -std=gnu2x -O1 -w -fno-builtin chk.c -lm -o chk
  ./chk path/to/006891cc.mtar "path/to/METAL GEAR SOLID3.exe" > chk.out
  ./chk path/to/006891cc.mtar "path/to/METAL GEAR SOLID3.exe" install > install.out
