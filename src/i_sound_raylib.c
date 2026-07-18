// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// SDL sound backend replacement file.
// The obsolete Linux OSS (/dev/dsp), sndserver, signal timer, and original
// software mixer have been removed.
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>

#include "doomdef.h"
#include "i_sound.h"
#include "i_system.h"
#include "w_wad.h"
#include "z_zone.h"
#undef KEY_ESCAPE
#undef KEY_ENTER
#undef KEY_TAB
#undef KEY_BACKSPACE
#undef KEY_PAUSE
#undef KEY_EQUAL
#undef KEY_MINUS

#undef KEY_F1
#undef KEY_F2
#undef KEY_F3
#undef KEY_F4
#undef KEY_F5
#undef KEY_F6
#undef KEY_F7
#undef KEY_F8
#undef KEY_F9
#undef KEY_F10
#undef KEY_F11
#undef KEY_F12
#include <raylib.h>
