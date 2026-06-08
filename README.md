# sdltimer - a simple SDL3 countdown timer

Draws a big 7-segment digital countdown in an SDL3 window. Uses [**clibx**](https://github.com/DavidBalishyan/clibx)
for logging and its boolean type. Renders the whole thing with nothing
but SDL3 rectangles (no fonts or textures needed).

## requirements

  - [SDL3](https://www.libsdl.org) (>= 3.2)
  - [clibx](httpsL//github.com/DavidBalishyan/clibx) (self contained in the source code)
  - a C compiler ([gcc](https://gcc.gnu.org) or [clang](https://clang.llvm.org))
  - [pkg-config](https://gitlab.freedesktop.org/pkg-config/pkg-config)


## building

    make

This puts the .o files under build/ and the sdltimer binary in the
current directory.  To clean up:

    make clean


## installing

    sudo make install

Installs the binary to /usr/local/bin/sdltimer and the man page to
/usr/local/share/man/man1/sdltimer.1.

The install prefix can be overridden:

    make install PREFIX=/usr


## usage

    ./sdltimer [TIMER_DURATION]

Default is 300 seconds (5 minutes).  TIMER_DURATION can be a plain
number (seconds) or a human-readable string with h/m/s suffixes:

    ./sdltimer 90          90 seconds
    ./sdltimer 90s         90 seconds
    ./sdltimer 5m          5 minutes
    ./sdltimer 1h          1 hour
    ./sdltimer 1h30m       1 hour 30 minutes
    ./sdltimer 1h30m20s    1 hour 30 minutes 20 seconds

For help or version info:

    ./sdltimer --help
    ./sdltimer -h
    ./sdltimer --version
    ./sdltimer -v

Once the window opens:

    SPACE   start / pause
    R       reset back to the initial duration
    ESC     quit

The display is red while running, dim amber while paused, and flashes
red when time is up.  The colon blinks once per second.


## files

  main.c          entry point, event loop, window management
  timer.h / .c    Timer struct + logic + 7-segment renderer
  config.h        window size / flags
  Makefile        build + install


## how it works

The Timer struct stores the total duration and tracks elapsed time
with SDL_GetTicks(), subtracting out any pause time.  Each frame the
remaining seconds are split into four digits (MM:SS) and drawn with
the 7-segment patterns in SEG[].  Each segment is a filled rectangle
positioned so all seven fit together without gaps.

When the countdown hits zero the finished flag is set, and the
render loop alternates between bright and dark red every 500 ms.

Both libraries are used at build time via pkg-config; clibx is
header-only, so it costs nothing at link time.


## license

See the [LICENSE](LICENSE) file
