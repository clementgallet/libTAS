#include "steam.h"
#include "logging.h"

bool SteamAPI_Init()
{
    debuglog(LCF_STEAM, "%s is called.", __func__);
	return true;
}

void SteamAPI_Shutdown()
{
    debuglog(LCF_STEAM, "%s is called.", __func__);
    return;
}

bool SteamAPI_IsSteamRunning()
{
    debuglog(LCF_STEAM, "%s is called.", __func__);
	return false;
}

bool SteamAPI_RestartAppIfNecessary( unsigned int unOwnAppID )
{
    debuglog(LCF_STEAM, "%s is called.", __func__);
	return false;
}

void SteamAPI_RunCallbacks()
{
    debuglog(LCF_STEAM, "%s is called.", __func__);
}

void SteamAPI_RegisterCallback( void *pCallback, int iCallback )
{
    debuglog(LCF_STEAM, "%s is called.", __func__);
}

void SteamAPI_UnregisterCallback( void *pCallback )
{
    debuglog(LCF_STEAM, "%s is called.", __func__);
}

