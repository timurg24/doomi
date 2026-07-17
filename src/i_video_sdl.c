// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <unistd.h>

#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

#define POINTER_WARP_COUNTDOWN	1

int X_width;
int X_height;

// Modern
SDL_Window* I_window = NULL;
SDL_Renderer* I_renderer = NULL;
SDL_Texture* I_buffer = NULL;
static uint32_t *rgba_buffer = NULL;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int	multiply=1;


//
//  Translates the key passed in `key`
//
static int xlatekey(SDL_Keycode key)
{
    int rc = (int)key;

    switch (key)
    {
        case SDLK_LEFT:       return KEY_LEFTARROW;
        case SDLK_RIGHT:      return KEY_RIGHTARROW;
        case SDLK_DOWN:       return KEY_DOWNARROW;
        case SDLK_UP:         return KEY_UPARROW;

        case SDLK_ESCAPE:     return KEY_ESCAPE;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:   return KEY_ENTER;

        case SDLK_TAB:        return KEY_TAB;

        case SDLK_F1:         return KEY_F1;
        case SDLK_F2:         return KEY_F2;
        case SDLK_F3:         return KEY_F3;
        case SDLK_F4:         return KEY_F4;
        case SDLK_F5:         return KEY_F5;
        case SDLK_F6:         return KEY_F6;
        case SDLK_F7:         return KEY_F7;
        case SDLK_F8:         return KEY_F8;
        case SDLK_F9:         return KEY_F9;
        case SDLK_F10:        return KEY_F10;
        case SDLK_F11:        return KEY_F11;
        case SDLK_F12:        return KEY_F12;

        case SDLK_BACKSPACE:
        case SDLK_DELETE:
            return KEY_BACKSPACE;

        case SDLK_PAUSE:
            return KEY_PAUSE;

        case SDLK_EQUALS:
        case SDLK_KP_EQUALS:
            return KEY_EQUALS;

        case SDLK_MINUS:
        case SDLK_KP_MINUS:
            return KEY_MINUS;

        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            return KEY_RSHIFT;

        case SDLK_LCTRL:
        case SDLK_RCTRL:
            return KEY_RCTRL;

        case SDLK_LALT:
        case SDLK_RALT:
        case SDLK_LGUI:
        case SDLK_RGUI:
            return KEY_RALT;

        default:
            break;
    }

    /*
     * Printable SDL keycodes generally use their Unicode/ASCII value.
     * DOOM expects lowercase letters.
     */
    if (rc >= 'A' && rc <= 'Z')
        rc = rc - 'A' + 'a';

    if (rc >= ' ' && rc <= '~')
        return rc;

    return 0;
}

void I_ShutdownGraphics(void)
{
	free(rgba_buffer);
    rgba_buffer = NULL;
  	SDL_DestroyTexture(I_buffer);
    SDL_DestroyRenderer(I_renderer);
    SDL_DestroyWindow(I_window);
    SDL_Quit();
}


// why would you put empty functions

//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?
}

static int	lastmousex = 0;
static int	lastmousey = 0;
boolean		mousemoved = false;
boolean		shmFinished;

bool i_running = true;

void I_GetEvent(const SDL_Event *sdl_event)
{

    event_t event;

	switch(sdl_event->type) {
		case SDL_EVENT_KEY_DOWN:
			if(sdl_event->key.repeat) break;

			event.type = ev_keydown;
			event.data1 = xlatekey(sdl_event->key.key);
			event.data2 = 0;
			event.data3 = 0;

			if(event.data1 != 0) D_PostEvent(&event);
			break;
		
		case SDL_EVENT_KEY_UP:
			event.type = ev_keyup;
			event.data1 = xlatekey(sdl_event->key.key);
			event.data2 = 0;
			event.data3 = 0;
		
			if(event.data1 != 0) D_PostEvent(&event);
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP: {
			event.type = ev_mouse;
			event.data1 = 0;
			event.data2 = 0;
			event.data3 = 0;

			// bit 0 = left
			// bit 1 = right
			// bit 2 = middle
			// (since when did they have middle mouse s_buttons in 1993?)

			SDL_MouseButtonFlags s_buttons = SDL_GetMouseState(NULL, NULL);

			if(s_buttons & SDL_BUTTON_LMASK) event.data1 |= 1;
			if(s_buttons & SDL_BUTTON_RMASK) event.data1 |= 2;
			if(s_buttons & SDL_BUTTON_MMASK) event.data1 |= 4;
			D_PostEvent(&event);
			break;
		}

		case SDL_EVENT_MOUSE_MOTION: {
			event.type = ev_mouse;
			event.data1 = 0;
			event.data2 = 0;
			event.data3 = 0; // did default values not exist in the dinosaur age?

			SDL_MouseButtonFlags s_buttons = SDL_GetMouseState(NULL, NULL);

            if (s_buttons & SDL_BUTTON_LMASK)
                event.data1 |= 1;

            if (s_buttons & SDL_BUTTON_RMASK)
                event.data1 |= 2;

            if (s_buttons & SDL_BUTTON_MMASK)
                event.data1 |= 4;

            event.data2 = (int)sdl_event->motion.xrel << 2;
            event.data3 = -((int)sdl_event->motion.yrel << 2);

            if (event.data2 != 0 || event.data3 != 0)
                D_PostEvent(&event);
            break;
		}

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_QUIT:
            i_running = false;
            break;

        default:
            break;
	}

}

bool shouldRun() {return i_running;}

//
// I_StartTic
//
void I_StartTic (void)
{
	SDL_Event event;

	while(SDL_PollEvent(&event)) {
		I_GetEvent(&event);
	}
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

static byte colors[256][3]; // palette
//
// I_FinishUpdate
//
void I_FinishUpdate(void)
{
    const int pixel_count = SCREENWIDTH * SCREENHEIGHT;

    for (int i = 0; i < pixel_count; ++i)
    {
        byte palette_index = screens[0][i];

        byte r = colors[palette_index][0];
        byte g = colors[palette_index][1];
        byte b = colors[palette_index][2];

        rgba_buffer[i] =
            ((uint32_t)r << 24) |
            ((uint32_t)g << 16) |
            ((uint32_t)b << 8)  |
            0xff;
    }

    if (!SDL_UpdateTexture(
            I_buffer,
            NULL,
            rgba_buffer,
            SCREENWIDTH * sizeof(*rgba_buffer)))
    {
        I_Error(
            "DoomI: SDL_UpdateTexture failed: %s",
            SDL_GetError()
        );
    }

    SDL_RenderClear(I_renderer);

    SDL_RenderTexture(
        I_renderer,
        I_buffer,
        NULL,
        NULL
    );

    SDL_RenderPresent(I_renderer);
}

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}


//
// Palette stuff.
//

void UploadNewPalette(byte *palette)
{

    for (int i = 0; i < 256; i++)
    {
        colors[i][0] = *palette++;
        colors[i][1] = *palette++;
        colors[i][2] = *palette++;
    }
}

//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
    UploadNewPalette(palette);
}

void I_InitGraphics(void)
{

    char*		displayname;
    int			pnum;

    static bool	firsttime=true;

    if (!firsttime)
		return;
    firsttime = false;

    signal(SIGINT, (void (*)(int)) I_Quit);

    if (M_CheckParm("-2"))
	multiply = 2;

    if (M_CheckParm("-3"))
	multiply = 3;

    if (M_CheckParm("-4"))
	multiply = 4;

    X_width = SCREENWIDTH * multiply;
    X_height = SCREENHEIGHT * multiply;

    // check for command-line display name
    if ( (pnum=M_CheckParm("-disp")) ) // suggest parentheses around assignment
		displayname = myargv[pnum+1];
    else
		displayname = "DOOM";

	if(!SDL_Init(SDL_INIT_VIDEO))
		I_Error("DoomI: SDL initialization failrue: %s", SDL_GetError());

	if(!SDL_CreateWindowAndRenderer(
		displayname,
		X_width,
		X_height,
		SDL_WINDOW_MOUSE_CAPTURE,
		&I_window,
		&I_renderer
	)) {
		const char* error = SDL_GetError();
		SDL_Quit();
		I_Error("DoomI: failed to create window: %s", error);
	}
	
	I_buffer = SDL_CreateTexture(
		I_renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREENWIDTH,
		SCREENHEIGHT
	);

	if(!I_buffer) {
		const char* error = SDL_GetError();
		SDL_DestroyRenderer(I_renderer);
		SDL_DestroyWindow(I_window);
		SDL_Quit();
		I_Error("DoomI: failed to create screen buffer texture: %s", error);
	}

	SDL_SetTextureScaleMode(I_buffer, SDL_SCALEMODE_NEAREST);

	rgba_buffer = malloc(
    (size_t)SCREENWIDTH *
    (size_t)SCREENHEIGHT *
    sizeof(*rgba_buffer)
	);

	if (!rgba_buffer)
	{
		I_Error("DoomI: failed to allocate RGBA framebuffer");
	}
}