#include "timer.h"
#include "config.h"

void Timer_Init(Timer *t, int seconds) {
    t->count_up = (seconds == 0);
    t->total_ms = seconds * 1000;
    t->remaining_ms = t->count_up ? 0 : t->total_ms;
    t->running = clibx_false;
    t->finished = !t->count_up && seconds <= 0;
    t->start_time = 0;
    t->paused_duration = 0;
    t->pause_start = 0;
}

void Timer_Toggle(Timer *t) {
    if (!t->count_up && t->finished) return;
    t->running = !t->running;
    if (t->running) {
        if (t->start_time == 0) {
            t->start_time = SDL_GetTicks();
        } else {
            t->paused_duration += SDL_GetTicks() - t->pause_start;
        }
    } else {
        t->pause_start = SDL_GetTicks();
    }
}

void Timer_Reset(Timer *t) {
    t->remaining_ms = t->count_up ? 0 : t->total_ms;
    t->running = clibx_false;
    t->finished = clibx_false;
    t->start_time = 0;
    t->paused_duration = 0;
    t->pause_start = 0;
}

void Timer_Update(Timer *t) {
    if (t->finished || !t->running) return;
    Uint64 now = SDL_GetTicks();
    Uint64 elapsed = (now - t->start_time) - t->paused_duration;
    if (t->count_up) {
        t->remaining_ms = (int)elapsed;
    } else if (elapsed >= (Uint64)t->total_ms) {
        t->remaining_ms = 0;
        t->running = clibx_false;
        t->finished = clibx_true;
    } else {
        t->remaining_ms = t->total_ms - (int)elapsed;
    }
}

static const unsigned char SEG[10] = {
    0b1111110, 0b0110000, 0b1101101, 0b1111001,
    0b0110011, 0b1011011, 0b1011111, 0b1110000,
    0b1111111, 0b1111011,
};

static void draw_digit(SDL_Renderer *r, float x, float y, float w, float h, int d, Uint8 rc, Uint8 gc, Uint8 bc) {
    if (d < 0 || d > 9) return;
    unsigned char p = SEG[d];
    float t = h / 10.0f;
    if (t < 1.5f) t = 1.5f;

    SDL_SetRenderDrawColor(r, rc, gc, bc, 255);

    if (p & 0b1000000) { SDL_FRect a = { x + t, y, w - 2*t, t }; SDL_RenderFillRect(r, &a); }
    if (p & 0b0100000) { SDL_FRect b = { x + w - t, y + t, t, (h - 3*t) / 2 }; SDL_RenderFillRect(r, &b); }
    if (p & 0b0010000) { SDL_FRect c = { x + w - t, y + (h + t) / 2, t, (h - 3*t) / 2 }; SDL_RenderFillRect(r, &c); }
    if (p & 0b0001000) { SDL_FRect d = { x + t, y + h - t, w - 2*t, t }; SDL_RenderFillRect(r, &d); }
    if (p & 0b0000100) { SDL_FRect e = { x, y + (h + t) / 2, t, (h - 3*t) / 2 }; SDL_RenderFillRect(r, &e); }
    if (p & 0b0000010) { SDL_FRect f = { x, y + t, t, (h - 3*t) / 2 }; SDL_RenderFillRect(r, &f); }
    if (p & 0b0000001) { SDL_FRect g = { x + t, y + (h - t) / 2, w - 2*t, t }; SDL_RenderFillRect(r, &g); }
}

void Timer_Render(SDL_Renderer *r, Timer *t, int win_w, int win_h) {
    if (win_w < 100 || win_h < 100) {
        SDL_SetRenderDrawColor(r, 10, 10, 10, 255);
        SDL_RenderClear(r);
        return;
    }

    int remaining = MAX(0, t->remaining_ms / 1000);
    int hours = remaining / 3600;
    int mins = (remaining % 3600) / 60;
    int secs = remaining % 60;

    int ndigits, ncolon, digits[6];
    if (hours >= 1) {
        ndigits = 6; ncolon = 2;
        digits[0] = hours / 10; digits[1] = hours % 10;
        digits[2] = mins / 10;  digits[3] = mins % 10;
        digits[4] = secs / 10;  digits[5] = secs % 10;
    } else {
        ndigits = 4; ncolon = 1;
        digits[0] = mins / 10;  digits[1] = mins % 10;
        digits[2] = secs / 10;  digits[3] = secs % 10;
    }

    float dh = win_h * 0.6f;
    float dw = dh * 0.5f;
    float gap = dw * 0.25f;
    float colon_w = dw * 0.5f;
    float total_w = ndigits * dw + (ndigits - 1) * gap + ncolon * colon_w;

    if (total_w > win_w * 0.85f) {
        float s = (win_w * 0.85f) / total_w;
        dw *= s; dh *= s; gap *= s; colon_w *= s;
        total_w = win_w * 0.85f;
    }

    float start_x = (win_w - total_w) * 0.5f;
    float start_y = (win_h - dh) * 0.5f;

    Uint8 rc, gc, bc, br, bg, bb;
    if (t->finished) {
        clibx_bool flash = (SDL_GetTicks() / 500) & 1;
        if (flash) { rc = 40;  gc = 255; bc = 40;  br = 0;  bg = 40; bb = 0;  }
        else       { rc = 0;   gc = 60;  bc = 0;   br = 0;  bg = 10; bb = 0;  }
    } else if (t->running) {
        int rv, gv;
        if (t->count_up) {
            int phase = (t->remaining_ms / 1000) % COUNTUP_CYCLE_SECS;
            int third = COUNTUP_CYCLE_SECS / 3;
            if (phase < third) {
                int p = phase * 255 / (third - 1);
                rv = p; gv = 255;
            } else if (phase < third * 2) {
                int p = (phase - third) * 255 / (third - 1);
                rv = 255; gv = 255 - p;
            } else {
                int p = (phase - third * 2) * 255 / (third - 1);
                rv = 255 - p; gv = p;
            }
        } else {
            int frac = t->total_ms ? (int)((Uint64)t->remaining_ms * 255 / t->total_ms) : 0;
            if (frac > 127) {
                int p = (frac - 128) * 2;
                rv = 255 - p; gv = 255;
            } else {
                int p = frac * 2;
                rv = 255; gv = p;
            }
        }
        rc = rv; gc = gv; bc = 0;
        br = rv / 10; bg = gv / 10; bb = 0;
    } else {
        rc = 100; gc = 40;  bc = 30;
        br = 5;   bg = 5;   bb = 8;
    }

    SDL_SetRenderDrawColor(r, br, bg, bb, 255);
    SDL_RenderClear(r);

    for (int i = 0; i < ndigits; i++) {
        float extra = 0;
        if (ncolon >= 1 && i >= 2) extra += colon_w;
        if (ncolon >= 2 && i >= 4) extra += colon_w;
        float x = start_x + i * (dw + gap) + extra;
        draw_digit(r, x, start_y, dw, dh, digits[i], rc, gc, bc);
    }

    SDL_SetRenderDrawColor(r, rc, gc, bc, 255);
    if (t->finished || t->running || t->start_time != 0) {
        clibx_bool show = t->finished || !t->running || (secs % 2 == 0);
        if (show) {
            float ds = dw * 0.10f;
            for (int c = 0; c < ncolon; c++) {
                int ci = c == 0 ? 2 : 4;
                float cx = start_x + ci * (dw + gap) + c * colon_w + colon_w * 0.5f;
                SDL_FRect td = { cx - ds * 0.5f, start_y + dh * 0.26f, ds, ds };
                SDL_FRect bd = { cx - ds * 0.5f, start_y + dh * 0.74f - ds, ds, ds };
                SDL_RenderFillRect(r, &td);
                SDL_RenderFillRect(r, &bd);
            }
        }
    }
}
