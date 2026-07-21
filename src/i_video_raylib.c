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


#include <stdlib.h>
#include <unistd.h>

#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
// #include <sys/socket.h>

// #include <netinet/in.h>
#include <errno.h>
#include <signal.h>


#include <raylib.h>
// raylib aliases to not conflict with doom
enum
{
    RL_KEY_ESCAPE      = KEY_ESCAPE,
    RL_KEY_ENTER       = KEY_ENTER,
    RL_KEY_TAB         = KEY_TAB,
    RL_KEY_BACKSPACE   = KEY_BACKSPACE,
    RL_KEY_DELETE      = KEY_DELETE,
    RL_KEY_PAUSE       = KEY_PAUSE,
    RL_KEY_EQUAL       = KEY_EQUAL,
    RL_KEY_MINUS       = KEY_MINUS,
    
    RL_KEY_F1          = KEY_F1,
    RL_KEY_F2          = KEY_F2,
    RL_KEY_F3          = KEY_F3,
    RL_KEY_F4          = KEY_F4,
    RL_KEY_F5          = KEY_F5,
    RL_KEY_F6          = KEY_F6,
    RL_KEY_F7          = KEY_F7,
    RL_KEY_F8          = KEY_F8,
    RL_KEY_F9          = KEY_F9,
    RL_KEY_F10         = KEY_F10,
    RL_KEY_F11         = KEY_F11,
    RL_KEY_F12         = KEY_F12
};

#include "i_sound.h"

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
// SDL_Window* I_window = NULL;
// SDL_Renderer* I_renderer = NULL;
// SDL_Texture* I_buffer = NULL;
Texture2D I_buffer;
static uint32_t *rgba_buffer = NULL;


// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int	multiply=1;


//
//  Translates the key passed in `key`
//
static int xlatekey(int key)
{
    switch (key)
    {
        /* Movement */
        case KEY_LEFT:
            return KEY_LEFTARROW;

        case KEY_RIGHT:
            return KEY_RIGHTARROW;

        case KEY_DOWN:
            return KEY_DOWNARROW;

        case KEY_UP:
            return KEY_UPARROW;

        /* General controls */
        case RL_KEY_ESCAPE:
            return KEY_ESCAPE;

        case RL_KEY_ENTER:
        case KEY_KP_ENTER:
            return KEY_ENTER;

        case RL_KEY_TAB:
            return KEY_TAB;

        /* Function keys */
        case RL_KEY_F1:
            return KEY_F1;

        case RL_KEY_F2:
            return KEY_F2;

        case RL_KEY_F3:
            return KEY_F3;

        case RL_KEY_F4:
            return KEY_F4;

        case RL_KEY_F5:
            return KEY_F5;

        case RL_KEY_F6:
            return KEY_F6;

        case RL_KEY_F7:
            return KEY_F7;

        case RL_KEY_F8:
            return KEY_F8;

        case RL_KEY_F9:
            return KEY_F9;

        case RL_KEY_F10:
            return KEY_F10;

        case RL_KEY_F11:
            return KEY_F11;

        case RL_KEY_F12:
            return KEY_F12;

        /* Editing */
        case RL_KEY_BACKSPACE:
        case RL_KEY_DELETE:
            return KEY_BACKSPACE;

        case RL_KEY_PAUSE:
            return KEY_PAUSE;

        /* Plus and minus */
        case RL_KEY_EQUAL:
        case KEY_KP_EQUAL:
            return KEY_EQUALS;

        case RL_KEY_MINUS:
        case KEY_KP_SUBTRACT:
            return KEY_MINUS;

        /* Modifiers */
        case KEY_LEFT_SHIFT:
        case KEY_RIGHT_SHIFT:
            return KEY_RSHIFT;

        case KEY_LEFT_CONTROL:
        case KEY_RIGHT_CONTROL:
            return KEY_RCTRL;

        case KEY_LEFT_ALT:
        case KEY_RIGHT_ALT:
        case KEY_LEFT_SUPER:
        case KEY_RIGHT_SUPER:
            return KEY_RALT;

        default:
            break;
    }

    /*
     * raylib letter key values use uppercase ASCII.
     * Doom expects lowercase ASCII.
     */
    if (key >= KEY_A && key <= KEY_Z)
        return 'a' + (key - KEY_A);

    /*
     * raylib number keys and common punctuation use ASCII values.
     */
    if (key >= ' ' && key <= '~')
        return key;

    return 0;
}

void I_ShutdownGraphics(void)
{
	free(rgba_buffer);
    rgba_buffer = NULL;
  	UnloadTexture(I_buffer);
    CloseWindow();
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

boolean i_running = true;

static void PostKeyEvent(int raylib_key, int doom_key)
{
    event_t event;

    if (IsKeyPressed(raylib_key))
    {
        event.type = ev_keydown;
        event.data1 = doom_key;
        event.data2 = 0;
        event.data3 = 0;

        D_PostEvent(&event);
    }

    if (IsKeyReleased(raylib_key))
    {
        event.type = ev_keyup;
        event.data1 = doom_key;
        event.data2 = 0;
        event.data3 = 0;

        D_PostEvent(&event);
    }
}

static int GetDoomMouseButtons(void)
{
    int buttons = 0;

    // Doom:
    // bit 0 = left
    // bit 1 = right
    // bit 2 = middle

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        buttons |= 1;

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        buttons |= 2;

    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
        buttons |= 4;

    return buttons;
}

void I_GetEvent(void)
{
    event_t event;
    Vector2 mouse_delta;

    /*
     * Keyboard events
     *
     * Using IsKeyPressed()/IsKeyReleased() avoids raylib's automatic
     * key-repeat becoming repeated Doom key-down events.
     */
    PostKeyEvent(KEY_LEFT,        KEY_LEFTARROW);
    PostKeyEvent(KEY_RIGHT,       KEY_RIGHTARROW);
    PostKeyEvent(KEY_DOWN,        KEY_DOWNARROW);
    PostKeyEvent(KEY_UP,          KEY_UPARROW);

    PostKeyEvent(KEY_W, 'w');
    PostKeyEvent(KEY_S, 's');
    PostKeyEvent(KEY_A, 'a');
    PostKeyEvent(KEY_D, 'd');

    PostKeyEvent(RL_KEY_ESCAPE,   KEY_ESCAPE);
    PostKeyEvent(RL_KEY_ENTER,    KEY_ENTER);
    PostKeyEvent(KEY_KP_ENTER,    KEY_ENTER);
    PostKeyEvent(RL_KEY_TAB,      KEY_TAB);

    PostKeyEvent(RL_KEY_F1,       KEY_F1);
    PostKeyEvent(RL_KEY_F2,       KEY_F2);
    PostKeyEvent(RL_KEY_F3,       KEY_F3);
    PostKeyEvent(RL_KEY_F4,       KEY_F4);
    PostKeyEvent(RL_KEY_F5,       KEY_F5);
    PostKeyEvent(RL_KEY_F6,       KEY_F6);
    PostKeyEvent(RL_KEY_F7,       KEY_F7);
    PostKeyEvent(RL_KEY_F8,       KEY_F8);
    PostKeyEvent(RL_KEY_F9,       KEY_F9);
    PostKeyEvent(RL_KEY_F10,      KEY_F10);
    PostKeyEvent(RL_KEY_F11,      KEY_F11);
    PostKeyEvent(RL_KEY_F12,      KEY_F12);

    PostKeyEvent(RL_KEY_BACKSPACE, KEY_BACKSPACE);
    PostKeyEvent(RL_KEY_DELETE,    KEY_BACKSPACE);
    PostKeyEvent(RL_KEY_PAUSE,     KEY_PAUSE);

    PostKeyEvent(RL_KEY_EQUAL,     KEY_EQUALS);
    PostKeyEvent(KEY_KP_EQUAL,     KEY_EQUALS);
    PostKeyEvent(RL_KEY_MINUS,     KEY_MINUS);
    PostKeyEvent(KEY_KP_SUBTRACT,  KEY_MINUS);

    PostKeyEvent(KEY_LEFT_SHIFT,    KEY_RSHIFT);
    PostKeyEvent(KEY_RIGHT_SHIFT,   KEY_RSHIFT);
    PostKeyEvent(KEY_LEFT_CONTROL,  KEY_RCTRL);
    PostKeyEvent(KEY_RIGHT_CONTROL, KEY_RCTRL);
    PostKeyEvent(KEY_LEFT_ALT,      KEY_RALT);
    PostKeyEvent(KEY_RIGHT_ALT,     KEY_RALT);
    PostKeyEvent(KEY_LEFT_SUPER,    KEY_RALT);
    PostKeyEvent(KEY_RIGHT_SUPER,   KEY_RALT);

    /*
     * Printable keys used by Doom menus, cheats, and save-game names.
     */
    for (int ray_key = KEY_ZERO; ray_key <= KEY_NINE; ray_key++)
    {
        PostKeyEvent(ray_key, '0' + (ray_key - KEY_ZERO));
    }

    for (int ray_key = KEY_A; ray_key <= KEY_Z; ray_key++)
    {
        int doom_key = 'a' + (ray_key - KEY_A);

        /*
         * W, A, S, and D are remapped above, so do not post them twice.
         */
        if (ray_key == KEY_W ||
            ray_key == KEY_A ||
            ray_key == KEY_S ||
            ray_key == KEY_D)
        {
            continue;
        }

        PostKeyEvent(ray_key, doom_key);
    }

    PostKeyEvent(KEY_SPACE,         ' ');
    PostKeyEvent(KEY_APOSTROPHE,    '\'');
    PostKeyEvent(KEY_COMMA,         ',');
    PostKeyEvent(KEY_PERIOD,        '.');
    PostKeyEvent(KEY_SLASH,         '/');
    PostKeyEvent(KEY_SEMICOLON,     ';');
    PostKeyEvent(KEY_LEFT_BRACKET,  '[');
    PostKeyEvent(KEY_RIGHT_BRACKET, ']');
    PostKeyEvent(KEY_BACKSLASH,     '\\');
    PostKeyEvent(KEY_GRAVE,         '`');

    /*
     * Mouse buttons and relative movement.
     *
     * GetMouseDelta() is the raylib equivalent of SDL's xrel/yrel.
     */
    mouse_delta = GetMouseDelta();

    if (mouse_delta.x != 0.0f ||
        mouse_delta.y != 0.0f ||
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
        IsMouseButtonReleased(MOUSE_BUTTON_LEFT) ||
        IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
        IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) ||
        IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) ||
        IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE))
    {
        event.type = ev_mouse;
        event.data1 = GetDoomMouseButtons();
        event.data2 = (int)mouse_delta.x << 2;
        event.data3 = -((int)mouse_delta.y << 2);

        D_PostEvent(&event);
    }

    if (WindowShouldClose())
    {
        i_running = false;
    }
}

boolean shouldRun() {return i_running;}

//
// I_StartTic
//
void I_StartTic (void)
{
    I_GetEvent();
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

    I_UpdateMusic();

    const int pixel_count = SCREENWIDTH * SCREENHEIGHT;

    for (int i = 0; i < pixel_count; ++i)
    {
        byte palette_index = screens[0][i];

        byte r = colors[palette_index][0];
        byte g = colors[palette_index][1];
        byte b = colors[palette_index][2];

        uint8_t *pixel = (uint8_t *)&rgba_buffer[i];

        pixel[0] = r;
        pixel[1] = g;
        pixel[2] = b;
        pixel[3] = 255;

    }

    UpdateTexture(I_buffer, rgba_buffer);

    BeginDrawing();

    DrawTexturePro(
        I_buffer,
        (Rectangle){
            0.0f,
            0.0f,
            (float)I_buffer.width,
            (float)I_buffer.height
        },
        (Rectangle){
            0.0f,
            0.0f,
            (float)GetScreenWidth(),
            (float)GetScreenHeight()
        },
        (Vector2){ 0.0f, 0.0f },
        0.0f,
        WHITE
    );

    EndDrawing();
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

    static boolean	firsttime=true;

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

    if (M_CheckParm("-5"))
    multiply = 5;

    X_width = SCREENWIDTH * multiply;
    X_height = SCREENHEIGHT * multiply;

    // check for command-line display name
    if ( (pnum=M_CheckParm("-disp")) ) // suggest parentheses around assignment
		displayname = myargv[pnum+1];
    else
		displayname = "DOOM";


    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(SCREENWIDTH * multiply, SCREENHEIGHT * multiply, "DOOM");

    DisableCursor();

    Image img = GenImageColor(SCREENWIDTH, SCREENHEIGHT, GRAY);
    I_buffer = LoadTextureFromImage(img);

	SetTextureFilter(I_buffer, TEXTURE_FILTER_POINT);

	rgba_buffer = malloc(
        (size_t)SCREENWIDTH *
        (size_t)SCREENHEIGHT *
        sizeof(*rgba_buffer)
	);

	if (!rgba_buffer)
	{
		I_Error("DoomI: failed to allocate RGBA framebuffer");
	}

    UnloadImage(img);

    SetExitKey(KEY_NULL);
}