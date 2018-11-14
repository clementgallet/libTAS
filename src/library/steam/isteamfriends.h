//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMFRIENDS_H_INCL
#define LIBTAS_ISTEAMFRIENDS_H_INCL

#include <stdint.h>
#include "steamtypes.h"

namespace libtas {

//-----------------------------------------------------------------------------
// Purpose: interface to accessing information about individual users,
//			that can be a friend, in a group, on a game server or in a lobby with the local user
//-----------------------------------------------------------------------------
class ISteamFriends
{
public:
	// returns the local players name - guaranteed to not be NULL.
	// this is the same name as on the users community profile page
	// this is stored in UTF-8 format
	// like all the other interface functions that return a char *, it's important that this pointer is not saved
	// off; it will eventually be free'd or re-allocated
	virtual const char *GetPersonaName();

	// Sets the player name, stores it on the server and publishes the changes to all friends who are online.
	// Changes take place locally immediately, and a PersonaStateChange_t is posted, presuming success.
	//
	// The final results are available through the return value SteamAPICall_t, using SetPersonaNameResponse_t.
	//
	// If the name change fails to happen on the server, then an additional global PersonaStateChange_t will be posted
	// to change the name back, in addition to the SetPersonaNameResponse_t callback.
	virtual SteamAPICall_t SetPersonaName( const char *pchPersonaName );

	// gets the status of the current user
	virtual EPersonaState GetPersonaState();

	// friend iteration
	// takes a set of k_EFriendFlags, and returns the number of users the client knows about who meet that criteria
	// then GetFriendByIndex() can then be used to return the id's of each of those users
	virtual int GetFriendCount( int iFriendFlags );

	// returns the steamID of a user
	// iFriend is a index of range [0, GetFriendCount())
	// iFriendsFlags must be the same value as used in GetFriendCount()
	// the returned CSteamID can then be used by all the functions below to access details about the user
	virtual CSteamID GetFriendByIndex( int iFriend, int iFriendFlags );

	// returns a relationship to a user
	virtual EFriendRelationship GetFriendRelationship( CSteamID steamIDFriend );

	// returns the current status of the specified user
	// this will only be known by the local user if steamIDFriend is in their friends list; on the same game server; in a chat room or lobby; or in a small group with the local user
	virtual EPersonaState GetFriendPersonaState( CSteamID steamIDFriend );

	// returns the name another user - guaranteed to not be NULL.
	// same rules as GetFriendPersonaState() apply as to whether or not the user knowns the name of the other user
	// note that on first joining a lobby, chat room or game server the local user will not known the name of the other users automatically; that information will arrive asyncronously
	//
	virtual const char *GetFriendPersonaName( CSteamID steamIDFriend );

	// returns true if the friend is actually in a game, and fills in pFriendGameInfo with an extra details
	virtual bool GetFriendGamePlayed( CSteamID steamIDFriend, FriendGameInfo_t *pFriendGameInfo );
	// accesses old friends names - returns an empty string when their are no more items in the history
	virtual const char *GetFriendPersonaNameHistory( CSteamID steamIDFriend, int iPersonaName );
	// friends steam level
	virtual int GetFriendSteamLevel( CSteamID steamIDFriend );

	// Returns nickname the current user has set for the specified player. Returns NULL if the no nickname has been set for that player.
	virtual const char *GetPlayerNickname( CSteamID steamIDPlayer );

	// // friend grouping (tag) apis
	// // returns the number of friends groups
	// virtual int GetFriendsGroupCount();
	// // returns the friends group ID for the given index (invalid indices return k_FriendsGroupID_Invalid)
	// virtual FriendsGroupID_t GetFriendsGroupIDByIndex( int iFG );
	// // returns the name for the given friends group (NULL in the case of invalid friends group IDs)
	// virtual const char *GetFriendsGroupName( FriendsGroupID_t friendsGroupID );
	// // returns the number of members in a given friends group
	// virtual int GetFriendsGroupMembersCount( FriendsGroupID_t friendsGroupID );
	// // gets up to nMembersCount members of the given friends group, if fewer exist than requested those positions' SteamIDs will be invalid
	// virtual void GetFriendsGroupMembersList( FriendsGroupID_t friendsGroupID, OUT_ARRAY_CALL(nMembersCount, GetFriendsGroupMembersCount, friendsGroupID ) CSteamID *pOutSteamIDMembers, int nMembersCount );
	//
	// // returns true if the specified user meets any of the criteria specified in iFriendFlags
	// // iFriendFlags can be the union (binary or, |) of one or more k_EFriendFlags values
	// virtual bool HasFriend( CSteamID steamIDFriend, int iFriendFlags );
	//
	// // clan (group) iteration and access functions
	// virtual int GetClanCount();
	// virtual CSteamID GetClanByIndex( int iClan );
	// virtual const char *GetClanName( CSteamID steamIDClan );
	// virtual const char *GetClanTag( CSteamID steamIDClan );
	// // returns the most recent information we have about what's happening in a clan
	// virtual bool GetClanActivityCounts( CSteamID steamIDClan, int *pnOnline, int *pnInGame, int *pnChatting );
	// // for clans a user is a member of, they will have reasonably up-to-date information, but for others you'll have to download the info to have the latest
	// virtual SteamAPICall_t DownloadClanActivityCounts( ARRAY_COUNT(cClansToRequest) CSteamID *psteamIDClans, int cClansToRequest );
	//
	// // iterators for getting users in a chat room, lobby, game server or clan
	// // note that large clans that cannot be iterated by the local user
	// // note that the current user must be in a lobby to retrieve CSteamIDs of other users in that lobby
	// // steamIDSource can be the steamID of a group, game server, lobby or chat room
	// virtual int GetFriendCountFromSource( CSteamID steamIDSource );
	// virtual CSteamID GetFriendFromSourceByIndex( CSteamID steamIDSource, int iFriend );
	//
	// // returns true if the local user can see that steamIDUser is a member or in steamIDSource
	// virtual bool IsUserInSource( CSteamID steamIDUser, CSteamID steamIDSource );
	//
	// // User is in a game pressing the talk button (will suppress the microphone for all voice comms from the Steam friends UI)
	// virtual void SetInGameVoiceSpeaking( CSteamID steamIDUser, bool bSpeaking );
	//
	// // activates the game overlay, with an optional dialog to open
	// // valid options are "Friends", "Community", "Players", "Settings", "OfficialGameGroup", "Stats", "Achievements"
	// virtual void ActivateGameOverlay( const char *pchDialog );
	//
	// // activates game overlay to a specific place
	// // valid options are
	// //		"steamid" - opens the overlay web browser to the specified user or groups profile
	// //		"chat" - opens a chat window to the specified user, or joins the group chat
	// //		"jointrade" - opens a window to a Steam Trading session that was started with the ISteamEconomy/StartTrade Web API
	// //		"stats" - opens the overlay web browser to the specified user's stats
	// //		"achievements" - opens the overlay web browser to the specified user's achievements
	// //		"friendadd" - opens the overlay in minimal mode prompting the user to add the target user as a friend
	// //		"friendremove" - opens the overlay in minimal mode prompting the user to remove the target friend
	// //		"friendrequestaccept" - opens the overlay in minimal mode prompting the user to accept an incoming friend invite
	// //		"friendrequestignore" - opens the overlay in minimal mode prompting the user to ignore an incoming friend invite
	// virtual void ActivateGameOverlayToUser( const char *pchDialog, CSteamID steamID );
	//
	// // activates game overlay web browser directly to the specified URL
	// // full address with protocol type is required, e.g. http://www.steamgames.com/
	// virtual void ActivateGameOverlayToWebPage( const char *pchURL );
	//
	// // activates game overlay to store page for app
	// virtual void ActivateGameOverlayToStore( AppId_t nAppID, EOverlayToStoreFlag eFlag );
	//
	// // Mark a target user as 'played with'. This is a client-side only feature that requires that the calling user is
	// // in game
	// virtual void SetPlayedWith( CSteamID steamIDUserPlayedWith );
	//
	// // activates game overlay to open the invite dialog. Invitations will be sent for the provided lobby.
	// virtual void ActivateGameOverlayInviteDialog( CSteamID steamIDLobby );
	//
	// // gets the small (32x32) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
	// virtual int GetSmallFriendAvatar( CSteamID steamIDFriend );
	//
	// // gets the medium (64x64) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
	// virtual int GetMediumFriendAvatar( CSteamID steamIDFriend );
	//
	// // gets the large (184x184) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
	// // returns -1 if this image has yet to be loaded, in this case wait for a AvatarImageLoaded_t callback and then call this again
	// virtual int GetLargeFriendAvatar( CSteamID steamIDFriend );
	//
	// // requests information about a user - persona name & avatar
	// // if bRequireNameOnly is set, then the avatar of a user isn't downloaded
	// // - it's a lot slower to download avatars and churns the local cache, so if you don't need avatars, don't request them
	// // if returns true, it means that data is being requested, and a PersonaStateChanged_t callback will be posted when it's retrieved
	// // if returns false, it means that we already have all the details about that user, and functions can be called immediately
	// virtual bool RequestUserInformation( CSteamID steamIDUser, bool bRequireNameOnly );
	//
	// // requests information about a clan officer list
	// // when complete, data is returned in ClanOfficerListResponse_t call result
	// // this makes available the calls below
	// // you can only ask about clans that a user is a member of
	// // note that this won't download avatars automatically; if you get an officer,
	// // and no avatar image is available, call RequestUserInformation( steamID, false ) to download the avatar
	// CALL_RESULT( ClanOfficerListResponse_t )
	// virtual SteamAPICall_t RequestClanOfficerList( CSteamID steamIDClan );
	//
	// // iteration of clan officers - can only be done when a RequestClanOfficerList() call has completed
	//
	// // returns the steamID of the clan owner
	// virtual CSteamID GetClanOwner( CSteamID steamIDClan );
	// // returns the number of officers in a clan (including the owner)
	// virtual int GetClanOfficerCount( CSteamID steamIDClan );
	// // returns the steamID of a clan officer, by index, of range [0,GetClanOfficerCount)
	// virtual CSteamID GetClanOfficerByIndex( CSteamID steamIDClan, int iOfficer );
	// // if current user is chat restricted, he can't send or receive any text/voice chat messages.
	// // the user can't see custom avatars. But the user can be online and send/recv game invites.
	// // a chat restricted user can't add friends or join any groups.
	// virtual uint32_t GetUserRestrictions();
	//
	// // Rich Presence data is automatically shared between friends who are in the same game
	// // Each user has a set of Key/Value pairs
	// // Note the following limits: k_cchMaxRichPresenceKeys, k_cchMaxRichPresenceKeyLength, k_cchMaxRichPresenceValueLength
	// // There are two magic keys:
	// //		"status"  - a UTF-8 string that will show up in the 'view game info' dialog in the Steam friends list
	// //		"connect" - a UTF-8 string that contains the command-line for how a friend can connect to a game
	// // GetFriendRichPresence() returns an empty string "" if no value is set
	// // SetRichPresence() to a NULL or an empty string deletes the key
	// // You can iterate the current set of keys for a friend with GetFriendRichPresenceKeyCount()
	// // and GetFriendRichPresenceKeyByIndex() (typically only used for debugging)
	// virtual bool SetRichPresence( const char *pchKey, const char *pchValue );
	// virtual void ClearRichPresence();
	// virtual const char *GetFriendRichPresence( CSteamID steamIDFriend, const char *pchKey );
	// virtual int GetFriendRichPresenceKeyCount( CSteamID steamIDFriend );
	// virtual const char *GetFriendRichPresenceKeyByIndex( CSteamID steamIDFriend, int iKey );
	// // Requests rich presence for a specific user.
	// virtual void RequestFriendRichPresence( CSteamID steamIDFriend );
	//
	// // rich invite support
	// // if the target accepts the invite, the pchConnectString gets added to the command-line for launching the game
	// // if the game is already running, a GameRichPresenceJoinRequested_t callback is posted containing the connect string
	// // invites can only be sent to friends
	// virtual bool InviteUserToGame( CSteamID steamIDFriend, const char *pchConnectString );
	//
	// // recently-played-with friends iteration
	// // this iterates the entire list of users recently played with, across games
	// // GetFriendCoplayTime() returns as a unix time
	// virtual int GetCoplayFriendCount();
	// virtual CSteamID GetCoplayFriend( int iCoplayFriend );
	// virtual int GetFriendCoplayTime( CSteamID steamIDFriend );
	// virtual AppId_t GetFriendCoplayGame( CSteamID steamIDFriend );
	//
	// // chat interface for games
	// // this allows in-game access to group (clan) chats from in the game
	// // the behavior is somewhat sophisticated, because the user may or may not be already in the group chat from outside the game or in the overlay
	// // use ActivateGameOverlayToUser( "chat", steamIDClan ) to open the in-game overlay version of the chat
	// CALL_RESULT( JoinClanChatRoomCompletionResult_t )
	// virtual SteamAPICall_t JoinClanChatRoom( CSteamID steamIDClan );
	// virtual bool LeaveClanChatRoom( CSteamID steamIDClan );
	// virtual int GetClanChatMemberCount( CSteamID steamIDClan );
	// virtual CSteamID GetChatMemberByIndex( CSteamID steamIDClan, int iUser );
	// virtual bool SendClanChatMessage( CSteamID steamIDClanChat, const char *pchText );
	// virtual int GetClanChatMessage( CSteamID steamIDClanChat, int iMessage, void *prgchText, int cchTextMax, EChatEntryType *peChatEntryType, OUT_STRUCT() CSteamID *psteamidChatter );
	// virtual bool IsClanChatAdmin( CSteamID steamIDClanChat, CSteamID steamIDUser );
	//
	// // interact with the Steam (game overlay / desktop)
	// virtual bool IsClanChatWindowOpenInSteam( CSteamID steamIDClanChat );
	// virtual bool OpenClanChatWindowInSteam( CSteamID steamIDClanChat );
	// virtual bool CloseClanChatWindowInSteam( CSteamID steamIDClanChat );
	//
	// // peer-to-peer chat interception
	// // this is so you can show P2P chats inline in the game
	// virtual bool SetListenForFriendsMessages( bool bInterceptEnabled );
	// virtual bool ReplyToFriendMessage( CSteamID steamIDFriend, const char *pchMsgToSend );
	// virtual int GetFriendMessage( CSteamID steamIDFriend, int iMessageID, void *pvData, int cubData, EChatEntryType *peChatEntryType );
	//
	// // following apis
	// CALL_RESULT( FriendsGetFollowerCount_t )
	// virtual SteamAPICall_t GetFollowerCount( CSteamID steamID );
	// CALL_RESULT( FriendsIsFollowing_t )
	// virtual SteamAPICall_t IsFollowing( CSteamID steamID );
	// CALL_RESULT( FriendsEnumerateFollowingList_t )
	// virtual SteamAPICall_t EnumerateFollowingList( uint32_t unStartIndex );
	//
	// virtual bool IsClanPublic( CSteamID steamIDClan );
	// virtual bool IsClanOfficialGameGroup( CSteamID steamIDClan );
};

}

#endif
