//----------------------------------------------------------
//
//  CheckPoint Library
//  Dependencies: samp-stdlib, pawn-list, samp-streamer-plugin
//  Copyright (c) 2021 by Veseon.
//
//----------------------------------------------------------

#include <a_samp>
#include <list>
#include <streamer>

//----------------------------------------------------------

const CheckPoint:INVALID_CHECK_POINT = CheckPoint:0;

#if !defined DEFAULT_CHECK_POINT_RADIUS
    const Float:DEFAULT_CHECK_POINT_RADIUS = 10.0;
#endif

#if !defined CheckPointObject
    enum CheckPointObject
    {
        Barrier = 1237
    };
#endif

#if !defined CheckPointMapIcon
    enum CheckPointMapIcon
    {
        CheckPoint = 0,
        Flag = 19,
        RaceCup = 33,
        WayPoint = 41,
        Truck = 51,
        RaceFlag = 53,
        Car = 55,
        Light = 56
    };
#endif

static cp@CheckPointExtraId;
static List:cp@PlayerCheckPointList[MAX_PLAYERS];

//----------------------------------------------------------

static stock cp@CheckPoint_SetExtraId(STREAMER_TAG_AREA:area, const STREAMER_TAG_OBJECT:object[2], STREAMER_TAG_MAP_ICON:mapicon)
{
    new data[3];
    data[0] = _:object[0];
    data[1] = _:object[1];
    data[2] = _:mapicon;
    Streamer_SetArrayData(STREAMER_TYPE_AREA, area, cp@CheckPointExtraId, data, sizeof data);
}

//----------------------------------------------------------

static stock cp@CheckPoint_GetExtraId(STREAMER_TAG_AREA:area, STREAMER_TAG_OBJECT:object[2], &STREAMER_TAG_MAP_ICON:mapicon)
{
    new data[3];
    Streamer_GetArrayData(STREAMER_TYPE_AREA, area, cp@CheckPointExtraId, data, sizeof data);
    object[0] = STREAMER_TAG_OBJECT:data[0];
    object[1] = STREAMER_TAG_OBJECT:data[1];
    mapicon = STREAMER_TAG_MAP_ICON:data[2];
}

//----------------------------------------------------------

stock bool:CheckPoint_IsValid(CheckPoint:checkpoint)
{
    if(!IsValidDynamicArea(STREAMER_TAG_AREA:checkpoint))
        return false;

    if(Streamer_IncludeFileVersion > 0x295)
    {
        return bool:Streamer_HasArrayData(STREAMER_TYPE_AREA, STREAMER_TAG_AREA:checkpoint, cp@CheckPointExtraId);
    }

    return bool:Streamer_GetArrayDataLength(STREAMER_TYPE_AREA, STREAMER_TAG_AREA:checkpoint, cp@CheckPointExtraId);
}

//----------------------------------------------------------

stock bool:CheckPoint_IsShownForPlayer(playerid, CheckPoint:checkpoint)
{
    if(!IsValidDynamicArea(STREAMER_TAG_AREA:checkpoint))
        return false;

    if(!Streamer_IsInArrayData(STREAMER_TYPE_AREA, STREAMER_TAG_AREA:checkpoint, E_STREAMER_PLAYER_ID, playerid))
        return false;

    return true;
}

//----------------------------------------------------------

stock CheckPoint:CheckPoint_Create(CheckPointObject:object, CheckPointMapIcon:mapicon, Float:minx, Float:miny, Float:maxx, Float:maxy, Float:z, Float:radius = DEFAULT_CHECK_POINT_RADIUS)
{
    new STREAMER_TAG_AREA:area, STREAMER_TAG_OBJECT:objectId[2], STREAMER_TAG_MAP_ICON:mapIcon, Float:minX = minx, Float:minY = miny, Float:maxX = maxx, Float:maxY = maxy;
    if((maxx - minx) > (maxy - miny))
    {
        maxY += (radius / 2);
        minY -= (radius / 2);
    }
    else
    {
        maxX += (radius / 2);
        minX -= (radius / 2);
    }
    area = CreateDynamicRectangle(minX, minY, maxX, maxY, .playerid = INVALID_PLAYER_ID, .priority = 1);
    objectId[0] = CreateDynamicObject(_:object, minx, miny, z, 0.0, 0.0, 0.0, .playerid = INVALID_PLAYER_ID);
    objectId[1] = CreateDynamicObject(_:object, maxx, maxy, z, 0.0, 0.0, 0.0, .playerid = INVALID_PLAYER_ID);
    mapIcon = CreateDynamicMapIcon((minx + maxx) / 2.0, (miny + maxy) / 2.0, z, _:mapicon, (mapicon == CheckPoint ? 0xFF0000FF : -1), .playerid = INVALID_PLAYER_ID);
    cp@CheckPoint_SetExtraId(area, objectId, mapIcon);
    return CheckPoint:area;
}

//----------------------------------------------------------

stock bool:CheckPoint_Destroy(CheckPoint:checkpoint)
{
    if(!CheckPoint_IsValid(checkpoint))
        return false;

    new STREAMER_TAG_AREA:area = STREAMER_TAG_AREA:checkpoint, STREAMER_TAG_OBJECT:object[2], STREAMER_TAG_MAP_ICON:mapIcon, players[MAX_PLAYERS] = {INVALID_PLAYER_ID, ...};
    Streamer_GetArrayData(STREAMER_TYPE_AREA, area, E_STREAMER_PLAYER_ID, players, sizeof players);
    for(new i; i < sizeof players; i++)
    {
        if(players[i] != INVALID_PLAYER_ID)
        {
            LIST_remove_val(cp@PlayerCheckPointList[i], _:checkpoint);
        }
    }
    cp@CheckPoint_GetExtraId(area, object, mapIcon);
    DestroyDynamicArea(area);
    DestroyDynamicObject(object[0]);
    DestroyDynamicObject(object[1]);
    DestroyDynamicMapIcon(mapIcon);
    return true;
}

//----------------------------------------------------------

stock bool:CheckPoint_ShowForPlayer(playerid, CheckPoint:checkpoint)
{
    if(!CheckPoint_IsValid(checkpoint))
        return false;

    if(CheckPoint_IsShownForPlayer(playerid, checkpoint))
        return false;

    new STREAMER_TAG_AREA:area = STREAMER_TAG_AREA:checkpoint, STREAMER_TAG_OBJECT:object[2], STREAMER_TAG_MAP_ICON:mapIcon;
    cp@CheckPoint_GetExtraId(area, object, mapIcon);
    Streamer_AppendArrayData(STREAMER_TYPE_AREA, area, E_STREAMER_PLAYER_ID, playerid);
    Streamer_AppendArrayData(STREAMER_TYPE_OBJECT, object[0], E_STREAMER_PLAYER_ID, playerid);
    Streamer_AppendArrayData(STREAMER_TYPE_OBJECT, object[1], E_STREAMER_PLAYER_ID, playerid);
    Streamer_AppendArrayData(STREAMER_TYPE_MAP_ICON, mapIcon, E_STREAMER_PLAYER_ID, playerid);
    LIST_push_back_val(cp@PlayerCheckPointList[playerid], _:checkpoint);
    return true;
}

//----------------------------------------------------------

static stock cp@CheckPoint_HideForPlayer(playerid, CheckPoint:checkpoint)
{
    new STREAMER_TAG_AREA:area = STREAMER_TAG_AREA:checkpoint, STREAMER_TAG_OBJECT:object[2], STREAMER_TAG_MAP_ICON:mapIcon;
    cp@CheckPoint_GetExtraId(area, object, mapIcon);
    Streamer_RemoveArrayData(STREAMER_TYPE_AREA, area, E_STREAMER_PLAYER_ID, playerid);
    Streamer_RemoveArrayData(STREAMER_TYPE_OBJECT, object[0], E_STREAMER_PLAYER_ID, playerid);
    Streamer_RemoveArrayData(STREAMER_TYPE_OBJECT, object[1], E_STREAMER_PLAYER_ID, playerid);
    Streamer_RemoveArrayData(STREAMER_TYPE_MAP_ICON, mapIcon, E_STREAMER_PLAYER_ID, playerid);
}

//----------------------------------------------------------

stock bool:CheckPoint_HideForPlayer(playerid, CheckPoint:checkpoint)
{
    if(!CheckPoint_IsValid(checkpoint))
        return false;

    if(!CheckPoint_IsShownForPlayer(playerid, checkpoint))
        return false;

    cp@CheckPoint_HideForPlayer(playerid, checkpoint);
    LIST_remove_val(cp@PlayerCheckPointList[playerid], _:checkpoint);
    return true;
}

//----------------------------------------------------------

#if defined CheckPoint_OnPlayerEnter
    forward CheckPoint_OnPlayerEnter(playerid, CheckPoint:checkpoint);
    public OnPlayerEnterDynamicArea(playerid, STREAMER_TAG_AREA:areaid)
    {
        if(CheckPoint_IsValid(CheckPoint:areaid))
        {
            CheckPoint_OnPlayerEnter(playerid, CheckPoint:areaid);
            return 1;
        }
        #if defined cp@OnPlayerEnterDynArea
            return cp@OnPlayerEnterDynArea(playerid, areaid);
        #else
            return 1;
        #endif
    }
    #if defined _ALS_OnPlayerEnterDynArea
        #undef OnPlayerEnterDynamicArea
    #else
        #define _ALS_OnPlayerEnterDynArea
    #endif
    #define OnPlayerEnterDynamicArea cp@OnPlayerEnterDynArea
    #if defined cp@OnPlayerEnterDynArea
        forward cp@OnPlayerEnterDynArea(playerid, STREAMER_TAG_AREA:areaid);
    #endif
#endif

//----------------------------------------------------------

public OnPlayerDisconnect(playerid, reason)
{
    LIST_foreach(checkpoint : cp@PlayerCheckPointList[playerid])
    {
        cp@CheckPoint_HideForPlayer(playerid, CheckPoint:MEM_get_val(checkpoint, _));
    }
    LIST_clear(cp@PlayerCheckPointList[playerid]);
    #if defined cp@OnPlayerDisconnect
        return cp@OnPlayerDisconnect(playerid, reason);
    #else
        return 1;
    #endif
}
#if defined _ALS_OnPlayerDisconnect
    #undef OnPlayerDisconnect
#else
    #define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect cp@OnPlayerDisconnect
#if defined cp@OnPlayerDisconnect
    forward cp@OnPlayerDisconnect(playerid, reason);
#endif

//----------------------------------------------------------

#if defined FILTERSCRIPT
    public OnFilterScriptInit()
    {
        // Generate unique (more like random?) identifier for streamer extra id.
        cp@CheckPointExtraId = E_STREAMER_CUSTOM(random(2^31 - 1));

        #if defined cp@OnFilterScriptInit
            return cp@OnFilterScriptInit();
        #else
            return 1;
        #endif
    }
    #if defined _ALS_OnFilterScriptInit
        #undef OnFilterScriptInit
    #else
        #define _ALS_OnFilterScriptInit
    #endif
    #define OnFilterScriptInit cp@OnFilterScriptInit
    #if defined cp@OnFilterScriptInit
        forward cp@OnFilterScriptInit();
    #endif
#else
    public OnGameModeInit()
    {
        // Generate unique (more like random?) identifier for streamer extra id.
        cp@CheckPointExtraId = E_STREAMER_CUSTOM(random(2^31 - 1));

        #if defined cp@OnGameModeInit
            return cp@OnGameModeInit();
        #else
            return 1;
        #endif
    }
    #if defined _ALS_OnGameModeInit
        #undef OnGameModeInit
    #else
        #define _ALS_OnGameModeInit
    #endif
    #define OnGameModeInit cp@OnGameModeInit
    #if defined cp@OnGameModeInit
        forward cp@OnGameModeInit();
    #endif
#endif

//----------------------------------------------------------
// EOF