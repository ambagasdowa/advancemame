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

#include "draw.h"
#include "video.h"
#include "update.h"
#include "blit.h"
#include "font.h"
#include "conf.h"
#include "os.h"
#include "videoall.h"

#include <string.h>

#include "option.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

/***************************************************************************/
/* crtc */

static int video_crtc_select_by_addr(const video_crtc* a, void* b) {
	return a==b;
}

static int video_crtc_select_by_compare(const video_crtc* a, void* _b) {
	const video_crtc* b = (const video_crtc*)_b;
	return video_crtc_compare(a,b)==0;
}

/***************************************************************************/
/* Common variable */

enum advance_t {
	advance_mame, advance_mess, advance_pac, advance_menu, advance_vbe, advance_vga
} the_advance; /* The current operating mode */

struct conf_context* the_config;

#ifdef __MSDOS__
int the_advance_vbe_active; /* if AdvanceVBE is active */
int the_advance_vga_active; /* if AdvanceVGA is active */
#endif

int the_mode_bit = 8;
int the_mode_type = VIDEO_FLAGS_TYPE_GRAPHICS | VIDEO_FLAGS_INDEX_RGB;

/***************************************************************************/
/* Common information screens */

/*
static int draw_text_warn(void) {
	int x = 0;
	int y = 0;
	int dx = text_size_x();
	int dy = text_size_y();
	static int userkey = 0;

	if (userkey != OS_INPUT_SPACE) {

	text_clear();

	draw_text_center(x,y,dx,
"WARNING! Playing with video parameter can DAMAGE YOUR MONITOR!"
	,COLOR_REVERSE);
	++y;

	draw_text_center(x,y,dx,
"IF YOU HEAR STRANGE SOUNDS PRESS ESC IMMEDITIALLY!"
	,COLOR_REVERSE);
	++y;

	y = draw_text_para(x,y,dx,dy-y,
"You can modify two series of parameters, horizontal and vertical. These "
"parameters control the course of the video beam."
"\n\n"
"The display region is the part used for drawing. Start always from 0. "
"The retrace region is the part used by the beam to return backward. "
"The blank region is the part in which the beam is turned off. "
"The total end is the total number of columns and lines."
"\n\n"
"The starting and ending values of these regions are shown in the correct order, from the "
"smallest to the greatest, you must mantain this condition."
"\n\n"
"Others important values are the horizontal and vertical clock frequencies, these "
"values MUST BE COMPATIBLE with your monitor. You MUST check your monitor's manual "
"for getting correct values. "
"Generic old SVGA monitors accept fixed HClock of 31.5 and 35.5 kHz, new multisync monitors "
"accept values in the range 30-70 kHz. The generic VClock's acceptable range is 50-120 Hz."
"\n\n"
"For centering the screen modify the retrace start or use the ARROWS. "
"For changing the resolution modify the display end. "
"For resizing the screen modify the total end and the polarization or use the CTRL+ARROWS. "
"\n\n"
"Press SPACE to continue or ESC to exit."
	,COLOR_NORMAL);

	do {
		userkey = input_getkey();
	} while (userkey!=OS_INPUT_SPACE && userkey!=OS_INPUT_ESC);

	}

	return userkey != OS_INPUT_SPACE;
}
*/

/*
static int draw_text_warn_bios(void) {
	int x = 0;
	int y = 0;
	int dx = text_size_x();
	int dy = text_size_y();
	int userkey = 0;

	if (userkey != OS_INPUT_Y) {

	text_clear();

	draw_text_center(x,y,dx,
"WARNING! Using wrong video mode can DAMAGE YOUR MONITOR!"
	,COLOR_REVERSE);
	++y;

	draw_text_center(x,y,dx,
"IF YOU HEAR STRANGE SOUNDS PRESS ESC IMMEDITIALLY!"
	,COLOR_REVERSE);
	++y;

	++y;
	y = draw_text_para(x,y,dx,dy-y,
"You are going to test a BIOS VGA or VBE mode with a low frequency monitor.\n"
"Usually these modes work only with normal high frequency monitors. "
"So proceed only if you know that you are doing."
"\n\n"
"Press Y to continue or ESC to exit."
	,COLOR_NORMAL);

	do {
		userkey = input_getkey();
	} while (userkey!=OS_INPUT_Y &&  userkey!=OS_INPUT_ESC);

	}

	return userkey != OS_INPUT_Y;
}
*/

static int draw_text_help(void) {
	int x = 0;
	int y = 0;
	int dx = text_size_x();
	int dy = text_size_y();
	int userkey = 0;

	text_clear();

	y = draw_text_para(x,y,dx,dy,
" HELP"
	,COLOR_REVERSE);

	y = draw_text_para(x,y,dx,dy-y,
"F2    Save the selected modes\n"
"F5    Create a new modeline (favourite modes with the specified size)\n"
"F6    Create a new modeline (favourite modes with the specified clock)\n"
"F7    Duplicate the current mode\n"
"F8    Set an arbitrary clock value\n"
"F9    Show a static test screen for the current video mode\n"
"F10   Show a dynamic test screen for the current video mode\n"
"SPACE Select/Unselect the current video mode\n"
"ENTER Test the current video mode\n"
"TAB   Rename the current video mode\n"
"ESC   Exit\n"
"\n"
"q/a   Increase the x/y Display End register (SHIFT for decreasing)\n"
"w/s   Increase the x/y Blank Start register (SHIFT for decreasing)\n"
"e/d   Increase the x/y Retrace Start register (SHIFT for decreasing)\n"
"r/f   Increase the x/y Retrace End register (SHIFT for decreasing)\n"
"t/g   Increase the x/y Blank End register (SHIFT for decreasing)\n"
"y/h   Increase the x/y Total End register (SHIFT for decreasing)\n"
"v     Increase the pixel clock (SHIFT for decreasing)\n"
"u/j   Change the polarization\n"
"x/c   Change the scan line mode\n"
"n/m   Change the tv mode\n"
"\n"
"Press ESC"
	,COLOR_NORMAL);

	video_wait_vsync();

	do {
		userkey = os_input_get();
	} while (userkey!=OS_INPUT_ESC);

	return userkey;
}

static int draw_text_error(void) {
	int x = 0;
	int y = 0;
	int dx = text_size_x();
	int dy = text_size_y();
	static int userkey = 0;

	text_clear();

	draw_text_center(x,y,dx,
"ERROR! An error occoured in your last action!"
	,COLOR_REVERSE);
	++y;

	++y;
	y = draw_text_para(x,y,dx,dy,
"Your last action generated an error. Probably you have requested an "
"unsupported feature by your hardware or software."
	,COLOR_NORMAL);

	if (*video_error_description_get()) {
		y = draw_text_para(x,y,dx,dy-y,"\nThe video software report this error:",COLOR_NORMAL);
		os_log(("v: error \"%s\"\n", video_error_description_get() ));
		y = draw_text_para(x,y,dx,dy-y,video_error_description_get(),COLOR_ERROR);
	}

	y = draw_text_para(x,y,dx,dy-y,"\nPress ESC",COLOR_NORMAL);

	video_wait_vsync();

	do {
		userkey = os_input_get();
	} while (userkey!=OS_INPUT_ESC);

	return userkey;
}

/***************************************************************************/
/* Menu */

int menu_base;
int menu_rel;
int menu_rel_max;
int menu_base_max;
int menu_max;

static video_crtc* menu_pos(int pos) {
	if (pos < 0 || pos >= menu_max)
		return 0;

	return video_crtc_container_pos(&the_modes,pos);
}

static video_crtc* menu_current(void) {
	return menu_pos(menu_base + menu_rel);
}

static void menu_modify(void) {
	the_modes_modified = 1;
}

static void menu_insert(video_crtc* crtc) {
	crtc->user_flags |= VIDEO_FLAGS_USER_BIT0;

	the_modes_modified = 1;

	video_crtc_container_insert_sort(&the_modes,crtc,video_crtc_compare);

	menu_max = video_crtc_container_max(&the_modes);
}

static void menu_remove(video_crtc* crtc) {

	the_modes_modified = 1;

	video_crtc_container_remove(&the_modes,video_crtc_select_by_addr,crtc);

	menu_max = video_crtc_container_max(&the_modes);
}

static void menu_item_draw(int x, int y, int dx, int pos, int selected) {
	char buffer[256];

	video_crtc* crtc = menu_pos(pos);

	if (crtc) {
		char tag;
		unsigned color;
		char vfreq[8];
		char hfreq[8];

		if (selected) {
			if (crtc->user_flags & VIDEO_FLAGS_USER_BIT0) {
				tag = '�';
				color = COLOR_SELECTED_MARK;
			} else {
				tag = ' ';
				color = COLOR_SELECTED;
			}
		} else {
			if (crtc->user_flags & VIDEO_FLAGS_USER_BIT0) {
				tag = '�';
				if (crtc_clock_check(&the_monitor,crtc)) {
					color = COLOR_MARK;
				} else {
					color = COLOR_MARK_BAD;
				}
			} else {
				tag = ' ';
				if (crtc_clock_check(&the_monitor,crtc)) {
					color = COLOR_NORMAL;
				} else {
					color = COLOR_BAD;
				}
			}
		}

		sprintf(vfreq,"%6.2f",(double)crtc_vclock_get(crtc));

		sprintf(hfreq,"%6.2f",(double)crtc_hclock_get(crtc) / 1E3);

		sprintf(buffer," %c %4d %4d %s %s %s",
			tag,
			crtc->hde,
			crtc->vde,
			hfreq,
			vfreq,
			crtc->name
		);

		draw_text_left(x,y,dx,buffer,color);

	} else {
		draw_text_fill(x,y,' ',dx,COLOR_NORMAL);
	}
}

static void menu_draw(int x, int y, int dx, int dy) {
	unsigned i;
	for(i=0;i<dy;++i) {
		if (menu_base + i < menu_max) {
			menu_item_draw(x,y+i,dx, menu_base + i, i == menu_rel);
		} else
			draw_text_fill(x,y+i,' ',dx,COLOR_NORMAL);
	}
}

/***************************************************************************/
/* Draw information bars */

static void format_info(char* buffer0, char* buffer1, char* buffer2, video_crtc* crtc) {
	double HD,HF,HS,HB;
	double VD,VF,VS,VB;
	double f;

	HD = crtc->hde;
	HF = crtc->hrs - crtc->hde;
	HS = crtc->hre - crtc->hrs;
	HB = crtc->ht - crtc->hre;
	f = 1 / (HD+HF+HS+HB);
	HD *= f;
	HF *= f;
	HS *= f;
	HB *= f;

	VD = crtc->vde;
	VF = crtc->vrs - crtc->vde;
	VS = crtc->vre - crtc->vrs;
	VB = crtc->vt - crtc->vre;
	f = 1 / (VD+VF+VS+VB);
	VD *= f;
	VF *= f;
	VS *= f;
	VB *= f;

	sprintf(buffer0,"plz clock  dsen rtst rten totl  disp  front sync  back  pclock");
	sprintf(buffer1,"h%c %7.3f%5d%5d%5d%5d %6.3f%6.3f%6.3f%6.3f %8.4f", crtc_is_nhsync(crtc) ? '-' : '+', crtc_hclock_get(crtc) / 1E3, crtc->hde,crtc->hrs,crtc->hre,crtc->ht,HD,HF,HS,HB, crtc_pclock_get(crtc) / 1E6);
	sprintf(buffer2,"v%c %7.3f%5d%5d%5d%5d %6.3f%6.3f%6.3f%6.3f%s%s%s%s", crtc_is_nvsync(crtc) ? '-' : '+', crtc_vclock_get(crtc), crtc->vde,crtc->vrs,crtc->vre,crtc->vt,VD,VF,VS,VB, crtc_is_doublescan(crtc) ? " doublescan" : "", crtc_is_interlace(crtc) ? " interlace" : "", crtc_is_tvpal(crtc) ? " tvpal" : "", crtc_is_tvntsc(crtc) ? " tvntsc" : "");
}

static void draw_text_info(int x, int y, int dx, int dy, int pos) {
	char buffer[3][256];

	video_crtc* crtc = menu_pos(pos);
	format_info(buffer[0],buffer[1],buffer[2],crtc);

	draw_text_left(x,y+0,dx,buffer[0],COLOR_INFO_TITLE);
	draw_text_left(x,y+1,dx,buffer[1],COLOR_INFO_NORMAL);
	draw_text_left(x,y+2,dx,buffer[2],COLOR_INFO_NORMAL);

	if (crtc) {
		if (!crtc_clock_check(&the_monitor,crtc)) {
			draw_text_left(x+dx-14,y,14," OUT OF RANGE ", COLOR_ERROR);
		}
	} else {
		draw_text_fillrect(x,y,' ',dx,dy,COLOR_INFO_NORMAL);
	}
}

static void draw_text_bit(int x, int y, int dx) {
	int i;
	int pos = x;

	pos += draw_text_string(pos,y,"Type ",COLOR_TITLE);

	for(i=0;i<6;++i) {
		const char* text;
		int bit;
		unsigned color;
		switch (i) {
			default:
			case 0 : text = "text"; bit = 0; break;
			case 1 : text = "8 bit"; bit = 8; break;
			case 2 : text = "15 bit"; bit = 15; break;
			case 3 : text = "16 bit"; bit = 16; break;
			case 4 : text = "24 bit"; bit = 24; break;
			case 5 : text = "32 bit"; bit = 32; break;
		}
		if (the_mode_bit == bit)
			color = COLOR_SELECTED;
		else
			color = COLOR_NORMAL;
		pos += draw_text_string(pos,y," ",color);
		pos += draw_text_string(pos,y,text,color);
		pos += draw_text_string(pos,y," ",color);
	}

	draw_text_fillrect(pos,y,' ',dx - (pos - x),1,COLOR_NORMAL);
}

static void draw_text_bar(int x, int by1, int by2, int dx) {
	char buffer[256];
	unsigned i;

	switch (the_advance) {
		case advance_vbe :
			sprintf(buffer," AdvanceVBE Video Config - " __DATE__ );
			break;
		case advance_vga :
			sprintf(buffer," AdvanceVGA Video Config - " __DATE__ );
			break;
		case advance_mess :
			sprintf(buffer," AdvanceMESS Video Config - " __DATE__ );
			break;
		case advance_mame :
			sprintf(buffer," AdvanceMAME Video Config - " __DATE__ );
			break;
		case advance_pac :
			sprintf(buffer," AdvancePAC Video Config - " __DATE__ );
			break;
		case advance_menu :
			sprintf(buffer," AdvanceMENU Video Config - " __DATE__ );
			break;
	}

	draw_text_left(x,by1,dx,buffer,COLOR_BAR);

	strcpy(buffer,"");
	for(i=0;i<video_driver_vector_max();++i) {
		if (video_driver_vector_pos(i) != 0) {
			if (*buffer)
				strcat(buffer,"/");
			strcat(buffer, video_driver_vector_pos(i)->name);
		}
	}

	draw_text_left(x + dx - strlen(buffer),by1,strlen(buffer),buffer,COLOR_BAR);

	sprintf(buffer, " #    x    y hclock vclock name");
	draw_text_left(x,by1+2,dx,buffer,COLOR_TITLE);

	sprintf(buffer," F1 Help  F2 Save  SPACE Select  TAB Rename  ENTER Test  ESC Exit");
	draw_text_left(x,by2,dx,buffer,COLOR_BAR);
}

/***************************************************************************/
/* Test screen */

static int test_default_command(int x, int y) {
	draw_string(x,y,"ENTER  Save & Exit",DRAW_COLOR_WHITE);
	++y;
	draw_string(x,y,"ESC    Exit",DRAW_COLOR_WHITE);
	++y;

	return y;
}

static int test_crtc(int x, int y, video_crtc* crtc, int print_clock, int print_measured_clock, int print_key) {
	char buffer[256];

	sprintf(buffer,"Horz  Vert");
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	if (print_clock) {
		sprintf(buffer,"%4.1f %5.1f %sClock Requested [kHz Hz]", crtc_hclock_get(crtc) / 1E3, crtc_vclock_get(crtc), print_key ? "     " : "");
		draw_string(x,y,buffer,DRAW_COLOR_WHITE);
		++y;
	}

	if (print_measured_clock) {
		sprintf(buffer,"     %5.1f %sClock Measured [Hz]", video_measured_vclock(), print_key ? "     " : "");
		draw_string(x,y,buffer,DRAW_COLOR_WHITE);
		++y;
	}

	sprintf(buffer,"%4d  %4d %sDisplay End",crtc->hde,crtc->vde, print_key ? "[qa] " : "");
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	sprintf(buffer,"%4d  %4d %sRetrace Start",crtc->hrs,crtc->vrs, print_key ? "[ed] " : "");
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;
	if (!(crtc->hde<=crtc->hrs)) {
		sprintf(buffer,"HDE<=HRS");
		draw_string(x,y,buffer,DRAW_COLOR_RED);
		++y;
	}
	if (!(crtc->vde<=crtc->vrs)) {
		sprintf(buffer,"VDE<=VRS");
		draw_string(x,y,buffer,DRAW_COLOR_RED);
		++y;
	}

	sprintf(buffer,"%4d  %4d %sRetrace End",crtc->hre,crtc->vre, print_key ? "[rf] " : "");
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;
	if (!(crtc->hrs<crtc->hre)) {
		sprintf(buffer,"HRS<HRE");
		draw_string(x,y,buffer,DRAW_COLOR_RED);
		++y;
	}
	if (!(crtc->vrs<crtc->vre)) {
		sprintf(buffer,"VRE<VRE");
		draw_string(x,y,buffer,DRAW_COLOR_RED);
		++y;
	}

	sprintf(buffer,"%4d  %4d %sTotal",crtc->ht,crtc->vt, print_key ? "[yh] " : "");
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;
	if (!(crtc->hre<=crtc->ht)) {
		sprintf(buffer,"HRE<=HT");
		draw_string(x,y,buffer,DRAW_COLOR_RED);
		++y;
	}
	if (!(crtc->vre<=crtc->vt)) {
		sprintf(buffer,"VRE<=VT");
		draw_string(x,y,buffer,DRAW_COLOR_RED);
		++y;
	}

	sprintf(buffer,"   %c     %c %sPolarization", crtc_is_nhsync(crtc) ? '-' : '+', crtc_is_nvsync(crtc) ? '-' : '+', print_key ? "[uj] " : "");
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	sprintf(buffer,"      %4s %sDoublescan", crtc_is_doublescan(crtc) ? "on" : "off", print_key ? "[x]  " : "");
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	sprintf(buffer,"      %4s %sInterlaced", crtc_is_interlace(crtc) ? "on" : "off", print_key ? "[c]  " : "");
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	sprintf(buffer,"      %4s %sTV PAL", crtc_is_tvpal(crtc) ? "on" : "off", print_key ? "[n]  " : "");
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	sprintf(buffer,"      %4s %sTV NTSC", crtc_is_tvntsc(crtc) ? "on" : "off", print_key ? "[m]  " : "");
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	if (print_clock) {
		sprintf(buffer,"%4.2f      %sPixelclock [MHz]", (double)crtc->pixelclock / 1E6, print_key ? "[v]  " : "");
		draw_string(x,y,buffer,DRAW_COLOR_WHITE);
		++y;
	}

	if (print_key) {
		++y;
		draw_string(x,y,"Q...U  Inc horz (SHIFT dec)",DRAW_COLOR_WHITE);
		++y;
		draw_string(x,y,"A...J  Inc vert (SHIFT dec)",DRAW_COLOR_WHITE);
		++y;
		draw_string(x,y,"XCV    Flip flag",DRAW_COLOR_WHITE);
		++y;
		draw_string(x,y,"ARROWS Center",DRAW_COLOR_WHITE);
		++y;
		draw_string(x,y,"CTRL+ARROWS Expand/Shrink",DRAW_COLOR_WHITE);
		++y;
	}

	return y;
}

#ifdef USE_VIDEO_VGALINE
static int test_vgaline(int x, int y, vgaline_video_mode* mode) {
	char buffer[256];
	draw_test_default();

	if (video_is_text()) {
		sprintf(buffer,"vgaline %dx%d %dx%d", video_size_x(), video_size_y(), video_size_x() / video_font_size_x(), video_size_y() / video_font_size_y());
		draw_string(x,y,buffer,DRAW_COLOR_WHITE);
		++y;
	} else {
		sprintf(buffer,"vgaline %dx%dx%d [%dx%d]", video_size_x(), video_size_y(), video_bits_per_pixel(), video_virtual_x(), video_virtual_y());
		draw_string(x,y,buffer,DRAW_COLOR_WHITE);
		++y;
	}

	++y;

	y = test_crtc(x,y,&mode->crtc,1,1,1);
	y = test_default_command(x,y);

	return y;
}
#endif

#ifdef USE_VIDEO_VBELINE
static int test_vbeline(int x, int y, vbeline_video_mode* mode) {
	char buffer[256];
	vbe_ModeInfoBlock info;

	draw_test_default();

	sprintf(buffer,"vbeline %dx%dx%d [%dx%d]", video_size_x(), video_size_y(), video_bits_per_pixel(), video_virtual_x(), video_virtual_y());
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	vbe_mode_info_get(&info, mode->mode);

	sprintf(buffer,"based on vbe mode 0x%x %dx%dx%d", mode->mode, info.XResolution, info.YResolution, info.BitsPerPixel);
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	++y;

	y = test_crtc(x,y,&mode->crtc,1,1,1);
	y = test_default_command(x,y);
	return y;
}
#endif

#ifdef USE_VIDEO_SVGALINE
static int test_svgaline(int x, int y, svgaline_video_mode* mode) {
	char buffer[256];

	draw_test_default();

	sprintf(buffer,"svgaline %dx%dx%d [%dx%d]", video_size_x(), video_size_y(), video_bits_per_pixel(), video_virtual_x(), video_virtual_y());
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	++y;

	y = test_crtc(x,y,&mode->crtc,1,1,1);
	y = test_default_command(x,y);
	return y;
}
#endif

#ifdef USE_VIDEO_SVGALIB
static int test_svgalib(int x, int y, svgalib_video_mode* mode) {
	char buffer[256];

	draw_test_default();

	sprintf(buffer,"svgalib %dx%dx%d [%dx%d]", video_size_x(), video_size_y(), video_bits_per_pixel(), video_virtual_x(), video_virtual_y());
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	++y;

	y = test_crtc(x,y,&mode->crtc,1,1,1);
	y = test_default_command(x,y);
	return y;
}
#endif

#ifdef USE_VIDEO_FB
static int test_fb(int x, int y, fb_video_mode* mode) {
	char buffer[256];

	draw_test_default();

	sprintf(buffer,"fb %dx%dx%d [%dx%d]", video_size_x(), video_size_y(), video_bits_per_pixel(), video_virtual_x(), video_virtual_y());
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	++y;

	y = test_crtc(x,y,&mode->crtc,1,1,1);
	y = test_default_command(x,y);
	return y;
}
#endif

#ifdef USE_VIDEO_VGA
static int test_vga(int x, int y, vga_video_mode* mode) {
	char buffer[256];
	struct tweak_crtc info;
	video_crtc crtc;

	draw_test_default();

	sprintf(buffer,"BIOS vga 0x%x %dx%dx%d [%dx%d]", mode->mode, video_size_x(), video_size_y(), video_bits_per_pixel(), video_virtual_x(), video_virtual_y());
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	++y;

	tweak_crtc_get(&info,tweak_reg_read,0);
	if (video_crtc_import(&crtc,&info,video_size_x(),video_size_y(),video_measured_vclock())==0) {
		++y;
		y = test_crtc(x,y,&crtc,0,1,0);
	}

	++y;
	y = test_default_command(x,y);

	return y;
}
#endif

#ifdef USE_VIDEO_VBE
static int test_vbe(int x, int y, vbe_video_mode* mode) {
	char buffer[256];
	struct vga_info info;
	struct vga_regs regs;
	video_crtc crtc;

	draw_test_default();

	sprintf(buffer,"BIOS vbe 0x%x %dx%dx%d [%dx%d]", mode->mode, video_size_x(), video_size_y(), video_bits_per_pixel(), video_virtual_x(), video_virtual_y());
	draw_string(x,y,buffer,DRAW_COLOR_WHITE);
	++y;

	++y;

	vga_mode_get(&regs);
	vga_regs_info_get(&regs,&info);
	if (video_crtc_import(&crtc,&info,video_size_x(),video_size_y(),video_measured_vclock())==0) {
		++y;
		y = test_crtc(x,y,&crtc,0,1,0);
	}

	++y;
	y = test_default_command(x,y);

	return y;
}
#endif

static int test_draw(int x, int y, video_mode* mode) {
	if (0) ;
#ifdef USE_VIDEO_VGA
	else if (video_current_driver() == &video_vga_driver)
		y = test_vga(x,y,(vga_video_mode*)mode->driver_mode);
#endif
#ifdef USE_VIDEO_VGALINE
	else if (video_current_driver() == &video_vgaline_driver)
		y = test_vgaline(x,y,(vgaline_video_mode*)mode->driver_mode);
#endif
#ifdef USE_VIDEO_SVGALINE
	else if (video_current_driver() == &video_svgaline_driver)
		y = test_svgaline(x,y,(svgaline_video_mode*)mode->driver_mode);
#endif
#ifdef USE_VIDEO_VBE
	else if (video_current_driver() == &video_vbe_driver)
		y = test_vbe(x,y,(vbe_video_mode*)mode->driver_mode);
#endif
#ifdef USE_VIDEO_VBELINE
	else if (video_current_driver() == &video_vbeline_driver)
		y = test_vbeline(x,y,(vbeline_video_mode*)mode->driver_mode);
#endif
#ifdef USE_VIDEO_SVGALIB
	else if (video_current_driver() == &video_svgalib_driver)
		y = test_svgalib(x,y,(svgalib_video_mode*)mode->driver_mode);
#endif
#ifdef USE_VIDEO_FB
	else if (video_current_driver() == &video_fb_driver)
		y = test_fb(x,y,(fb_video_mode*)mode->driver_mode);
#endif
	return y;
}

static int test_exe_crtc(int userkey, video_crtc* crtc) {
	int modify = 0;
	unsigned pred_t;
	int xdelta;

	if (crtc->hde % 9 == 0 && crtc->hrs % 9 == 0 && crtc->hre % 9 == 0 && crtc->ht % 9 == 0)
		xdelta = 9;
	else
		xdelta = 8;

	switch (userkey) {
		case OS_INPUT_CTRLLEFT :
			pred_t = crtc->ht;
			crtc->ht -= crtc->ht % xdelta;
			crtc->ht -= xdelta;
			crtc->pixelclock = (double)crtc->pixelclock * crtc->ht / pred_t;
			modify = 1;
			break;
		case OS_INPUT_CTRLRIGHT :
			pred_t = crtc->ht;
			crtc->ht -= crtc->ht % xdelta;
			crtc->ht += xdelta;
			crtc->pixelclock = (double)crtc->pixelclock * crtc->ht / pred_t;
			modify = 1;
			break;
		case OS_INPUT_CTRLUP :
			crtc->vt -= 1;
			modify = 1;
			break;
		case OS_INPUT_CTRLDOWN :
			crtc->vt += 1;
			modify = 1;
			break;
		case OS_INPUT_LEFT :
			crtc->hrs -= crtc->hrs % xdelta;
			crtc->hrs += xdelta;
			crtc->hre -= crtc->hre % xdelta;
			crtc->hre += xdelta;
			modify = 1;
			break;
		case OS_INPUT_RIGHT :
			crtc->hrs -= crtc->hrs % xdelta;
			crtc->hrs -= xdelta;
			crtc->hre -= crtc->hre % xdelta;
			crtc->hre -= xdelta;
			modify = 1;
			break;
		case OS_INPUT_DOWN :
			crtc->vrs -= 1;
			crtc->vre -= 1;
			modify = 1;
			break;
		case OS_INPUT_UP :
			crtc->vrs += 1;
			crtc->vre += 1;
			modify = 1;
			break;
		case 'u' :
		case 'U' :
			modify = 1;
			if (crtc_is_nhsync(crtc))
				crtc_phsync_set(crtc);
			else
				crtc_nhsync_set(crtc);
			break;
		case 'j' :
		case 'J' :
			modify = 1;
			if (crtc_is_nvsync(crtc))
				crtc_pvsync_set(crtc);
			else
				crtc_nvsync_set(crtc);
			break;
		case 'e' :
			crtc->hrs -= crtc->hrs % xdelta;
			crtc->hrs += xdelta;
			modify = 1;
			break;
		case 'E' :
			crtc->hrs -= crtc->hrs % xdelta;
			crtc->hrs -= xdelta;
			modify = 1;
			break;
		case 'r' :
			crtc->hre -= crtc->hre % xdelta;
			crtc->hre += xdelta;
			modify = 1;
			break;
		case 'R' :
			crtc->hre -= crtc->hre % xdelta;
			crtc->hre -= xdelta;
			modify = 1;
			break;
		case 'y' :
			crtc->ht -= crtc->ht % xdelta;
			crtc->ht += xdelta;
			modify = 1;
			break;
		case 'Y' :
			crtc->ht -= crtc->ht % xdelta;
			crtc->ht -= xdelta;
			modify = 1;
			break;
		case 'q' :
			crtc->hde -= crtc->hde % xdelta;
			crtc->hde += xdelta;
			modify = 1;
			break;
		case 'Q' :
			crtc->hde -= crtc->hde % xdelta;
			crtc->hde -= xdelta;
			modify = 1;
			break;
		case 'a' :
			++crtc->vde;
			modify = 1;
			break;
		case 'A' :
			--crtc->vde;
			modify = 1;
			break;
		case 'd' :
			++crtc->vrs;
			modify = 1;
			break;
		case 'D' :
			--crtc->vrs;
			modify = 1;
			break;
		case 'f' :
			++crtc->vre;
			modify = 1;
			break;
		case 'F' :
			--crtc->vre;
			modify = 1;
			break;
		case 'h' :
			++crtc->vt;
			modify = 1;
			break;
		case 'H' :
			--crtc->vt;
			modify = 1;
			break;
		case 'x' :
		case 'X' :
			modify = 1;
			if (crtc_is_doublescan(crtc))
				crtc_singlescan_set(crtc);
			else
				crtc_doublescan_set(crtc);
			break;
		case 'c' :
		case 'C' :
			modify = 1;
			if (crtc_is_interlace(crtc))
				crtc_singlescan_set(crtc);
			else
				crtc_interlace_set(crtc);
			break;
		case 'v' :
			modify = 1;
			crtc->pixelclock += 100000;
			break;
		case 'V' :
			modify = 1;
			crtc->pixelclock -= 100000;
			break;
		case 'n' :
		case 'N' :
			modify = 1;
			if (crtc_is_tvpal(crtc))
				crtc_notv_set(crtc);
			else
				crtc_tvpal_set(crtc);
			break;
		case 'm' :
		case 'M' :
			modify = 1;
			if (crtc_is_tvntsc(crtc))
				crtc_notv_set(crtc);
			else
				crtc_tvntsc_set(crtc);
			break;
	}
	return modify;
}

/***************************************************************************/
/* Menu command */

static void cmd_type(int key) {
	if (key == OS_INPUT_RIGHT) {
		switch (the_mode_bit) {
			case 0 : the_mode_bit = 8; the_mode_type = VIDEO_FLAGS_TYPE_GRAPHICS | VIDEO_FLAGS_INDEX_RGB; break;
			case 8 : the_mode_bit = 15; the_mode_type = VIDEO_FLAGS_TYPE_GRAPHICS | VIDEO_FLAGS_INDEX_RGB; break;
			case 15 : the_mode_bit = 16; the_mode_type = VIDEO_FLAGS_TYPE_GRAPHICS | VIDEO_FLAGS_INDEX_RGB; break;
			case 16 : the_mode_bit = 24; the_mode_type = VIDEO_FLAGS_TYPE_GRAPHICS | VIDEO_FLAGS_INDEX_RGB; break;
			case 24 : the_mode_bit = 32; the_mode_type = VIDEO_FLAGS_TYPE_GRAPHICS | VIDEO_FLAGS_INDEX_RGB; break;
			case 32 : the_mode_bit = 0; the_mode_type = VIDEO_FLAGS_TYPE_TEXT | VIDEO_FLAGS_INDEX_TEXT; break;
			default: the_mode_bit = 0; the_mode_type = VIDEO_FLAGS_TYPE_TEXT | VIDEO_FLAGS_INDEX_TEXT; break;
		}
	} else {
		switch (the_mode_bit) {
			case 15 : the_mode_bit = 8; the_mode_type = VIDEO_FLAGS_TYPE_GRAPHICS | VIDEO_FLAGS_INDEX_RGB; break;
			case 16 : the_mode_bit = 15; the_mode_type = VIDEO_FLAGS_TYPE_GRAPHICS | VIDEO_FLAGS_INDEX_RGB; break;
			case 24 : the_mode_bit = 16; the_mode_type = VIDEO_FLAGS_TYPE_GRAPHICS | VIDEO_FLAGS_INDEX_RGB; break;
			case 32 : the_mode_bit = 24; the_mode_type = VIDEO_FLAGS_TYPE_GRAPHICS | VIDEO_FLAGS_INDEX_RGB; break;
			case 0 : the_mode_bit = 32; the_mode_type = VIDEO_FLAGS_TYPE_GRAPHICS | VIDEO_FLAGS_INDEX_RGB; break;
			case 8 : the_mode_bit = 0; the_mode_type = VIDEO_FLAGS_TYPE_TEXT | VIDEO_FLAGS_INDEX_TEXT; break;
			default: the_mode_bit = 0; the_mode_type = VIDEO_FLAGS_TYPE_TEXT | VIDEO_FLAGS_INDEX_TEXT; break;
		}
	}
}

static void cmd_select(void) {
	video_crtc* crtc;

	crtc = menu_current();
	if (!crtc)
		return;

	crtc->user_flags = crtc->user_flags ^ VIDEO_FLAGS_USER_BIT0;

	menu_modify();
}

static int cmd_offvideo_test(int userkey) {
	video_crtc* crtc;
	video_crtc crtc_save;
	int modify = 0;

	crtc = menu_current();
	if (!crtc)
		return -1;

	crtc_save = *crtc;

	modify = test_exe_crtc(userkey,crtc);

	if (!modify) {
		sound_warn();
		return 0;
	}

	menu_modify();

	if (strcmp(crtc->name,DEFAULT_TEXT_MODE)==0) {
		if (!crtc_clock_check(&the_monitor,crtc) && crtc_clock_check(&the_monitor,&crtc_save)) {
			*crtc = crtc_save;
			sound_error();
			return 0;
		}
		text_reset();
	}

	return 0;
}

static int cmd_onvideo_test(void) {
	video_crtc* crtc;
	video_mode mode;
	int done ;
	video_crtc crtc_save;
	int dirty = 1;
	int crtc_save_modified;

	crtc = menu_current();
	if (!crtc)
		return -1;

	if (!crtc_clock_check(&the_monitor,crtc))
		return -1;

	if (video_mode_generate(&mode,crtc,the_mode_bit,the_mode_type)!=0) {
		return -1;
	}

	video_mode_done(1);
	if (video_mode_set(&mode)!=0) {
		return -1;
	}

	crtc_save = *crtc;
	crtc_save_modified = the_modes_modified;

	done = 0;
	while (!done) {
		int userkey;
		int modify = 0;

		video_crtc last_crtc = *crtc;
		video_mode last_mode = mode;
		int vm_last_modified = the_modes_modified;

		if (dirty) {
			test_draw(1,1,&mode);
			dirty = 0;
		}

		video_wait_vsync();

		userkey = os_input_get();

		switch (userkey) {
			case OS_INPUT_ESC :
				done = 1;
				/* restore */
				*crtc = crtc_save;
				the_modes_modified = crtc_save_modified;
				break;
			case OS_INPUT_ENTER :
				done = 1;
				break;
		}

		if (!done) {
			modify = test_exe_crtc(userkey,crtc);

			if (modify) {
				the_modes_modified = 1;
				dirty = 1;

				if (crtc_clock_check(&the_monitor,crtc)
					&& video_mode_generate(&mode,crtc,the_mode_bit,the_mode_type)==0) {
					if (video_mode_set(&mode) != 0) {
						/* abort */
						*crtc = crtc_save;
						the_modes_modified = crtc_save_modified;
						return -1;
					}
				} else {
					/* restore */
					mode = last_mode;
					*crtc = last_crtc;
					the_modes_modified = vm_last_modified;

					dirty = 1;
					sound_error();
				}
			} else {
				sound_warn();
			}
		}
	}

	return 0;
}

static int cmd_onvideo_calib(void) {
	video_mode mode;
	video_crtc* crtc;
	unsigned speed;
	char buffer[128];

	if ((the_mode_type & VIDEO_FLAGS_TYPE_MASK) != VIDEO_FLAGS_TYPE_GRAPHICS) {
		video_error_description_set("Command supported only in graphics mode");
		return -1;
	}

	crtc = menu_current();
	if (!crtc)
		return -1;

	if (!crtc_clock_check(&the_monitor,crtc))
		return -1;

	if (video_mode_generate(&mode,crtc,the_mode_bit,the_mode_type)!=0) {
		return -1;
	}

	if (video_mode_set(&mode)!=0) {
		return -1;
	}

	draw_graphics_palette();
	/* draw_graphics_out_of_screen(0); */
	draw_graphics_clear();

	speed = draw_graphics_speed(0,0,video_size_x(),video_size_y());
	draw_graphics_calib(0,0,video_size_x(),video_size_y());

	sprintf(buffer," %.2f MB/s", speed / (double)(1024*1024));
	draw_string(0,0,buffer,DRAW_COLOR_WHITE);

	video_wait_vsync();

	os_input_get();

	return 0;
}

static int cmd_onvideo_animate(void) {
	video_mode mode;
	video_crtc* crtc;
	unsigned i;
	int counter;

	if ((the_mode_type & VIDEO_FLAGS_TYPE_MASK) != VIDEO_FLAGS_TYPE_GRAPHICS) {
		video_error_description_set("Command supported only in graphics mode");
		return -1;
	}

	crtc = menu_current();
	if (!crtc)
		return -1;

	if (!crtc_clock_check(&the_monitor,crtc))
		return -1;

	if (video_mode_generate(&mode,crtc,the_mode_bit,the_mode_type)!=0) {
		return -1;
	}

	if (video_mode_set(&mode)!=0) {
		return -1;
	}

	update_init(2);

	draw_graphics_palette();

	for(i=0;i<3;++i) {
		update_start();
		video_clear(update_x_get(),update_y_get(),video_size_x(),video_size_y(),0);
		update_stop(1);
	}

	counter = update_page_max_get();
	while (!os_input_hit()) {
		update_start();
		draw_graphics_animate(update_x_get(),update_y_get(),video_size_x(),video_size_y(), counter - update_page_max_get() + 1, 1);
		++counter;
		draw_graphics_animate(update_x_get(),update_y_get(),video_size_x(),video_size_y(), counter, 0);
		update_stop(1);
	}

	video_wait_vsync();

	os_input_get();

	update_done();

	return 0;
}

static void cmd_gotopos(int i) {
	if (i >= menu_max)
		i = menu_max - 1;
	if (i<0)
		i = 0;
	if (menu_base <= i && i < menu_base + menu_rel_max) {
		menu_rel = i - menu_base;
	} else if (i<menu_base) {
		menu_base = i;
		menu_rel = 0;
	} else {
		menu_base = i - menu_rel_max + 1;
		if (menu_base<0)
			menu_base = 0;
		menu_rel = i - menu_base;
	}
}

static int cmd_input_key(const char* tag, const char* keys) {
	draw_text_fill(0,text_size_y()-1,' ',text_size_x(),COLOR_REVERSE);
	draw_text_string(2,text_size_y()-1,tag,COLOR_REVERSE);
	draw_text_string(2+strlen(tag),text_size_y()-1,keys,COLOR_INPUT);

	while (1) {
		int i;
		unsigned k;
		video_wait_vsync();

		k = os_input_get();
		if (k == OS_INPUT_ESC)
			return -1;

		for(i=0;keys[i];++i)
			if (toupper(k)==toupper(keys[i]))
				return i;
	}
}

static int cmd_input_string(const char* tag, char* buffer, unsigned length) {
	draw_text_fill(0,text_size_y()-1,' ',text_size_x(),COLOR_REVERSE);
	draw_text_string(2,text_size_y()-1,tag,COLOR_REVERSE);

	if (draw_text_read(2 + strlen(tag),text_size_y()-1,buffer,length,COLOR_INPUT) == OS_INPUT_ENTER) {
		return 0;
	}

	return -1;
}

static void cmd_rename(void) {
	video_crtc* crtc;
	char buffer[128];

	crtc = menu_current();
	if (!crtc)
		return;

	strcpy(buffer,"");
	if (cmd_input_string(" Name : ",buffer,VIDEO_NAME_MAX)!=0)
		return;

	strcpy(crtc->name,buffer);

	menu_modify();
}

static void cmd_copy(void) {
	video_crtc* crtc;
	video_crtc copy;

	crtc = menu_current();
	if (!crtc)
		return;

	copy = *crtc;
	strcpy(copy.name,"duplicated");

	menu_insert(&copy);
}

static int cmd_modeline_create(int favourite_vtotal) {
	video_crtc crtc;
	char buffer[80];
	double freq = 0;
	unsigned x;
	unsigned y;
	video_error res;

	strcpy(crtc.name,"format_created");

	strcpy(buffer,"");
	if (cmd_input_string(" Vertical clock [Hz] (example 60.0) : ",buffer,10)!=0)
		return 0;
	freq = strtod(buffer,0);
	if (freq < 10 || freq > 200) {
		video_error_description_set("Invalid vertical clock value, usually in the range 10 - 200.0 [Hz]");
		return -1;
	}

	strcpy(buffer,"");
	if (cmd_input_string(" X resolution [pixel] : ",buffer,10)!=0)
		return 0;
	x = atoi(buffer);
	if (x < 64 || x > 2048) {
		video_error_description_set("Invalid x resolution value");
		return -1;
	}

	strcpy(buffer,"");
	if (cmd_input_string(" Y resolution [pixel] : ",buffer,10)!=0)
		return 0;
	y = atoi(buffer);
	if (y < 64 || y > 2048) {
		video_error_description_set("Invalid y resolution value");
		return -1;
	}

	res = generate_find_interpolate_double(&crtc, x, y, freq, &the_monitor, &the_interpolate, VIDEO_DRIVER_FLAGS_PROGRAMMABLE_SINGLESCAN | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_DOUBLESCAN | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_INTERLACE | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_CLOCK | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_CRTC, GENERATE_ADJUST_EXACT);
	if (favourite_vtotal) {
		if (res != 0)
			res = generate_find_interpolate_double(&crtc, x, y, freq, &the_monitor, &the_interpolate, VIDEO_DRIVER_FLAGS_PROGRAMMABLE_SINGLESCAN | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_DOUBLESCAN | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_INTERLACE | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_CLOCK | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_CRTC, GENERATE_ADJUST_VCLOCK);
	} else {
		if (res != 0)
			res = generate_find_interpolate_double(&crtc, x, y, freq, &the_monitor, &the_interpolate, VIDEO_DRIVER_FLAGS_PROGRAMMABLE_SINGLESCAN | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_DOUBLESCAN | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_INTERLACE | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_CLOCK | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_CRTC, GENERATE_ADJUST_VTOTAL);
	}
	if (res != 0)
		res = generate_find_interpolate_double(&crtc, x, y, freq, &the_monitor, &the_interpolate, VIDEO_DRIVER_FLAGS_PROGRAMMABLE_SINGLESCAN | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_DOUBLESCAN | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_INTERLACE | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_CLOCK | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_CRTC, GENERATE_ADJUST_VCLOCK | GENERATE_ADJUST_VTOTAL);
	if (res != 0) {
		video_error_description_set("Requests out of your monitor range");
		return -1;
	}

	menu_insert(&crtc);

	return 0;
}

/*
static int cmd_modeline_create_gtf(void) {
	video_crtc crtc;
	char buffer[80];
	double freq = 0;
	unsigned x;
	unsigned y;

	strcpy(crtc.name,"gtf_created");

	strcpy(buffer,"");
	if (cmd_input_string(" Vertical clock [Hz] (example 60.0) : ",buffer,10)!=0)
		return 0;
	freq = strtod(buffer,0);
	if (freq < 10 || freq > 200) {
		video_error_description_set("Invalid vertical clock value, usually in the range 10 - 200.0 [Hz]");
		return -1;
	}

	strcpy(buffer,"");
	if (cmd_input_string(" X resolution [pixel] : ",buffer,10)!=0)
		return 0;
	x = atoi(buffer);
	if (x < 64 || x > 2048) {
		video_error_description_set("Invalid x resolution value");
		return -1;
	}

	strcpy(buffer,"");
	if (cmd_input_string(" Y resolution [pixel] : ",buffer,10)!=0)
		return 0;
	y = atoi(buffer);
	if (y < 64 || y > 2048) {
		video_error_description_set("Invalid y resolution value");
		return -1;
	}

	if (gtf_find(&crtc, x, y, freq, &the_monitor, &the_gtf, VIDEO_DRIVER_FLAGS_PROGRAMMABLE_SINGLESCAN | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_DOUBLESCAN | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_INTERLACE | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_CLOCK | VIDEO_DRIVER_FLAGS_PROGRAMMABLE_CRTC, GTF_ADJUST_EXACT | GTF_ADJUST_VCLOCK) != 0) {
		video_error_description_set("Request out of your monitor range");
		return -1;
	}

	menu_insert(&crtc);

	return 0;
}
*/

static int cmd_mode_clock(void) {
	video_crtc* crtc;
	char buffer[80];
	double freq = 0;

	int i = menu_base + menu_rel;
	if (i >= menu_max)
		return 0;

	crtc = video_crtc_container_pos(&the_modes,i);

	strcpy(buffer,"");

	switch (cmd_input_key(" Set Vertical/Horizontal/Pixel clock ? ","vhp")) {
		case 0 :
			if (cmd_input_string(" Vertical clock [Hz] (example 60.0) : ",buffer,10)!=0)
				return 0;
			freq = strtod(buffer,0);
			if (freq < 10 || freq > 200) {
				video_error_description_set("Invalid vertical clock value, usually in the range 10 - 200.0 [Hz]");
				return -1;
			}
			crtc->pixelclock = freq * crtc->ht * crtc->vt;
			if (crtc_is_interlace(crtc))
				crtc->pixelclock /= 2;
			if (crtc_is_doublescan(crtc))
				crtc->pixelclock *= 2;
			break;
		case 1 :
			if (cmd_input_string(" Horizontal clock [kHz] (example 31.5) : ",buffer,10)!=0)
				return 0;
			freq = strtod(buffer,0) * 1E3;
			if (freq < 10*1E3 || freq > 100*1E3) {
				video_error_description_set("Invalid horizontal clock value, usually in the range 15.0 - 80.0 [kHz]");
				return -1;
			}
			crtc->pixelclock = freq * crtc->ht;
			break;
		case 2 :
			if (cmd_input_string(" Pixel clock [MHz] (example 14.0) : ",buffer,10)!=0)
				return 0;
			freq = strtod(buffer,0) * 1E6;
			if (freq < 1*1E6 || freq > 300*1E6) {
				video_error_description_set("Invalid pixel clock value, usually in the range 1.0 - 300.0 [MHz]");
				return -1;
			}
			crtc->pixelclock = freq;
			break;
		default: return 0;
	}

	menu_modify();

	return 0;
}

static void cmd_del(void) {
	video_crtc* crtc;

	crtc = menu_current();
	if (!crtc)
		return;

	menu_remove(crtc);
}

static void cmd_save(void) {
	video_crtc_container selected;
	video_crtc_container_iterator i;
	video_crtc_container_init(&selected);

	for(video_crtc_container_iterator_begin(&i,&the_modes);!video_crtc_container_iterator_is_end(&i);video_crtc_container_iterator_next(&i)) {
		const video_crtc* crtc = video_crtc_container_iterator_get(&i);
		if (crtc->user_flags & VIDEO_FLAGS_USER_BIT0)
			video_crtc_container_insert(&selected,crtc);
	}

	video_crtc_container_save(the_config, &selected);
	video_crtc_container_done(&selected);

	the_modes_modified = 0;
}

static int cmd_exit(void) {
	if (the_modes_modified) {
		int res;

		sound_warn();

		res = cmd_input_key(" Save before exiting : ","yn");
		if (res < 0)
			return 0;

		if (res == 0)
			cmd_save();
	}

	return 1;
}

/***************************************************************************/
/* Menu main */

/* Window coordinate */
#define MENU_X 0
#define MENU_Y 3
#define MENU_DX (text_size_x())
#define MENU_DY (text_size_y()-7)

#define INFO_X 0
#define INFO_Y (text_size_y()-4)
#define INFO_DX (text_size_x())
#define INFO_DY 3

#define BAR_X 0
#define BAR_Y1 0
#define BAR_Y2 (text_size_y()-1)
#define BAR_DX (text_size_x())

/* Menu */
static int menu_run(void) {
	int done;
	int userkey;

	menu_base = 0;
	menu_rel = 0;
	menu_rel_max = MENU_DY;
	menu_max = video_crtc_container_max(&the_modes);
	menu_base_max = menu_max -  menu_rel_max;
	if (menu_base_max < 0)
		menu_base_max = 0;

	done = 0;
	while (!done) {
		draw_text_bit(BAR_X,BAR_Y1+1,BAR_DX);
		draw_text_bar(BAR_X,BAR_Y1,BAR_Y2,BAR_DX);
		draw_text_info(INFO_X,INFO_Y,INFO_DX,INFO_DY,menu_base + menu_rel);
		menu_draw(MENU_X,MENU_Y,MENU_DX,MENU_DY);

		video_wait_vsync();

		userkey = os_input_get();

		switch (userkey) {
			case OS_INPUT_UP:
				cmd_gotopos( menu_base + menu_rel - 1);
				break;
			case OS_INPUT_DOWN:
				cmd_gotopos( menu_base + menu_rel + 1 );
				break;
			case OS_INPUT_HOME: {
				int i = menu_base + menu_rel - 1;
				if (i<0)
					i = 0;
				while (i>0 && !(video_crtc_container_pos(&the_modes,i)->user_flags & VIDEO_FLAGS_USER_BIT0))
					--i;
				cmd_gotopos( i );
				break;
			}
			case OS_INPUT_END: {
				int i = menu_base + menu_rel + 1;
				if (i >= menu_max)
					i = menu_max - 1;
				while (i < menu_max - 1 && !(video_crtc_container_pos(&the_modes,i)->user_flags & VIDEO_FLAGS_USER_BIT0))
					++i;
				cmd_gotopos( i );
				break;
			}
			case OS_INPUT_PGDN:
				cmd_gotopos( menu_base + menu_rel + menu_rel_max );
				break;
			case OS_INPUT_PGUP:
				cmd_gotopos( menu_base + menu_rel - menu_rel_max );
				break;
			case OS_INPUT_F2:
				cmd_save();
				break;
			case OS_INPUT_LEFT :
			case OS_INPUT_RIGHT :
				cmd_type(userkey);
				break;
			case OS_INPUT_ESC:
				done = cmd_exit();
				break;
			case OS_INPUT_SPACE:
				cmd_select();
				cmd_gotopos( menu_base + menu_rel + 1 );
				break;
			case OS_INPUT_ENTER:
				if (cmd_onvideo_test() != 0) {
					text_reset();
					draw_text_error();
				} else {
					text_reset();
				}
				break;
			case OS_INPUT_F9:
				if (cmd_onvideo_calib() != 0) {
					text_reset();
					draw_text_error();
				} else {
					text_reset();
				}
				break;
			case OS_INPUT_F10:
				if (cmd_onvideo_animate() != 0) {
					text_reset();
					draw_text_error();
				} else {
					text_reset();
				}
				break;
			case OS_INPUT_TAB :
				cmd_rename();
				break;
			case OS_INPUT_F5 :
				if (cmd_modeline_create(1) !=0) {
					text_reset();
					draw_text_error();
				} else {
					text_reset();
				}
				break;
			case OS_INPUT_F6 :
				if (cmd_modeline_create(0) !=0) {
					text_reset();
					draw_text_error();
				} else {
					text_reset();
				}
				break;
			case OS_INPUT_F7 :
				cmd_copy();
				break;
			case OS_INPUT_F8 :
				if (cmd_mode_clock() !=0) {
					text_reset();
					draw_text_error();
				} else {
					text_reset();
				}
				break;
			case OS_INPUT_DEL :
				cmd_del();
				cmd_gotopos( menu_base + menu_rel );
				break;
			case OS_INPUT_F1:
				draw_text_help();
				break;
			default:
				if (cmd_offvideo_test(userkey) != 0) {
					draw_text_error();
				}
				break;
		}
	}
	return userkey;
}

/***************************************************************************/
/* External utilities */

#ifdef __MSDOS__

/* RUThere code for int2f */
#define VBE_RUT_ACK1 0xAD17
#define VBE_RUT_ACK2 0x17BE
#define VGA_RUT_ACK1 0xAD17
#define VGA_RUT_ACK2 0x175A

static void rut(void) {
	__dpmi_regs r;
	unsigned i;

	for(i=0xC0;i<=0xFF;++i) {
		r.h.ah = i;
		r.h.al = 0;
		r.x.bx = 0;
		r.x.cx = 0;
		r.x.dx = 0;
		__dpmi_int(0x2f,&r);
		if (r.h.al == 0xFF && r.x.cx == VBE_RUT_ACK1 && r.x.dx == VBE_RUT_ACK2) {
			the_advance_vbe_active = 1;
			break;
		}
	}

	for(i=0xC0;i<=0xFF;++i) {
		r.h.ah = i;
		r.h.al = 0;
		r.x.bx = 0;
		r.x.cx = 0;
		r.x.dx = 0;
		__dpmi_int(0x2f,&r);
		if (r.h.al == 0xFF && r.x.cx == VGA_RUT_ACK1 && r.x.dx == VGA_RUT_ACK2) {
			the_advance_vga_active = 1;
			break;
		}
	}
}

#endif

/***************************************************************************/
/* Main */

void video_log_va(const char *text, va_list arg)
{
	os_msg_va(text,arg);
}

static void error_callback(void* context, enum conf_callback_error error, const char* file, const char* tag, const char* valid, const char* desc, ...) {
	va_list arg;
	va_start(arg, desc);
	vfprintf(stderr, desc, arg);
	fprintf(stderr, "\n");
	if (valid)
		fprintf(stderr, "%s\n", valid);
	va_end(arg);
}

static struct conf_conv STANDARD[] = {
{ "*", "*", "*", "%s", "%s", "%s", 1 }
};

void os_signal(int signum) {
	os_default_signal(signum);
}

int os_main(int argc, char* argv[]) {
	video_crtc_container selected;
	video_crtc_container_iterator i;
	const char* opt_rc;
	int opt_log;
	int opt_logsync;
	int j;
	int res;
	const char* section_map[1];

	opt_rc = 0;
	opt_log = 0;
	opt_logsync = 0;
	the_advance = advance_mame;
	the_sound_flag = 1;

	the_config = conf_init();

	if (os_init(the_config)!=0) {
		fprintf(stderr,"Error initializing the OS support\n");
		goto err_conf;
	}

	video_reg(the_config);
	monitor_register(the_config);
	video_crtc_container_register(the_config);
	generate_interpolate_register(the_config);
	gtf_register(the_config);

	if (conf_input_args_load(the_config, 1, "", &argc, argv, error_callback, 0) != 0)
		goto err_os;

	for(j=1;j<argc;++j) {
		if (optionmatch(argv[j],"rc") && j+1<argc) {
			opt_rc = argv[++j];
		} else if (optionmatch(argv[j],"log")) {
			opt_log = 1;
		} else if (optionmatch(argv[j],"logsync")) {
			opt_logsync = 1;
		} else if (optionmatch(argv[j],"nosound")) {
			the_sound_flag = 0;
		} else if (optionmatch(argv[j],"advmamev")) {
			the_advance = advance_mame;
		} else if (optionmatch(argv[j],"advmessv")) {
			the_advance = advance_mess;
		} else if (optionmatch(argv[j],"advpacv")) {
			the_advance = advance_pac;
		} else if (optionmatch(argv[j],"advmenuv")) {
			the_advance = advance_menu;
		} else if (optionmatch(argv[j],"vgav")) {
			the_advance = advance_vga;
		} else if (optionmatch(argv[j],"vbev")) {
			the_advance = advance_vbe;
		} else {
			fprintf(stderr,"Unknow option %s\n",argv[j]);
			goto err;
		}
	}

#ifdef __MSDOS__
	rut();
#endif

	if (the_advance == advance_vga) {
#ifdef __MSDOS__
		if (the_advance_vga_active) {
			fprintf(stderr,"The AdvanceVGA utility is active. Disable it before running vgav.\n");
			goto err;
		}
		video_reg_driver(the_config, &video_vgaline_driver);
#else
		fprintf(stderr,"The AdvanceVGA utility works only in DOS.\n");
		goto err;
#endif
	} else if (the_advance == advance_vbe) {
#ifdef __MSDOS__
		if (the_advance_vbe_active) {
			fprintf(stderr,"The AdvanceVBE utility is active. Disable it before running vbev.\n");
			goto err;
		}
		video_reg_driver(the_config, &video_vbeline_driver);
		video_reg_driver(the_config, &video_vgaline_driver); /* for the text modes */
#else
		fprintf(stderr,"The AdvanceVBE utility works only in DOS.\n");
		goto err;
#endif
	} else {
#ifdef USE_VIDEO_SVGALIB
		video_reg_driver(the_config, &video_svgalib_driver);
#endif
#ifdef USE_VIDEO_FB
		video_reg_driver(the_config, &video_fb_driver);
#endif
#ifdef USE_VIDEO_SVGALINE
		video_reg_driver(the_config, &video_svgaline_driver);
#endif
#ifdef USE_VIDEO_VBELINE
		video_reg_driver(the_config, &video_vbeline_driver);
#endif
#ifdef USE_VIDEO_VGALINE
		video_reg_driver(the_config, &video_vgaline_driver);
#endif
#ifdef USE_VIDEO_DOS
		video_reg_driver(the_config, &video_dos_driver);
#endif
#ifdef USE_VIDEO_SLANG
		video_reg_driver(the_config, &video_slang_driver);
#endif
	}

	if (!opt_rc) {
		switch (the_advance) {
			case advance_vbe : opt_rc = "vbe.rc"; break;
			case advance_vga : opt_rc = "vga.rc"; break;
			case advance_menu : opt_rc = os_config_file_home("advmenu.rc"); break;
			case advance_mame : opt_rc = os_config_file_home("advmame.rc"); break;
			case advance_mess : opt_rc = os_config_file_home("advmess.rc"); break;
			case advance_pac : opt_rc = os_config_file_home("advpac.rc"); break;
			default : opt_rc = "advv.rc"; break;
		}
	}

	if (access(opt_rc,R_OK)!=0) {
		fprintf(stderr,"Configuration file %s not found\n", opt_rc);
		goto err_os;
	}

	if (conf_input_file_load_adv(the_config, 0, opt_rc, opt_rc, 1, 1, STANDARD, sizeof(STANDARD)/sizeof(STANDARD[0]), error_callback, 0) != 0)
		goto err_os;

	if (opt_log || opt_logsync) {
		const char* log = 0;
		switch (the_advance) {
			case advance_vbe : log = "vbev.log"; break;
			case advance_vga : log = "vgav.log"; break;
			case advance_menu : log = "advmenuv.log"; break;
			case advance_mame : log = "advmamev.log"; break;
			case advance_mess : log = "advmessv.log"; break;
			case advance_pac : log = "advpacv.log"; break;
			default: log = "advv.log"; break;
		}
		remove(log);
		os_msg_init(log,opt_logsync);
        }

	os_log(("v: %s %s\n",__DATE__,__TIME__));

	section_map[0] = "";
	conf_section_set(the_config, section_map, 1);

	if (video_load(the_config, "") != 0) {
		fprintf(stderr,"Error loading the video options from the configuration file %s\n", opt_rc);
		fprintf(stderr,"%s\n",video_error_description_get());
		goto err_os;
	}

	if (os_inner_init() != 0) {
		goto err_os;
	}

	video_init();

	video_blit_set_mmx(os_mmx_get());

	if (monitor_load(the_config, &the_monitor) != 0) {
		printf("Error loading the clock options from the configuration file %s\n", opt_rc);
		printf("%s\n",video_error_description_get());
		goto err_video;
	}

	os_log(("v: pclock %.3f - %.3f\n",(double)the_monitor.pclock.low,(double)the_monitor.pclock.high));
	for(j=0;j<VIDEO_MONITOR_RANGE_MAX;++j)
		if (the_monitor.hclock[j].low)
			os_log(("v: hclock %.3f - %.3f\n",(double)the_monitor.hclock[j].low,(double)the_monitor.hclock[j].high));
	for(j=0;j<VIDEO_MONITOR_RANGE_MAX;++j)
		if (the_monitor.vclock[j].low)
			os_log(("v: vclock %.3f - %.3f\n",(double)the_monitor.vclock[j].low,(double)the_monitor.vclock[j].high));

	/* load generate_linear config */
	res = generate_interpolate_load(the_config, &the_interpolate);
	if (res<0) {
		fprintf(stderr,"Error loading the format options from the configuration file %s.\n", opt_rc);
		fprintf(stderr,"%s\n", video_error_description_get());
		goto err_video;
	}
	if (res>0) {
		generate_default_vga(&the_interpolate.map[0].gen);
		the_interpolate.map[0].hclock = 31500;
		the_interpolate.mac = 1;
	}

	/* load generate_linear config */
	res = gtf_load(the_config, &the_gtf);
	if (res<0) {
		fprintf(stderr,"Error loading the gtf options from the configuration file %s.\n", opt_rc);
		fprintf(stderr,"%s\n", video_error_description_get());
		goto err_video;
	}
	if (res>0) {
		gtf_default_vga(&the_gtf);
	}

	/* all mode */
	video_crtc_container_init(&selected);

	if (the_advance == advance_vbe) {
		video_crtc_container_insert_default_modeline_svga(&selected);
		video_crtc_container_insert_default_bios_vga(&selected); /* for text modes */
	} else if (the_advance == advance_vga) {
		video_crtc_container_insert_default_modeline_vga(&selected);
	} else {
		video_crtc_container_insert_default_modeline_vga(&selected);
		video_crtc_container_insert_default_modeline_svga(&selected);
	}

	/* sort */
	video_crtc_container_init(&the_modes);
	for(video_crtc_container_iterator_begin(&i,&selected);!video_crtc_container_iterator_is_end(&i);video_crtc_container_iterator_next(&i)) {
		video_crtc* crtc = video_crtc_container_iterator_get(&i);
		video_crtc_container_insert_sort(&the_modes,crtc,video_crtc_compare);
	}
	video_crtc_container_done(&selected);

	/* load selected */
	video_crtc_container_init(&selected);

	if (video_crtc_container_load(the_config, &selected) != 0) {
		fprintf(stderr,video_error_description_get());
		goto err_video;
	}

	/* union set */
	for(video_crtc_container_iterator_begin(&i,&selected);!video_crtc_container_iterator_is_end(&i);video_crtc_container_iterator_next(&i)) {
		video_crtc* crtc = video_crtc_container_iterator_get(&i);
		int has = video_crtc_container_has(&the_modes,crtc,video_crtc_compare) != 0;
		if (has)
			video_crtc_container_remove(&the_modes,video_crtc_select_by_compare,crtc);
		crtc->user_flags |= VIDEO_FLAGS_USER_BIT0;
		video_crtc_container_insert_sort(&the_modes,crtc,video_crtc_compare);
	}
	video_crtc_container_done(&selected);

	the_modes_modified = 0;

	text_init();

	sound_signal();

	menu_run();

	os_log(("v: shutdown\n"));

	text_done();

	video_crtc_container_done(&the_modes);

	video_done();

	os_inner_done();

	os_log(("v: the end\n"));

	if (opt_log || opt_logsync) {
		os_msg_done();
	}

	os_done();

	conf_save(the_config,0);

	conf_done(the_config);

	return EXIT_SUCCESS;

err_video:
	video_done();
	os_inner_done();
err_os:
	if (opt_log || opt_logsync) {
		os_msg_done();
	}
	os_done();
err_conf:
	conf_done(the_config);
err:
	return EXIT_FAILURE;
}

#ifdef __MSDOS__

/* Keep Allegro small */
BEGIN_GFX_DRIVER_LIST
END_GFX_DRIVER_LIST

BEGIN_COLOR_DEPTH_LIST
END_COLOR_DEPTH_LIST

BEGIN_DIGI_DRIVER_LIST
END_DIGI_DRIVER_LIST

BEGIN_MIDI_DRIVER_LIST
END_MIDI_DRIVER_LIST

BEGIN_JOYSTICK_DRIVER_LIST
END_JOYSTICK_DRIVER_LIST

#endif