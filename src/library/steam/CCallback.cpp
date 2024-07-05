/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "CCallback.h"
#include "CCallbackManager.h"

/* CCallResult */

namespace libtas {

void CCallResult::Run(void *pvParam)
{
    Run(pvParam, false, m_hAPICall);
}

void CCallResult::Run(void *pvParam, bool bIOFailure, SteamAPICall_t hSteamAPICall)
{
    if (hSteamAPICall != m_hAPICall)
        return;

    m_hAPICall = 0;
    m_Func(m_pObj, pvParam, bIOFailure);
}

CCallResult::CCallResult(enum steam_callback_type type, size_t data_size) : CCallbackBase()
{
    m_hAPICall = 0;
    m_pObj = nullptr;
    m_Func = nullptr;
    m_iCallback = type;
    m_dataSize = data_size;
}

CCallResult::~CCallResult()
{
    Cancel();
}

void CCallResult::Set(SteamAPICall_t hAPICall, void *p, func_t func)
{
    if (m_hAPICall)
        CCallbackManager::UnregisterApiCallResult(this, m_hAPICall);

    m_hAPICall = hAPICall;
    m_pObj = p;
    m_Func = func;

    if (m_hAPICall)
        CCallbackManager::RegisterApiCallResult(this, m_hAPICall);
}

bool CCallResult::IsActive() const
{
    return !!m_hAPICall;
}

void CCallResult::Cancel()
{
    if (!IsActive())
        return;

    CCallbackManager::UnregisterApiCallResult(this, m_hAPICall);
    m_hAPICall = 0;
}

/* CCallback */

void CCallback::Run(void *pvParam)
{
    m_Func(m_pObj, pvParam);
}

CCallback::CCallback(void *pObj, func_t func, size_t data_size, bool is_game_server) : CCallbackImpl()
{
    m_pObj = pObj;
    m_Func = func;
    m_dataSize = data_size;

    Register(m_pObj, m_Func);

    if (is_game_server && (m_nCallbackFlags & k_ECallbackFlagsRegistered))
        m_nCallbackFlags |= k_ECallbackFlagsGameServer;
}

CCallback::~CCallback()
{
    Unregister();
}

void CCallback::Register(void *pObj, func_t func)
{
    if (!pObj || !func)
        return;

    if (m_nCallbackFlags & k_ECallbackFlagsRegistered)
        Unregister();

    m_pObj = pObj;
    m_Func = func;

    CCallbackManager::RegisterCallback(this, m_iCallback);
}

void CCallback::Unregister()
{
    CCallbackManager::UnregisterCallback(this);
}

}
