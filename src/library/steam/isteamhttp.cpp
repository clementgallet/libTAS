/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "logging.h"

namespace libtas {

HTTPRequestHandle ISteamHTTP::CreateHTTPRequest( EHTTPMethod eHTTPRequestMethod, const char *pchAbsoluteURL )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return 0;
}

bool ISteamHTTP::SetHTTPRequestContextValue( HTTPRequestHandle hRequest, uint64_t ulContextValue )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    return false;
}

bool ISteamHTTP::SetHTTPRequestNetworkActivityTimeout( HTTPRequestHandle hRequest, uint32_t unTimeoutSeconds )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    return false;
}

bool ISteamHTTP::SetHTTPRequestHeaderValue( HTTPRequestHandle hRequest, const char *pchHeaderName, const char *pchHeaderValue )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    return false;
}

bool ISteamHTTP::SetHTTPRequestGetOrPostParameter( HTTPRequestHandle hRequest, const char *pchParamName, const char *pchParamValue )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    return false;
}

bool ISteamHTTP::SendHTTPRequest( HTTPRequestHandle hRequest, SteamAPICall_t *pCallHandle )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    return false;
}

bool ISteamHTTP::SendHTTPRequestAndStreamResponse( HTTPRequestHandle hRequest, SteamAPICall_t *pCallHandle )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    return false;
}

bool ISteamHTTP::DeferHTTPRequest( HTTPRequestHandle hRequest )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    return false;
}

bool ISteamHTTP::PrioritizeHTTPRequest( HTTPRequestHandle hRequest )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    return false;
}

bool ISteamHTTP::GetHTTPResponseHeaderSize( HTTPRequestHandle hRequest, const char *pchHeaderName, uint32_t *unResponseHeaderSize )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    return false;
}

bool ISteamHTTP::GetHTTPResponseHeaderValue( HTTPRequestHandle hRequest, const char *pchHeaderName, uint8_t *pHeaderValueBuffer, uint32_t unBufferSize )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    return false;
}

bool ISteamHTTP::GetHTTPResponseBodySize( HTTPRequestHandle hRequest, uint32_t *unBodySize )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    return false;
}

bool ISteamHTTP::GetHTTPResponseBodyData( HTTPRequestHandle hRequest, uint8_t *pBodyDataBuffer, uint32_t unBufferSize )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

bool ISteamHTTP::GetHTTPStreamingResponseBodyData( HTTPRequestHandle hRequest, uint32_t cOffset, uint8_t *pBodyDataBuffer, uint32_t unBufferSize )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

bool ISteamHTTP::ReleaseHTTPRequest( HTTPRequestHandle hRequest )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

bool ISteamHTTP::GetHTTPDownloadProgressPct( HTTPRequestHandle hRequest, float *pflPercentOut )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

bool ISteamHTTP::SetHTTPRequestRawPostBody( HTTPRequestHandle hRequest, const char *pchContentType, uint8_t *pubBody, uint32_t unBodyLen )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

HTTPCookieContainerHandle ISteamHTTP::CreateCookieContainer( bool bAllowResponsesToModify )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return 0;
}

bool ISteamHTTP::ReleaseCookieContainer( HTTPCookieContainerHandle hCookieContainer )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

bool ISteamHTTP::SetCookie( HTTPCookieContainerHandle hCookieContainer, const char *pchHost, const char *pchUrl, const char *pchCookie )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

bool ISteamHTTP::SetHTTPRequestCookieContainer( HTTPRequestHandle hRequest, HTTPCookieContainerHandle hCookieContainer )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

bool ISteamHTTP::SetHTTPRequestUserAgentInfo( HTTPRequestHandle hRequest, const char *pchUserAgentInfo )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

bool ISteamHTTP::SetHTTPRequestRequiresVerifiedCertificate( HTTPRequestHandle hRequest, bool bRequireVerifiedCertificate )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

bool ISteamHTTP::SetHTTPRequestAbsoluteTimeoutMS( HTTPRequestHandle hRequest, uint32_t unMilliseconds )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

bool ISteamHTTP::GetHTTPRequestWasTimedOut( HTTPRequestHandle hRequest, bool *pbWasTimedOut )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

}
