# Changes from upstream

This is the Asciiverse packaging of BSD Trek: Eric Allman's `trek`
(UC Berkeley, 1976), from Kenji Rikitake's bsdtrek
(https://github.com/jj1bdx/bsdtrek, the `upstream` remote), which takes
the FreeBSD and DragonFly BSD version and makes it build on Linux. Our
changes start after upstream's `36c8663` (2017-02-11).

The game itself is unchanged: the original typed-command Star Trek. The
same game in Mike Sharov's full-screen rework, without the Star Trek
names, is in the catalog as Space Pirate Hunt (`spirhunt`, BSD Games).

## Code

- **Modern compilers.** `trek.h` defined the shared globals in every
  file, which only linked while compilers merged such definitions
  (`-fcommon`); it now declares them and `externs.c` defines them.
- **Saved games survive updates.** `dump` wrote `trek.dump` to the
  current directory, which for an installed game is the install folder,
  deleted on update. It now goes to `bsdtrek/trek.dump` under
  `$XDG_DATA_HOME`, `~/.local/share` or `%APPDATA%`.
- **Restoring works again.** The dump tags each data area with its memory
  address, and `restart` refused unless the addresses matched; programs
  now load at a different address every run, so restoring never worked
  on a current system. The tags are no longer compared, and the two
  pointers inside the saved data (the event list and the ship's name) are
  moved to this run's addresses. The dump format is unchanged.
- **Windows.** Builds with MinGW: a plain compiler check replaces glibc's
  `__GNUC_PREREQ`, and `rand()` stands in for BSD `random()`.

## Packaging (new files)

- `LICENSE`: the BSD licence from the source headers (the Regents of the
  University of California, 1980, 1993).
- `asciiverse/build.sh`, `asciiverse/package.sh`: one compiler call, and
  one archive per target with the man page, LICENSE and README.md.
- `asciiverse/casts/bsdtrek.cast`: the preview, 100x30.
- `.github/workflows/release.yml`: static Linux in Alpine (x86_64,
  aarch64), macOS (x86_64, arm64) and Windows x86_64 (MinGW), plus
  SHA256SUMS, on a tag.
