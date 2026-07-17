// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// Dummy networking backend.
// Networking is disabled; only single-player mode is supported.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

#include "i_system.h"
#include "i_net.h"

#include "d_net.h"
#include "doomstat.h"
#include "m_argv.h"


//
// I_InitNetwork
//
// Initializes Doom's networking structures for a single-player game.
// No sockets or operating-system networking APIs are used.
//
void I_InitNetwork(void)
{
    int parm;

    doomcom = malloc(sizeof(*doomcom));

    if (doomcom == NULL)
    {
        I_Error("I_InitNetwork: failed to allocate doomcom");
    }

    memset(doomcom, 0, sizeof(*doomcom));

    doomcom->id = DOOMCOM_ID;

    // Single-player configuration.
    doomcom->consoleplayer = 0;
    doomcom->numplayers = 1;
    doomcom->numnodes = 1;

    doomcom->deathmatch = 0;
    doomcom->extratics = 0;
    doomcom->ticdup = 1;

    // Preserve the original -dup parameter for compatibility.
    parm = M_CheckParm("-dup");

    if (parm != 0 && parm < myargc - 1)
    {
        doomcom->ticdup = myargv[parm + 1][0] - '0';

        if (doomcom->ticdup < 1)
        {
            doomcom->ticdup = 1;
        }
        else if (doomcom->ticdup > 9)
        {
            doomcom->ticdup = 9;
        }
    }

    if (M_CheckParm("-extratic"))
    {
        doomcom->extratics = 1;
    }

    // Ignore multiplayer arguments because this backend has no networking.
    if (M_CheckParm("-net"))
    {
        I_Error(
            "Networking is disabled in this build. "
            "Remove the -net command-line argument."
        );
    }

    netgame = false;
}


//
// I_NetCmd
//
// Dummy implementation of the platform networking command.
//
// CMD_GET reports that no packet is available.
// CMD_SEND is ignored because single-player packets are handled internally
// by d_net.c and should not reach the platform backend.
//
void I_NetCmd(void)
{
    if (doomcom == NULL)
    {
        I_Error("I_NetCmd called before I_InitNetwork");
    }

    switch (doomcom->command)
    {
        case CMD_GET:
            // A remotenode value of -1 means no packet is available.
            doomcom->remotenode = -1;
            doomcom->datalength = 0;
            break;

        case CMD_SEND:
            // There are no remote nodes in a single-player game.
            break;

        default:
            I_Error("I_NetCmd: invalid command %i", doomcom->command);
            break;
    }
}
