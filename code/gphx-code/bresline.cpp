/*
 * bresline.cpp
 *
 *  Created on: Dec 7, 2011
 *      Author: emint
 */

#include "SDL/SDL.h"
#include <iostream>
#include <cmath>

using namespace std;
/**
 * Application window parameters.
 */
#define APP_WIDTH 512
#define APP_HEIGHT 512
#define APP_BPP 32

/**
 * To define a point on screen
 */
typedef struct {
    double x;
    double y;
    bool active;
} pointT;

/**
 * From:
 * http://www.libsdl.org/intro.en/usingvideo.html
 */
void draw_pixel(SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G, Uint8 B) {
  Uint32 color = SDL_MapRGB(screen->format, R, G, B);
  switch (screen->format->BytesPerPixel) {
    case 1: // Assuming 8-bpp
    {
      Uint8 *bufp;
      bufp = (Uint8 *) screen->pixels + y * screen->pitch + x;
      *bufp = color;
    }
      break;
    case 2: // Probably 15-bpp or 16-bpp
    {
      Uint16 *bufp;
      bufp = (Uint16 *) screen->pixels + y * screen->pitch / 2 + x;
      *bufp = color;
    }
      break;
    case 3: // Slow 24-bpp mode, usually not used
    {
      Uint8 *bufp;
      bufp = (Uint8 *) screen->pixels + y * screen->pitch + x * 3;
      if (SDL_BYTEORDER == SDL_LIL_ENDIAN) {
        bufp[0] = color;
        bufp[1] = color >> 8;
        bufp[2] = color >> 16;
      } else {
        bufp[2] = color;
        bufp[1] = color >> 8;
        bufp[0] = color >> 16;
      }
    }
      break;
    case 4: // Probably 32-bpp
    {
      Uint32 *bufp;
      bufp = (Uint32 *) screen->pixels + y * screen->pitch / 4 + x;
      *bufp = color;
    }
      break;
  }
}

void set_point(pointT &point, int x, int y) {
  point.x = x;
  point.y = y;
  point.active = true;
}
double calc_slope(pointT p1, pointT p2) {
  double slope = (p2.y - p1.y) / (p2.x - p1.x);
  return slope;
}

void swap_axis(pointT & p1) {
  double tmp = p1.x;
  p1.x = p1.y;
  p1.y = tmp;
}
pointT find_smaller(pointT p1, pointT p2) {
  if (p1.x < p2.x)
    return p1;
  else
    return p2;
}

pointT find_bigger(pointT p1, pointT p2) {
  if (p1.x > p2.x)
    return p1;
  else
    return p2;
}
/**
 * An implementation of Bresenham's Line Algorithm
 *
 * We don't do any of the integrals only optimizations here.
 */
void draw_line(SDL_Surface* surface, pointT p1, pointT p2) {
  /*
   *
   * The factor we use to determine if we will be incrementing in the
   * non-constant direction
   * */
  const double error_bound = .5;

  /*
   * Whether or not we have to reflect over y=x.
   *
   * This is determined by whether or not the slope will be > 1.
   *
   * Since we want smaller slopes (< 1), reflection helps us achieve this if
   * the desired line does not meet this.
   * */
  bool reflect = abs(p2.y - p1.y) > abs(p2.x - p1.x);

  if (reflect) {
    swap_axis(p1);
    swap_axis(p2);
  }

  double slope = calc_slope(p1, p2);

  /*
   * Calculate whether or not we will be moving up or down
   */
  double step_size = (slope < 0 ? -1 : 1);

  /*
   * To make the accumulation and comparison to error a little more clear
   * we just make the slope the absolute value
   */
  slope = abs(slope);

  /*
   * Set the points so we are always moving towards the bigger
   */
  pointT smaller = find_smaller(p1, p2);
  pointT bigger = find_bigger(p1, p2);

  double slopeAccum = 0;
  double cur_y = smaller.y;

  for (double x_tmp = smaller.x; x_tmp < bigger.x; ++x_tmp) {
    slopeAccum += slope;

    /*
     * Check if we need to move up the non-constant axis
     */
    if (slopeAccum > error_bound) {
      cur_y += step_size;
      slopeAccum -= 1;
    }
    /*
     * Since we might have changed what our 'x' and 'y' mean due to reflection
     * we have to specify which ones there were originally
     */
    if (reflect)
      draw_pixel(surface, cur_y, x_tmp, rand() % 255, rand() % 255,
          rand() % 255);
    else
      draw_pixel(surface, x_tmp, cur_y, rand() % 255, rand() % 255,
          rand() % 255);
  }
}

void draw_lines(SDL_Surface* surface) {
  SDL_Event event;
  srand(time(NULL));
  bool close = false;
  pointT point1;
  pointT point2;
  point1.active = false;
  point2.active = false;
  while (!close) {
    if (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_MOUSEBUTTONUP:
          draw_pixel(surface, event.button.x, event.button.y, rand() % 255,
              rand() % 255, rand() % 255);
          if (!point1.active) {
            set_point(point1, event.button.x, event.button.y);
          } else {
            set_point(point2, event.button.x, event.button.y);
            draw_line(surface, point1, point2);
            point1.active = point2.active = false;
          }
          SDL_Flip(surface);

          break;
        case SDL_QUIT:
          close = true;
          break;
      }
    }
  }
}

int main(int argc, char* argv[]) {
  //Start SDL
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    cout << "Error initializing SDL. " << SDL_GetError();
    return -1;
  }

  SDL_Surface* surface = SDL_SetVideoMode(APP_WIDTH, APP_HEIGHT, APP_BPP,
      SDL_HWSURFACE | SDL_DOUBLEBUF);

  if (!surface) {
    cout << "Error initializing main canvas with size " << APP_WIDTH << "x"
        << APP_HEIGHT << ". Error: " << SDL_GetError();
    return -1;
  }

  draw_lines(surface);
}
