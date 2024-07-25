#include "circle.h"

void drawCircle(SDL_Renderer *rend, int x, int y, int radius)
{
    for (int w = 0; w < radius * 2; w++)
    {
        for (int h = 0; h < radius * 2; h++)
        {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx*dx + dy*dy) < (radius * radius)) {
                SDL_RenderDrawPoint(rend, x + dx, y + dy);
            }
        }
    }
}