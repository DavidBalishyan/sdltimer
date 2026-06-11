# sdltimer

Draws a big 7-segment digital countdown or count-up in an SDL3 window. Uses [**clibx**](https://github.com/DavidBalishyan/clibx) for logging and its
boolean type.  Renders the whole thing with nothing but SDL3 rectangles (no fonts or textures needed).

## requirements

  - [SDL3](https://www.libsdl.org) (>= 3.2)
  - [clibx](https://github.com/DavidBalishyan/clibx) (self contained in the source code)
  - a C compiler ([gcc](https://gcc.gnu.org) or [clang](https://clang.llvm.org))
  - [pkg-config](https://gitlab.freedesktop.org/pkg-config/pkg-config)

## building

    make

This puts the .o files under build/ and the sdltimer binary in the
current directory. To clean up:

    make clean

## installing

    make install

Installs the binary to /usr/local/bin/sdltimer and the man page to
/usr/local/share/man/man1/sdltimer.1.

The install prefix can be overridden:

    make install PREFIX=/usr

## usage

    sdltimer [TIMER_DURATION]

Without an argument the timer counts up from zero indefinitely.
With an argument it counts down from the given duration:

    sdltimer 90          90 seconds
    sdltimer 90s         90 seconds
    sdltimer 5m          5 minutes
    sdltimer 1h          1 hour
    sdltimer 1h30m       1 hour 30 minutes
    sdltimer 1h30m20s    1 hour 30 minutes 20 seconds

For help or version info:

    sdltimer --help
    sdltimer -h
    sdltimer --version
    sdltimer -v

Once the window opens:

    SPACE   start / pause
    R       reset
    ESC     quit

## colours

  - **Running (countdown)** - smoothly shifts green → yellow → red as time
    drains, giving a visual cue of remaining time.
  - **Running (count-up)** - cycles through the same spectrum; the cycle
    period is configurable via `COUNTUP_CYCLE_SECS` in `config.h`.
  - **Paused** - dim amber.
  - **Finished** - flashing green.

The colon blinks once per second while running. The display shows HH:MM:SS
when an hour or more remains, otherwise MM:SS.

## how it works

The Timer struct stores the total duration (or zero for count-up) and tracks
elapsed time with SDL_GetTicks(), subtracting out any pause time.  Each frame
the display value (remaining or elapsed) is split into digits and drawn with
the 7-segment patterns in SEG[]. Every segment is a filled rectangle
positioned so all seven fit together without gaps.

When a countdown hits zero the finished flag is set and the display flashes
green every 500 ms.

Both libraries are used at build time via pkg-config; clibx is header-only,
so it costs nothing at link time.

## license

See the [LICENSE](LICENSE) file ([GPLv3](https://www.gnu.org/licenses/gpl-3.0.html))
