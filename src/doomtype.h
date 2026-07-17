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
// DESCRIPTION:
//	Simple basic typedefs, isolated here to make it easier
//	 separating modules.
//    
//-----------------------------------------------------------------------------


#ifndef __DOOMTYPE__
#define __DOOMTYPE__


#include <stdbool.h>
#include <stdint.h>

// typedef int32_t bool;
typedef int boolean;

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

typedef unsigned char byte;

#include <limits.h>

#ifndef MAXCHAR
    #define MAXCHAR   SCHAR_MAX
#endif
#ifndef MINCHAR
    #define MINCHAR   SCHAR_MIN
#endif

#ifndef MAXSHORT
    #define MAXSHORT  SHRT_MAX
#endif
#ifndef MINSHORT
    #define MINSHORT  SHRT_MIN
#endif

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
