/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "isteamhttp.h"
#include "../logging.h"

namespace libtas {

HTTPRequestHandle ISteamHTTP::CreateHTTPRequest( EHTTPMethod eHTTPRequestMethod, const char *pchAbsoluteURL )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

bool ISteamHTTP::SetHTTPRequestContextValue( HTTPRequestHandle hRequest, uint64_t ulContextValue )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamHTTP::SetHTTPRequestNetworkActivityTimeout( HTTPRequestHandle hRequest, uint32_t unTimeoutSeconds )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamHTTP::SetHTTPRequestHeaderValue( HTTPRequestHandle hRequest, const char *pchHeaderName, const char *pchHeaderValue )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamHTTP::SetHTTPRequestGetOrPostParameter( HTTPRequestHandle hRequest, const char *pchParamName, const char *pchParamValue )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamHTTP::SendHTTPRequest( HTTPRequestHandle hRequest, SteamAPICall_t *pCallHandle )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamHTTP::SendHTTPRequestAndStreamResponse( HTTPRequestHandle hRequest, SteamAPICall_t *pCallHandle )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamHTTP::DeferHTTPRequest( HTTPRequestHandle hRequest )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamHTTP::PrioritizeHTTPRequest( HTTPRequestHandle hRequest )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamHTTP::GetHTTPResponseHeaderSize( HTTPRequestHandle hRequest, const char *pchHeaderName, uint32_t *unResponseHeaderSize )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamHTTP::GetHTTPResponseHeaderValue( HTTPRequestHandle hRequest, const char *pchHeaderName, uint8_t *pHeaderValueBuffer, uint32_t unBufferSize )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamHTTP::GetHTTPResponseBodySize( HTTPRequestHandle hRequest, uint32_t *unBodySize )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamHTTP::GetHTTPResponseBodyData( HTTPRequestHandle hRequest, uint8_t *pBodyDataBuffer, uint32_t unBufferSize )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamHTTP::GetHTTPStreamingResponseBodyData( HTTPRequestHandle hRequest, uint32_t cOffset, uint8_t *pBodyDataBuffer, uint32_t unBufferSize )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamHTTP::ReleaseHTTPRequest( HTTPRequestHandle hRequest )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamHTTP::GetHTTPDownloadProgressPct( HTTPRequestHandle hRequest, float *pflPercentOut )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamHTTP::SetHTTPRequestRawPostBody( HTTPRequestHandle hRequest, const char *pchContentType, uint8_t *pubBody, uint32_t unBodyLen )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

HTTPCookieContainerHandle ISteamHTTP::CreateCookieContainer( bool bAllowResponsesToModify )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

bool ISteamHTTP::ReleaseCookieContainer( HTTPCookieContainerHandle hCookieContainer )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamHTTP::SetCookie( HTTPCookieContainerHandle hCookieContainer, const char *pchHost, const char *pchUrl, const char *pchCookie )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamHTTP::SetHTTPRequestCookieContainer( HTTPRequestHandle hRequest, HTTPCookieContainerHandle hCookieContainer )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamHTTP::SetHTTPRequestUserAgentInfo( HTTPRequestHandle hRequest, const char *pchUserAgentInfo )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamHTTP::SetHTTPRequestRequiresVerifiedCertificate( HTTPRequestHandle hRequest, bool bRequireVerifiedCertificate )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamHTTP::SetHTTPRequestAbsoluteTimeoutMS( HTTPRequestHandle hRequest, uint32_t unMilliseconds )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamHTTP::GetHTTPRequestWasTimedOut( HTTPRequestHandle hRequest, bool *pbWasTimedOut )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

}
