#include <SDL3/SDL.h>
#include "clibx.h"
#include "config.h"
#include "timer.h"

static int parse_seconds(const char *s) {
    if (!s || !*s) return -1;
    int total = 0, cur = 0, has_unit = 0;
    while (*s) {
        if (*s >= '0' && *s <= '9') {
            cur = cur * 10 + (*s - '0');
        } else if (*s == 'h') { total += cur * 3600; cur = 0; has_unit = 1; }
        else if (*s == 'm') { total += cur * 60;    cur = 0; has_unit = 1; }
        else if (*s == 's') { total += cur;          cur = 0; has_unit = 1; }
        else return -1;
        s++;
    }
    total += has_unit ? cur : total ? 0 : cur;
    return total > 0 ? total : -1;
}

static void print_help(void) {
    printf("Usage: " NAME " [TIMER_DURATION]\n");
    printf("\n");
    printf("Opens an SDL3 window with a 7-segment countdown display.\n");
    printf("Without an argument the timer counts up indefinitely.\n");
    printf("\n");
    printf("TIMER_DURATION can be:\n");
    printf("  a plain number   treated as seconds  (e.g. 90)\n");
    printf("  with units       5m      5 minutes\n");
    printf("                   90s      90 seconds\n");
    printf("                   1h       1 hour\n");
    printf("                   1h30m    1h 30m  (= 90m)\n");
    printf("                   1h30m20s 1h 30m 20s\n");
    printf("                   (default: count up)\n");
    printf("\n");
    printf("Controls:\n");
    printf("  SPACE   start / pause\n");
    printf("  R       reset\n");
    printf("  ESC     quit\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help     show this help and exit\n");
    printf("  -v, --version  show version and exit\n");
}

static void print_version(void) {
    printf(NAME " " VERSION "\n");
}

int main(int argc, char *argv[]) {
    int seconds = 0;

    if (argc > 1) {
        if (STREQ(argv[1], "--help") || STREQ(argv[1], "-h")) {
            print_help();
            return EXIT_SUCCESS;
        }
        if (STREQ(argv[1], "--version") || STREQ(argv[1], "-v")) {
            print_version();
            return EXIT_SUCCESS;
        }
        seconds = parse_seconds(argv[1]);
        if (seconds <= 0) {
            fprintf(stderr, "error: invalid duration '%s'\n", argv[1]);
            fprintf(stderr, "Try '" NAME " --help' for usage.\n");
            return EXIT_FAILURE;
        }
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        ERROR("SDL_Init: %s", SDL_GetError());
    }

    SDL_Window *win = SDL_CreateWindow(NAME, WIDTH, HEIGHT, WINDOW_FLAGS);
    if (!win) {
        ERROR("SDL_CreateWindow: %s", SDL_GetError());
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, NULL);
    if (!ren) {
        ERROR("SDL_CreateRenderer: %s", SDL_GetError());
    }

    Timer timer;
    Timer_Init(&timer, seconds);
    if (seconds > 0)
        LOG(NAME " set for %d seconds. SPACE=start/pause  R=reset  ESC=quit", seconds);
    else
        LOG(NAME " counting up. SPACE=start/pause  R=reset  ESC=quit");

    clibx_bool quit = clibx_false;
    int prev_state = -1;

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            } else if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat) {
                if (e.key.key == SDLK_SPACE) {
                    Timer_Toggle(&timer);
                } else if (e.key.key == SDLK_R) {
                    Timer_Reset(&timer);
                } else if (e.key.key == SDLK_ESCAPE) {
                    quit = true;
                }
            }
        }

        Timer_Update(&timer);

        int ww, wh;
        SDL_GetWindowSize(win, &ww, &wh);

        Timer_Render(ren, &timer, ww, wh);

        SDL_RenderPresent(ren);

        if (timer.finished && prev_state != 2) {
            SDL_SetWindowTitle(win, NAME " - FINISHED!");
            prev_state = 2;
        } else if (!timer.running && timer.start_time != 0 && !timer.finished && prev_state != 1) {
            SDL_SetWindowTitle(win, NAME " - PAUSED");
            prev_state = 1;
        } else if (timer.running && prev_state != 0) {
            SDL_SetWindowTitle(win, NAME);
            prev_state = 0;
        }

        SDL_Delay(16);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    LOG("Goodbye!");
    return EXIT_SUCCESS;
}
