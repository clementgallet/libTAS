#include "steam.h"
#include "logging.h"

bool SteamAPI_Init()
{
    debuglog(LCF_STEAM, __func__, " call.");
	return true;
}

void SteamAPI_Shutdown()
{
    debuglog(LCF_STEAM, __func__, " call.");
    return;
}

bool SteamAPI_IsSteamRunning()
{
    debuglog(LCF_STEAM, __func__, " call.");
	return false;
}

bool SteamAPI_RestartAppIfNecessary( unsigned int unOwnAppID )
{
    debuglog(LCF_STEAM, __func__, " call.");
	return false;
}

void SteamAPI_RunCallbacks()
{
    debuglog(LCF_STEAM, __func__, " call.");
}

void SteamAPI_RegisterCallback( void *pCallback, int iCallback )
{
    debuglog(LCF_STEAM, __func__, " call.");
}

void SteamAPI_UnregisterCallback( void *pCallback )
{
    debuglog(LCF_STEAM, __func__, " call.");
}

