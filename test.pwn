#include <a_samp>

#define STREAMER_ENABLE_TAGS

#include "checkpoint.inc"

enum E_CHECKPOINT
{
    CheckPoint:CheckPointId,
    CheckPointObject:Object,
    CheckPointMapIcon:MapIcon,
    Float:MinX,
    Float:MinY,
    Float:MaxX,
    Float:MaxY,
    Float:Z
};
new CheckPointData[][E_CHECKPOINT] =
{
    {INVALID_CHECK_POINT,   Barrier,    CheckPoint,     144.3318,    66.9802,   150.8382,   79.4799,    1.29575},
    {INVALID_CHECK_POINT,   Barrier,    CheckPoint,     191.1907,    49.2438,   195.2143,   63.2610,    1.3607},
    {INVALID_CHECK_POINT,   Barrier,    CheckPoint,     225.0351,    40.1107,   240.2447,   40.3515,    1.4787},
    {INVALID_CHECK_POINT,   Barrier,    CheckPoint,     224.4973,   -15.1673,   238.9789,   -13.6275,   0.5},
    {INVALID_CHECK_POINT,   Barrier,    CheckPoint,     225.2619,   -64.6838,   240.8934,   -63.9234,   0.5},
    {INVALID_CHECK_POINT,   Barrier,    CheckPoint,     225.1828,   -152.9203,  225.3761,   -136.5711,  0.5},
    {INVALID_CHECK_POINT,   Barrier,    RaceFlag,       193.3322,   -150.8401,  193.5930,   -139.1640,  0.5}
};

main(){}

public OnGameModeInit()
{
    AddPlayerClass(299, 82.7588, 109.0594, 2.0859, 239.3224, 0, 0, 0, 0, 0, 0);

    CheckPoint_IsValid(CheckPoint:1);

    for(new i; i < sizeof CheckPointData; i++)
    {
        CheckPointData[i][CheckPointId] = CheckPoint_Create(CheckPointData[i][Object], CheckPointData[i][MapIcon], CheckPointData[i][MinX], CheckPointData[i][MinY], CheckPointData[i][MaxX], CheckPointData[i][MaxY], CheckPointData[i][Z]);
    }
}

public OnGameModeExit()
{
    for(new i; i < sizeof CheckPointData; i++)
    {
        CheckPoint_Destroy(CheckPointData[i][CheckPointId]);
    }
}

public OnPlayerSpawn(playerid)
{
    new Float:x, Float:y, Float:z, Float:angle;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, angle);

    new car = CreateVehicle(560, x, y, z + 2.5, angle, -1, -1, -1, 0);
    AddVehicleComponent(car, 1010);
    PutPlayerInVehicle(playerid, car, 0);

    for(new i; i < sizeof CheckPointData; i++)
    {
        //CheckPoint_ShowForPlayer(playerid, CheckPointData[i][CheckPointId]);
    }
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    if(strcmp("/car", cmdtext, false, 5) == 0)
    {
        new Float:x, Float:y, Float:z, Float:angle;
        GetPlayerPos(playerid, x, y, z);
        GetPlayerFacingAngle(playerid, angle);

        new car = CreateVehicle(560, x, y, z + 2.5, angle, -1, -1, -1, 0);
        AddVehicleComponent(car, 1010);
        PutPlayerInVehicle(playerid, car, 0);
        return 1;
    }
    if(strcmp("/show", cmdtext, false, 6) == 0)
    {
        for(new i; i < sizeof CheckPointData; i++)
        {
            CheckPoint_ShowForPlayer(playerid, CheckPointData[i][CheckPointId]);
        }
        return 1;
    }
    if(strcmp("/hide", cmdtext, false, 6) == 0)
    {
        for(new i; i < sizeof CheckPointData; i++)
        {
            CheckPoint_HideForPlayer(playerid, CheckPointData[i][CheckPointId]);
        }
        return 1;
    }
    if(strcmp("/create", cmdtext, false, 8) == 0)
    {
        for(new i; i < sizeof CheckPointData; i++)
        {
            if(CheckPointData[i][CheckPointId] == INVALID_CHECK_POINT)
            {
                CheckPointData[i][CheckPointId] = CheckPoint_Create(CheckPointData[i][Object], CheckPointData[i][MapIcon], CheckPointData[i][MinX], CheckPointData[i][MinY], CheckPointData[i][MaxX], CheckPointData[i][MaxY], CheckPointData[i][Z]);
                CheckPoint_ShowForPlayer(playerid, CheckPointData[i][CheckPointId]);
            }
        }
        return 1;
    }
    if(strcmp("/destroy", cmdtext, false, 9) == 0)
    {
        for(new i; i < sizeof CheckPointData; i++)
        {
            CheckPoint_Destroy(CheckPointData[i][CheckPointId]);
            CheckPointData[i][CheckPointId] = INVALID_CHECK_POINT;
        }
        return 1;
    }
    return 0;
}

public CheckPoint_OnPlayerEnter(playerid, CheckPoint:checkpoint)
{
    static const sounds[] = {1137, 1138, 1139};
    PlayerPlaySound(playerid, sounds[random(sizeof sounds)], 0.0, 0.0, 0.0);

    new message[144];
    format(message, sizeof message, "You have entered checkpoint id: %d", _:checkpoint);
    SendClientMessage(playerid, -1, message);

    CheckPoint_HideForPlayer(playerid, checkpoint);
}