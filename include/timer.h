#ifndef TIMER_H
#define TIMER_H

#include <SDL3/SDL.h>
#include "clibx.h"

typedef struct {
    int total_ms;
    int remaining_ms;
    clibx_bool running;
    clibx_bool finished;
    clibx_bool count_up;
    Uint64 start_time;
    Uint64 paused_duration;
    Uint64 pause_start;
} Timer;

void Timer_Init(Timer *t, int seconds);
void Timer_Toggle(Timer *t);
void Timer_Reset(Timer *t);
void Timer_Update(Timer *t);
void Timer_Render(SDL_Renderer *r, Timer *t, int win_w, int win_h);

#endif
