/*
 * This file is part of the AdvanceMAME project.
 *
 * Copyright (C) 1999-2002 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __DRAW_H
#define __DRAW_H

#include "video.h"
#include "generate.h"
#include "gtf.h"
#include "crtcbag.h"

/***************************************************************************/
/* Colors */

#define COLOR_TITLE 0x0B
#define COLOR_INPUT 0x0F
#define COLOR_BAD 0x0C
#define COLOR_MARK 0x30
#define COLOR_MARK_BAD 0x34
#define COLOR_SELECTED 0x70
#define COLOR_SELECTED_MARK 0x7F

#define COLOR_INFO_NORMAL 0x08
#define COLOR_INFO_TITLE 0x07

/***************************************************************************/
/* Video crtc container extension */

video_crtc* video_crtc_container_pos(video_crtc_container* vmc, unsigned pos);
unsigned video_crtc_container_max(video_crtc_container* vmc);

/***************************************************************************/
/* Sound */

extern int the_sound_flag;

void sound_error(void);
void sound_warn(void);
void sound_signal(void);

/***************************************************************************/
/* Monitor */

/* Video monitor specifications */
extern video_monitor the_monitor;

/* Video mode generator specifications */
extern video_generate_interpolate_set the_interpolate;

extern video_gtf the_gtf;

/***************************************************************************/
/* Text output */

#define DEFAULT_TEXT_MODE "default_text"

extern video_crtc_container the_modes; /* main video mode container */
extern int the_modes_modified; /* container need to be saved */
extern video_mode the_default_mode;  /* Default video mode */
extern int the_default_mode_flag; /* Default video mode set */

unsigned text_size_x(void);
unsigned text_size_y(void);
void text_clear(void);
void text_set_font(void);
int text_video_crtc_compare(const video_crtc* a, const video_crtc* b);
void text_init(void);
void text_reset(void);
void text_done(void);
void text_put(int x, int y, char c, int color);

/***************************************************************************/
/* Draw interface */

#define DRAW_COLOR_BLACK draw_color(0,0,0)
#define DRAW_COLOR_GRAY draw_color(196,196,196)
#define DRAW_COLOR_WHITE draw_color(255,255,255)
#define DRAW_COLOR_RED draw_color(255,0,0)
#define DRAW_COLOR_GREEN draw_color(0,255,0)
#define DRAW_COLOR_BLUE draw_color(0,0,255)

unsigned draw_color(unsigned r, unsigned g, unsigned b);
void draw_char(int x, int y, char c, unsigned color);
void draw_string(int x, int y, const char* s, unsigned color);
void draw_text_fill(int x, int y, char c, int dx, unsigned color);
void draw_text_fillrect(int x, int y, char c, int dx, int dy, unsigned color);
void draw_text_left(int x, int y, int dx, const char* s, unsigned color);
void draw_text_center(int x, int y, int dx, const char* s, unsigned color);
unsigned draw_text_string(int x, int y, const char* s, unsigned color);
int draw_text_para(int x, int y, int dx, int dy, const char* s, unsigned color);
int draw_text_read(int x, int y, char* s, int dx, unsigned color);
void draw_graphics_palette(void);
void draw_graphics_animate(int s_x, int s_y, int s_dx, int s_dy, unsigned counter, int do_clear);
void draw_graphics_out_of_screen(int do_clear);
void draw_graphics_calib(int s_x, int s_y, int s_dx, int s_dy);
unsigned draw_graphics_speed(int s_x, int s_y, int s_dx, int s_dy);
void draw_test_default(void);
void draw_graphics_clear(void);

/***************************************************************************/
/* Menu */

#define COLOR_BAR 0x70
#define COLOR_ERROR 0x4F
#define COLOR_REVERSE 0x70
#define COLOR_NORMAL 0x03
#define COLOR_BOLD 0x0F
#define COLOR_LOW 0x08

typedef void (*entry_print)(int x, int y, int dx, void* data, int n, int selected);

typedef int (*entry_separator)(void* data, int n);

int draw_text_menu(int x, int y, int dx, int dy, void* data, int mac, entry_print print, entry_separator separator, int* abase, int* apos, int* akey);

#endif