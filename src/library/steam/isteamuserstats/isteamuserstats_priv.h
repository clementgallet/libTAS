//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMUSERSTATS_PRIV_H_INCL
#define LIBTAS_ISTEAMUSERSTATS_PRIV_H_INCL

#include "steam/steamtypes.h"

#include <stdint.h>
#include <string>

namespace libtas {

bool ISteamUserStats_RequestCurrentStats();

// Data accessors
bool ISteamUserStats_GetStatInt32( const char *pchName, int *pData );
bool ISteamUserStats_GetStatFloat( const char *pchName, float *pData );

// Set / update data
bool ISteamUserStats_SetStatInt32( const char *pchName, int nData );
bool ISteamUserStats_SetStatFloat( const char *pchName, float fData );
bool ISteamUserStats_UpdateAvgRateStat( const char *pchName, float flCountThisSession, double dSessionLength );

// Achievement flag accessors
bool ISteamUserStats_GetAchievement( const char *pchName, bool *pbAchieved );
bool ISteamUserStats_SetAchievement( const char *pchName );
bool ISteamUserStats_ClearAchievement( const char *pchName );

// Get the achievement status, and the time it was unlocked if unlocked.
// If the return value is true, but the unlock time is zero, that means it was unlocked before Steam
// began tracking achievement unlock times (December 2009). Time is seconds since January 1, 1970.
bool ISteamUserStats_GetAchievementAndUnlockTime( const char *pchName, bool *pbAchieved, unsigned int *punUnlockTime );

// Store the current data on the server, will get a callback when set
// And one callback for every new achievement
//
// If the callback has a result of k_EResultInvalidParam, one or more stats
// uploaded has been rejected, either because they broke constraints
// or were out of date. In this case the server sends back updated values.
// The stats should be re-iterated to keep in sync.
bool ISteamUserStats_StoreStats();

// Achievement / GroupAchievement metadata

// Gets the icon of the achievement, which is a handle to be used in ISteamUtils::GetImageRGBA(), or 0 if none set.
// A return value of 0 may indicate we are still fetching data, and you can wait for the UserAchievementIconFetched_t callback
// which will notify you when the bits are ready. If the callback still returns zero, then there is no image set for the
// specified achievement.
int ISteamUserStats_GetAchievementIcon( const char *pchName );

// Get general attributes for an achievement. Accepts the following keys:
// - "name" and "desc" for retrieving the localized achievement name and description (returned in UTF8)
// - "hidden" for retrieving if an achievement is hidden (returns "0" when not hidden, "1" when hidden)
const char *ISteamUserStats_GetAchievementDisplayAttribute( const char *pchName, const char *pchKey );

// Achievement progress - triggers an AchievementProgress callback, that is all.
// Calling this w/ N out of N progress will NOT set the achievement, the game must still do that.
bool ISteamUserStats_IndicateAchievementProgress( const char *pchName, unsigned int nCurProgress, unsigned int nMaxProgress );

// Used for iterating achievements. In general games should not need these functions because they should have a
// list of existing achievements compiled into them
unsigned int ISteamUserStats_GetNumAchievements();
// Get achievement name iAchievement in [0,GetNumAchievements)
const char *ISteamUserStats_GetAchievementName( unsigned int iAchievement );

// Friends stats & achievements

// downloads stats for the user
// returns a UserStatsReceived_t received when completed
// if the other user has no stats, UserStatsReceived_t.m_eResult will be set to k_EResultFail
// these stats won't be auto-updated; you'll need to call RequestUserStats() again to refresh any data
SteamAPICall_t ISteamUserStats_RequestUserStats( CSteamID steamIDUser );

// requests stat information for a user, usable after a successful call to RequestUserStats()
bool ISteamUserStats_GetUserStatInt32( CSteamID steamIDUser, const char *pchName, int *pData );
bool ISteamUserStats_GetUserStatFloat( CSteamID steamIDUser, const char *pchName, float *pData );
bool ISteamUserStats_GetUserAchievement( CSteamID steamIDUser, const char *pchName, bool *pbAchieved );
// See notes for GetAchievementAndUnlockTime above
bool ISteamUserStats_GetUserAchievementAndUnlockTime( CSteamID steamIDUser, const char *pchName, bool *pbAchieved, unsigned int *punUnlockTime );

// Reset stats
bool ISteamUserStats_ResetAllStats( bool bAchievementsToo );

// Leaderboard functions

// asks the Steam back-end for a leaderboard by name, and will create it if it's not yet
// This call is asynchronous, with the result returned in LeaderboardFindResult_t
SteamAPICall_t ISteamUserStats_FindOrCreateLeaderboard( const char *pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType );

// as above, but won't create the leaderboard if it's not found
// This call is asynchronous, with the result returned in LeaderboardFindResult_t
SteamAPICall_t ISteamUserStats_FindLeaderboard( const char *pchLeaderboardName );

// returns the name of a leaderboard
const char *ISteamUserStats_GetLeaderboardName( SteamLeaderboard_t hSteamLeaderboard );

// returns the total number of entries in a leaderboard, as of the last request
int ISteamUserStats_GetLeaderboardEntryCount( SteamLeaderboard_t hSteamLeaderboard );

// returns the sort method of the leaderboard
ELeaderboardSortMethod ISteamUserStats_GetLeaderboardSortMethod( SteamLeaderboard_t hSteamLeaderboard );

// returns the display type of the leaderboard
ELeaderboardDisplayType ISteamUserStats_GetLeaderboardDisplayType( SteamLeaderboard_t hSteamLeaderboard );

// Asks the Steam back-end for a set of rows in the leaderboard.
// This call is asynchronous, with the result returned in LeaderboardScoresDownloaded_t
// LeaderboardScoresDownloaded_t will contain a handle to pull the results from GetDownloadedLeaderboardEntries() (below)
// You can ask for more entries than exist, and it will return as many as do exist.
// k_ELeaderboardDataRequestGlobal requests rows in the leaderboard from the full table, with nRangeStart & nRangeEnd in the range [1, TotalEntries]
// k_ELeaderboardDataRequestGlobalAroundUser requests rows around the current user, nRangeStart being negate
//   e.g. DownloadLeaderboardEntries( hLeaderboard, k_ELeaderboardDataRequestGlobalAroundUser, -3, 3 ) will return 7 rows, 3 before the user, 3 after
// k_ELeaderboardDataRequestFriends requests all the rows for friends of the current user
SteamAPICall_t ISteamUserStats_DownloadLeaderboardEntries( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd );
// as above, but downloads leaderboard entries for an arbitrary set of users - ELeaderboardDataRequest is k_ELeaderboardDataRequestUsers
// if a user doesn't have a leaderboard entry, they won't be included in the result
// a max of 100 users can be downloaded at a time, with only one outstanding call at a time
SteamAPICall_t ISteamUserStats_DownloadLeaderboardEntriesForUsers( SteamLeaderboard_t hSteamLeaderboard, CSteamID *prgUsers, int cUsers );

// Returns data about a single leaderboard entry
// use a for loop from 0 to LeaderboardScoresDownloaded_t::m_cEntryCount to get all the downloaded entries
// e.g.
//		void OnLeaderboardScoresDownloaded( LeaderboardScoresDownloaded_t *pLeaderboardScoresDownloaded )
//		{
//			for ( int index; index < pLeaderboardScoresDownloaded->m_cEntryCount; index++ )
//			{
//				LeaderboardEntry_t leaderboardEntry;
//				int details[3];		// we know this is how many we've stored previously
//				GetDownloadedLeaderboardEntry( pLeaderboardScoresDownloaded->m_hSteamLeaderboardEntries, index, &leaderboardEntry, details, 3 );
//				assert( leaderboardEntry.m_cDetails == 3 );
//				...
//			}
// once you've accessed all the entries, the data will be free'd, and the SteamLeaderboardEntries_t handle will become invalid
bool ISteamUserStats_GetDownloadedLeaderboardEntry( SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry_t *pLeaderboardEntry, int *pDetails, int cDetailsMax );

// Uploads a user score to the Steam back-end.
// This call is asynchronous, with the result returned in LeaderboardScoreUploaded_t
// Details are extra game-defined information regarding how the user got that score
// pScoreDetails points to an array of int's, cScoreDetailsCount is the number of int's in the list
SteamAPICall_t ISteamUserStats_UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardUploadScoreMethod eLeaderboardUploadScoreMethod, int nScore, const int *pScoreDetails, int cScoreDetailsCount );

// Attaches a piece of user generated content the user's entry on a leaderboard.
// hContent is a handle to a piece of user generated content that was shared using ISteamUserRemoteStorage::FileShare().
// This call is asynchronous, with the result returned in LeaderboardUGCSet_t.
SteamAPICall_t ISteamUserStats_AttachLeaderboardUGC( SteamLeaderboard_t hSteamLeaderboard, UGCHandle_t hUGC );

// Retrieves the number of players currently playing your game (online + offline)
// This call is asynchronous, with the result returned in NumberOfCurrentPlayers_t
SteamAPICall_t ISteamUserStats_GetNumberOfCurrentPlayers();

// Requests that Steam fetch data on the percentage of players who have received each achievement
// for the game globally.
// This call is asynchronous, with the result returned in GlobalAchievementPercentagesReady_t.
SteamAPICall_t ISteamUserStats_RequestGlobalAchievementPercentages();

// Get the info on the most achieved achievement for the game, returns an iterator index you can use to fetch
// the next most achieved afterwards.  Will return -1 if there is no data on achievement
// percentages (ie, you haven't called RequestGlobalAchievementPercentages and waited on the callback).
int ISteamUserStats_GetMostAchievedAchievementInfo( char *pchName, unsigned int unNameBufLen, float *pflPercent, bool *pbAchieved );

// Get the info on the next most achieved achievement for the game. Call this after GetMostAchievedAchievementInfo or another
// GetNextMostAchievedAchievementInfo call passing the iterator from the previous call. Returns -1 after the last
// achievement has been iterated.
int ISteamUserStats_GetNextMostAchievedAchievementInfo( int iIteratorPrevious, char *pchName, unsigned int unNameBufLen, float *pflPercent, bool *pbAchieved );

// Returns the percentage of users who have achieved the specified achievement.
bool ISteamUserStats_GetAchievementAchievedPercent( const char *pchName, float *pflPercent );

// Requests global stats data, which is available for stats marked as "aggregated".
// This call is asynchronous, with the results returned in GlobalStatsReceived_t.
// nHistoryDays specifies how many days of day-by-day history to retrieve in addition
// to the overall totals. The limit is 60.
SteamAPICall_t ISteamUserStats_RequestGlobalStats( int nHistoryDays );

// Gets the lifetime totals for an aggregated stat
bool ISteamUserStats_GetGlobalStatInt64( const char *pchStatName, int64_t *pData );
bool ISteamUserStats_GetGlobalStatDouble( const char *pchStatName, double *pData );

// Gets history for an aggregated stat. pData will be filled with daily values, starting with today.
// So when called, pData[0] will be today, pData[1] will be yesterday, and pData[2] will be two days ago,
// etc. cubData is the size in bytes of the pubData buffer. Returns the number of
// elements actually set.
int ISteamUserStats_GetGlobalStatHistoryInt64( const char *pchStatName, int64_t *pData, unsigned int cubData );
int ISteamUserStats_GetGlobalStatHistoryDouble( const char *pchStatName, double *pData, unsigned int cubData );

// For achievements that have related Progress stats, use this to query what the bounds of that progress are.
// You may want this info to selectively call IndicateAchievementProgress when appropriate milestones of progress
// have been made, to show a progress notification to the user.
bool ISteamUserStats_GetAchievementProgressLimitsInt32( const char *pchName, int32_t *pnMinProgress, int32_t *pnMaxProgress );
bool ISteamUserStats_GetAchievementProgressLimitsFloat( const char *pchName, float *pfMinProgress, float *pfMaxProgress );

}

#endif
