// WL_ACT1.C

#include "wl_def.h"
#pragma hdrstop

/*
=============================================================================

                                                        STATICS

=============================================================================
*/

statobj_t statobjlist[MAXSTATS];
statobj_t *laststatobj;

struct
{
    short picnum;
    wl_stat_t type;
    uint32_t specialFlags; // they are ORed to the statobj_t flags
} statinfo[] = {
    {SPR_STAT_0},                       // puddle          spr1v
    {SPR_STAT_1, block},                // Green Barrel    "
    {SPR_STAT_2, block},                // Table/chairs    "
    {SPR_STAT_3, block, FL_FULLBRIGHT}, // Floor lamp      "
    {SPR_STAT_4, none, FL_FULLBRIGHT},  // Chandelier      "
    {SPR_STAT_5, block},                // Hanged man      "
    {SPR_STAT_6, bo_alpo},              // Bad food        "
    {SPR_STAT_7, block},                // Red pillar      "
    //
    // NEW PAGE
    //
    {SPR_STAT_8, block},                // Tree            spr2v
    {SPR_STAT_9},                       // Skeleton flat   "
    {SPR_STAT_10, block},               // Sink            " (SOD:gibs)
    {SPR_STAT_11, block},               // Potted plant    "
    {SPR_STAT_12, block},               // Urn             "
    {SPR_STAT_13, block},               // Bare table      "
    {SPR_STAT_14, none, FL_FULLBRIGHT}, // Ceiling light   "
#ifndef SPEAR
    {SPR_STAT_15}, // Kitchen stuff   "
#else
    {SPR_STAT_15, block}, // Gibs!
#endif
    //
    // NEW PAGE
    //
    {SPR_STAT_16, block},   // suit of armor   spr3v
    {SPR_STAT_17, block},   // Hanging cage    "
    {SPR_STAT_18, block},   // SkeletoninCage  "
    {SPR_STAT_19},          // Skeleton relax  "
    {SPR_STAT_20, bo_key1}, // Key 1           "
    {SPR_STAT_21, bo_key2}, // Key 2           "
    {SPR_STAT_22, block},   // stuff             (SOD:gibs)
    {SPR_STAT_23},          // stuff
    //
    // NEW PAGE
    //
    {SPR_STAT_24, bo_food},       // Good food       spr4v
    {SPR_STAT_25, bo_firstaid},   // First aid       "
    {SPR_STAT_26, bo_clip},       // Clip            "
    {SPR_STAT_27, bo_machinegun}, // Machine gun     "
    {SPR_STAT_28, bo_chaingun},   // Gatling gun     "
    {SPR_STAT_29, bo_cross},      // Cross           "
    {SPR_STAT_30, bo_chalice},    // Chalice         "
    {SPR_STAT_31, bo_bible},      // Bible           "
    //
    // NEW PAGE
    //
    {SPR_STAT_32, bo_crown},                   // crown           spr5v
    {SPR_STAT_33, bo_fullheal, FL_FULLBRIGHT}, // one up          "
    {SPR_STAT_34, bo_gibs},                    // gibs            "
    {SPR_STAT_35, block},                      // barrel          "
    {SPR_STAT_36, block},                      // well            "
    {SPR_STAT_37, block},                      // Empty well      "
    {SPR_STAT_38, bo_gibs},                    // Gibs 2          "
    {SPR_STAT_39, block},                      // flag            "
//
// NEW PAGE
//
#ifndef SPEAR
    {SPR_STAT_40, block}, // Call Apogee          spr7v
#else
    {SPR_STAT_40},        // Red light
#endif
    //
    // NEW PAGE
    //
    {SPR_STAT_41}, // junk            "
    {SPR_STAT_42}, // junk            "
    {SPR_STAT_43}, // junk            "
#ifndef SPEAR
    {SPR_STAT_44}, // pots            "
#else
    {SPR_STAT_44, block}, // Gibs!
#endif
    {SPR_STAT_45, block}, // stove           " (SOD:gibs)
    {SPR_STAT_46, block}, // spears          " (SOD:gibs)
    {SPR_STAT_47},        // vines           "
//
// NEW PAGE
//
#ifdef SPEAR
    {SPR_STAT_48, block},     // marble pillar
    {SPR_STAT_49, bo_25clip}, // bonus 25 clip
    {SPR_STAT_50, block},     // truck
    {SPR_STAT_51, bo_spear},  // SPEAR OF DESTINY!
#endif

    {SPR_STAT_26, bo_clip2}, // Clip            "

    {-1} // terminator
};

/*
===============
=
= InitStaticList
=
===============
*/

void InitStaticList(void)
{
    laststatobj = &statobjlist[0];
}

/*
===============
=
= SpawnStatic
=
===============
*/

void SpawnStatic(int tilex, int tiley, int type)
{
    laststatobj->shapenum = statinfo[type].picnum;
    laststatobj->tilex = tilex;
    laststatobj->tiley = tiley;
    laststatobj->visspot = &spotvis[tilex][tiley];

    switch (statinfo[type].type)
    {
    case block:
        actorat[tilex][tiley] = (objtype *)64; // consider it a blocking tile
    case none:
        laststatobj->flags = 0;
        break;

    case bo_cross:
    case bo_chalice:
    case bo_bible:
    case bo_crown:
    case bo_fullheal:
        if (!loadedgame)
            gamestate.treasuretotal++;

    case bo_firstaid:
    case bo_key1:
    case bo_key2:
    case bo_key3:
    case bo_key4:
    case bo_clip:
    case bo_25clip:
    case bo_machinegun:
    case bo_chaingun:
    case bo_food:
    case bo_alpo:
    case bo_gibs:
    case bo_spear:
        laststatobj->flags = FL_BONUS;
        laststatobj->itemnumber = statinfo[type].type;
        break;
    }

    laststatobj->flags |= statinfo[type].specialFlags;

    laststatobj++;

    if (laststatobj == &statobjlist[MAXSTATS])
        Quit("Too many static objects!\n");
}

/*
===============
=
= PlaceItemType
=
= Called during game play to drop actors' items.  It finds the proper
= item number based on the item type (bo_???).  If there are no free item
= spots, nothing is done.
=
===============
*/

void PlaceItemType(int itemtype, int tilex, int tiley)
{
    int type;
    statobj_t *spot;

    //
    // find the item number
    //
    for (type = 0;; type++)
    {
        if (statinfo[type].picnum == -1) // end of list
            Quit("PlaceItemType: couldn't find type!");
        if (statinfo[type].type == itemtype)
            break;
    }

    //
    // find a spot in statobjlist to put it in
    //
    for (spot = &statobjlist[0];; spot++)
    {
        if (spot == laststatobj)
        {
            if (spot == &statobjlist[MAXSTATS])
                return;    // no free spots
            laststatobj++; // space at end
            break;
        }

        if (spot->shapenum == -1) // -1 is a free spot
            break;
    }
    //
    // place it
    //
    spot->shapenum = statinfo[type].picnum;
    spot->tilex = tilex;
    spot->tiley = tiley;
    spot->visspot = &spotvis[tilex][tiley];
    spot->flags = FL_BONUS | statinfo[type].specialFlags;
    spot->itemnumber = statinfo[type].type;
}

/*
=============================================================================

                                  DOORS

doorobjlist[] holds most of the information for the doors

doorposition[] holds the amount the door is open, ranging from 0 to 0xffff
        this is directly accessed by AsmRefresh during rendering

The number of doors is limited to 64 because a spot in tilemap holds the
        door number in the low 6 bits, with the high bit meaning a door center
        and bit 6 meaning a door side tile

Open doors conect two areas, so sounds will travel between them and sight
        will be checked when the player is in a connected area.

Areaconnect is incremented/decremented by each door. If >0 they connect

Every time a door opens or closes the areabyplayer matrix gets recalculated.
        An area is true if it connects with the player's current spor.

=============================================================================
*/

#define DOORWIDTH 0x7800
#define OPENTICS 300

doorobj_t doorobjlist[MAXDOORS], *lastdoorobj;
short doornum;

word doorposition[MAXDOORS]; // leading edge of door 0=closed
                             // 0xffff = fully open

byte areaconnect[NUMAREAS][NUMAREAS];

boolean areabyplayer[NUMAREAS];

/*
==============
=
= ConnectAreas
=
= Scans outward from playerarea, marking all connected areas
=
==============
*/

void RecursiveConnect(int areanumber)
{
    int i;

    for (i = 0; i < NUMAREAS; i++)
    {
        if (areaconnect[areanumber][i] && !areabyplayer[i])
        {
            areabyplayer[i] = true;
            RecursiveConnect(i);
        }
    }
}

void ConnectAreas(void)
{
    memset(areabyplayer, 0, sizeof(areabyplayer));
    areabyplayer[player->areanumber] = true;
    RecursiveConnect(player->areanumber);
}

void InitAreas(void)
{
    memset(areabyplayer, 0, sizeof(areabyplayer));
    if (player->areanumber < NUMAREAS)
        areabyplayer[player->areanumber] = true;
}

/*
===============
=
= InitDoorList
=
===============
*/

void InitDoorList(void)
{
    memset(areabyplayer, 0, sizeof(areabyplayer));
    memset(areaconnect, 0, sizeof(areaconnect));

    lastdoorobj = &doorobjlist[0];
    doornum = 0;
}

/*
===============
=
= SpawnDoor
=
===============
*/

void SpawnDoor(int tilex, int tiley, boolean vertical, int lock)
{
    word *map;

    if (doornum == MAXDOORS)
        Quit("64+ doors on level!");

    doorposition[doornum] = 0; // doors start out fully closed
    lastdoorobj->tilex = tilex;
    lastdoorobj->tiley = tiley;
    lastdoorobj->vertical = vertical;
    lastdoorobj->lock = lock;
    lastdoorobj->action = dr_closed;

    actorat[tilex][tiley] = (objtype *)(uintptr_t)(doornum | 0x80); // consider it a solid wall

    //
    // make the door tile a special tile, and mark the adjacent tiles
    // for door sides
    //
    tilemap[tilex][tiley] = doornum | 0x80;
    map = mapsegs[0] + (tiley << mapshift) + tilex;
    if (vertical)
    {
        *map = *(map - 1); // set area number
        tilemap[tilex][tiley - 1] |= 0x40;
        tilemap[tilex][tiley + 1] |= 0x40;
    }
    else
    {
        *map = *(map - mapwidth); // set area number
        tilemap[tilex - 1][tiley] |= 0x40;
        tilemap[tilex + 1][tiley] |= 0x40;
    }

    doornum++;
    lastdoorobj++;
}

//===========================================================================

/*
=====================
=
= OpenDoor
=
=====================
*/

void OpenDoor(int door)
{
    if (doorobjlist[door].action == dr_open)
        doorobjlist[door].ticcount = 0; // reset open time
    else
        doorobjlist[door].action = dr_opening; // start it opening
}

/*
=====================
=
= CloseDoor
=
=====================
*/

void CloseDoor(int door)
{
    int tilex, tiley, area;
    objtype *check;

    //
    // don't close on anything solid
    //
    tilex = doorobjlist[door].tilex;
    tiley = doorobjlist[door].tiley;

    if (actorat[tilex][tiley])
        return;

    if (player->tilex == tilex && player->tiley == tiley)
        return;

    if (doorobjlist[door].vertical)
    {
        if (player->tiley == tiley)
        {
            if (((player->x + MINDIST) >> TILESHIFT) == tilex)
                return;
            if (((player->x - MINDIST) >> TILESHIFT) == tilex)
                return;
        }
        check = actorat[tilex - 1][tiley];
        if (ISPOINTER(check) && ((check->x + MINDIST) >> TILESHIFT) == tilex)
            return;
        check = actorat[tilex + 1][tiley];
        if (ISPOINTER(check) && ((check->x - MINDIST) >> TILESHIFT) == tilex)
            return;
    }
    else if (!doorobjlist[door].vertical)
    {
        if (player->tilex == tilex)
        {
            if (((player->y + MINDIST) >> TILESHIFT) == tiley)
                return;
            if (((player->y - MINDIST) >> TILESHIFT) == tiley)
                return;
        }
        check = actorat[tilex][tiley - 1];
        if (ISPOINTER(check) && ((check->y + MINDIST) >> TILESHIFT) == tiley)
            return;
        check = actorat[tilex][tiley + 1];
        if (ISPOINTER(check) && ((check->y - MINDIST) >> TILESHIFT) == tiley)
            return;
    }

    //
    // play door sound if in a connected area
    //
    area = *(mapsegs[0] + (doorobjlist[door].tiley << mapshift) + doorobjlist[door].tilex) - AREATILE;
    if (areabyplayer[area])
    {
        PlaySoundLocTile(CLOSEDOORSND, doorobjlist[door].tilex,
                         doorobjlist[door].tiley); // JAB
    }

    doorobjlist[door].action = dr_closing;
    //
    // make the door space solid
    //
    actorat[tilex][tiley] = (objtype *)(uintptr_t)(door | 0x80);
}

/*
=====================
=
= OperateDoor
=
= The player wants to change the door's direction
=
=====================
*/

void OperateDoor(int door)
{
    int lock;

    lock = doorobjlist[door].lock;
    if (lock >= dr_lock1 && lock <= dr_lock4)
    {
        if (!(gamestate.keys & (1 << (lock - dr_lock1))))
        {
            SD_PlaySound(NOWAYSND); // locked
            return;
        }
    }

    switch (doorobjlist[door].action)
    {
    case dr_closed:
    case dr_closing:
        OpenDoor(door);
        break;
    case dr_open:
    case dr_opening:
        CloseDoor(door);
        break;
    }
}

//===========================================================================

/*
===============
=
= DoorOpen
=
= Close the door after three seconds
=
===============
*/

void DoorOpen(int door)
{
    if ((doorobjlist[door].ticcount += (short)tics) >= OPENTICS)
        CloseDoor(door);
}

/*
===============
=
= DoorOpening
=
===============
*/

void DoorOpening(int door)
{
    unsigned area1, area2;
    word *map;
    int32_t position;

    position = doorposition[door];
    if (!position)
    {
        //
        // door is just starting to open, so connect the areas
        //
        map = mapsegs[0] + (doorobjlist[door].tiley << mapshift) + doorobjlist[door].tilex;

        if (doorobjlist[door].vertical)
        {
            area1 = *(map + 1);
            area2 = *(map - 1);
        }
        else
        {
            area1 = *(map - mapwidth);
            area2 = *(map + mapwidth);
        }
        area1 -= AREATILE;
        area2 -= AREATILE;

        if (area1 < NUMAREAS && area2 < NUMAREAS)
        {
            areaconnect[area1][area2]++;
            areaconnect[area2][area1]++;

            if (player->areanumber < NUMAREAS)
                ConnectAreas();

            if (areabyplayer[area1])
                PlaySoundLocTile(OPENDOORSND, doorobjlist[door].tilex,
                                 doorobjlist[door].tiley); // JAB
        }
    }

    //
    // slide the door by an adaptive amount
    //
    position += tics << 10;
    if (position >= 0xffff)
    {
        //
        // door is all the way open
        //
        position = 0xffff;
        doorobjlist[door].ticcount = 0;
        doorobjlist[door].action = dr_open;
        actorat[doorobjlist[door].tilex][doorobjlist[door].tiley] = 0;
    }

    doorposition[door] = (word)position;
}

/*
===============
=
= DoorClosing
=
===============
*/

void DoorClosing(int door)
{
    unsigned area1, area2;
    word *map;
    int32_t position;
    int tilex, tiley;

    tilex = doorobjlist[door].tilex;
    tiley = doorobjlist[door].tiley;

    if (((int)(uintptr_t)actorat[tilex][tiley] != (door | 0x80)) || (player->tilex == tilex && player->tiley == tiley))
    { // something got inside the door
        OpenDoor(door);
        return;
    };

    position = doorposition[door];

    //
    // slide the door by an adaptive amount
    //
    position -= tics << 10;
    if (position <= 0)
    {
        //
        // door is closed all the way, so disconnect the areas
        //
        position = 0;

        doorobjlist[door].action = dr_closed;

        map = mapsegs[0] + (doorobjlist[door].tiley << mapshift) + doorobjlist[door].tilex;

        if (doorobjlist[door].vertical)
        {
            area1 = *(map + 1);
            area2 = *(map - 1);
        }
        else
        {
            area1 = *(map - mapwidth);
            area2 = *(map + mapwidth);
        }
        area1 -= AREATILE;
        area2 -= AREATILE;

        if (area1 < NUMAREAS && area2 < NUMAREAS)
        {
            areaconnect[area1][area2]--;
            areaconnect[area2][area1]--;

            if (player->areanumber < NUMAREAS)
                ConnectAreas();
        }
    }

    doorposition[door] = (word)position;
}

/*
=====================
=
= MoveDoors
=
= Called from PlayLoop
=
=====================
*/

void MoveDoors(void)
{
    int door;

    if (gamestate.victoryflag) // don't move door during victory sequence
        return;

    for (door = 0; door < doornum; door++)
    {
        switch (doorobjlist[door].action)
        {
        case dr_open:
            DoorOpen(door);
            break;

        case dr_opening:
            DoorOpening(door);
            break;

        case dr_closing:
            DoorClosing(door);
            break;
        }
    }
}

/*
=============================================================================

                                PUSHABLE WALLS

=============================================================================
*/

word pwallstate;
word pwallpos; // amount a pushable wall has been moved (0-63)
word pwallx, pwally;
byte pwalldir, pwalltile;
int dirs[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

/*
===============
=
= PushWall
=
===============
*/

void PushWall(int checkx, int checky, int dir)
{
    int oldtile, dx, dy;

    if (pwallstate)
        return;

    oldtile = tilemap[checkx][checky];
    if (!oldtile)
        return;

    dx = dirs[dir][0];
    dy = dirs[dir][1];

    if (actorat[checkx + dx][checky + dy])
    {
        SD_PlaySound(NOWAYSND);
        return;
    }
    actorat[checkx + dx][checky + dy] = (objtype *)(uintptr_t)(tilemap[checkx + dx][checky + dy] = oldtile);

    gamestate.secretcount++;
    pwallx = checkx;
    pwally = checky;
    pwalldir = dir;
    pwallstate = 1;
    pwallpos = 0;
    pwalltile = tilemap[pwallx][pwally];
    tilemap[pwallx][pwally] = 64;
    tilemap[pwallx + dx][pwally + dy] = 64;
    *(mapsegs[1] + (pwally << mapshift) + pwallx) = 0; // remove P tile info
    *(mapsegs[0] + (pwally << mapshift) + pwallx) =
        *(mapsegs[0] + (player->tiley << mapshift) + player->tilex); // set correct floorcode (BrotherTank's fix)

    SD_PlaySound(PUSHWALLSND);
}

/*
=================
=
= MovePWalls
=
=================
*/

void MovePWalls(void)
{
    int oldblock, oldtile;

    if (!pwallstate)
        return;

    oldblock = pwallstate / 128;

    pwallstate += (word)tics;

    if (pwallstate / 128 != oldblock)
    {
        // block crossed into a new block
        oldtile = pwalltile;

        //
        // the tile can now be walked into
        //
        tilemap[pwallx][pwally] = 0;
        actorat[pwallx][pwally] = 0;
        *(mapsegs[0] + (pwally << mapshift) + pwallx) = player->areanumber + AREATILE;

        int dx = dirs[pwalldir][0], dy = dirs[pwalldir][1];
        //
        // see if it should be pushed farther
        //
        if (pwallstate >= 256) // only move two tiles fix
        {
            //
            // the block has been pushed two tiles
            //
            pwallstate = 0;
            tilemap[pwallx + dx][pwally + dy] = oldtile;
            return;
        }
        else
        {
            int xl, yl, xh, yh;
            xl = (player->x - PLAYERSIZE) >> TILESHIFT;
            yl = (player->y - PLAYERSIZE) >> TILESHIFT;
            xh = (player->x + PLAYERSIZE) >> TILESHIFT;
            yh = (player->y + PLAYERSIZE) >> TILESHIFT;

            pwallx += dx;
            pwally += dy;

            if (actorat[pwallx + dx][pwally + dy] ||
                xl <= pwallx + dx && pwallx + dx <= xh && yl <= pwally + dy && pwally + dy <= yh)
            {
                pwallstate = 0;
                tilemap[pwallx][pwally] = oldtile;
                return;
            }
            actorat[pwallx + dx][pwally + dy] = (objtype *)(uintptr_t)(tilemap[pwallx + dx][pwally + dy] = oldtile);
            tilemap[pwallx + dx][pwally + dy] = 64;
        }
    }

    pwallpos = (pwallstate / 2) & 63;
}
