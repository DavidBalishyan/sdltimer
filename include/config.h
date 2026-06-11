// The configuration header file for sdltimer. You are free to change these values
#ifndef CONFIG_H
#define CONFIG_H

// App info
#define VERSION "1.0"
#define NAME "sdltimer"

// Window sizes
#define WIDTH 640
#define HEIGHT 480

// https://wiki.libsdl.org/SDL3/SDL_WindowFlags
#define WINDOW_FLAGS \
    SDL_WINDOW_RESIZABLE

// Count-up colour-cycle period in seconds (default 360 = 6 min)
#define COUNTUP_CYCLE_SECS 360

#endif
