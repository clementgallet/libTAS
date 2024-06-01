
#include "CCallback.h"
#include "CCallbackManager.h"

#include "logging.h"

#include <inttypes.h>
#include <stdint.h>
#include <forward_list>
#include <mutex>

namespace libtas {

namespace CCallbackManager {

struct call_output
{
    enum steam_callback_type type;
    bool is_api_call : true;
    bool is_handled : true;
    bool io_failure : true;
    SteamAPICall_t api_call;
    void *data;
    int data_size;
};

struct api_call_result
{
    CCallbackBase *callback;
    SteamAPICall_t api_call;
};

static SteamAPICall_t last_api_call_id;

static std::forward_list<CCallbackBase*> callbacks[STEAM_CALLBACK_TYPE_MAX];
static std::recursive_mutex callback_mutex[STEAM_CALLBACK_TYPE_MAX];

static std::forward_list<api_call_result> api_call_results;
static std::recursive_mutex api_call_result_mutex;

static std::forward_list<call_output> call_outputs;
static std::recursive_mutex call_output_mutex;

int Init(void)
{
    last_api_call_id = 0;
    return 0;
}

void RegisterCallback(CCallbackBase *callback, enum steam_callback_type type)
{
    if (type >= STEAM_CALLBACK_TYPE_MAX)
        return;

    callback->m_iCallback = type;
    callback->m_nCallbackFlags |= CCallbackBase::k_ECallbackFlagsRegistered;

    callback_mutex[type].lock();
    callbacks[type].push_front(callback);
    callback_mutex[type].unlock();
}

void UnregisterCallback(CCallbackBase *callback)
{
    if (callback->m_iCallback >= STEAM_CALLBACK_TYPE_MAX)
        return;

    callback_mutex[callback->m_iCallback].lock();
    callbacks[callback->m_iCallback].remove(callback);
    callback_mutex[callback->m_iCallback].unlock();

    callback->m_nCallbackFlags &= ~CCallbackBase::k_ECallbackFlagsRegistered;
}

void RegisterApiCallResult(CCallbackBase *callback, SteamAPICall_t api_call)
{
    struct api_call_result call_result;

    if (callback->m_iCallback >= STEAM_CALLBACK_TYPE_MAX)
        return;

    call_result.callback = callback;
    call_result.api_call = api_call;

    callback->m_nCallbackFlags |= CCallbackBase::k_ECallbackFlagsRegistered;

    api_call_result_mutex.lock();
    api_call_results.push_front(call_result);
    api_call_result_mutex.unlock();
}

void UnregisterApiCallResult(CCallbackBase *callback, SteamAPICall_t api_call)
{
    if (callback->m_iCallback >= STEAM_CALLBACK_TYPE_MAX)
        return;

    api_call_result_mutex.lock();
    for (auto it = api_call_results.before_begin(); it != api_call_results.end(); ) {
        auto prev_it = it;
        it++;
        if (it == api_call_results.end())
            break;
            
        if (it->callback == callback && it->api_call == api_call) {
            api_call_results.erase_after(prev_it);
            break;
        }
    }
    api_call_result_mutex.unlock();
    
    callback->m_nCallbackFlags &= ~CCallbackBase::k_ECallbackFlagsRegistered;
}

void DispatchCallbackOutput(enum steam_callback_type type, void *data, size_t data_size)
{
    struct call_output out;

    if (type >= STEAM_CALLBACK_TYPE_MAX)
        return;

    out.type = type;
    out.is_api_call = false;
    out.is_handled = false;
    out.io_failure = false;
    out.api_call = 0;
    out.data = nullptr;
    if (data) {
        out.data = malloc(data_size);
        memcpy(out.data, data, data_size);
    }
    out.data_size = data_size;

    call_output_mutex.lock();
    call_outputs.push_front(out);
    call_output_mutex.unlock();
}

SteamAPICall_t AwaitApiCallResultOutput(void)
{
    return ++last_api_call_id;
}

void DispatchApiCallResultOutput(SteamAPICall_t api_call, enum steam_callback_type type, bool io_failure, void *data, size_t data_size)
{
    struct call_output out;

    if (type >= STEAM_CALLBACK_TYPE_MAX)
        return;

    out.type = type;
    out.is_api_call = true;
    out.is_handled = false;
    out.io_failure = io_failure;
    out.api_call = api_call;
    out.data = nullptr;
    if (data) {
        out.data = malloc(data_size);
        memcpy(out.data, data, data_size);
    }
    out.data_size = data_size;

    call_output_mutex.lock();
    call_outputs.push_front(out);
    call_output_mutex.unlock();
}

static bool ApiCallResultOutput(bool only_check, SteamAPICall_t api_call, void *data, int data_size, enum steam_callback_type type_expected, bool *io_failure)
{
    bool result = false;

    if (io_failure)
        *io_failure = false;

    call_output_mutex.lock();

    for (auto it = call_outputs.before_begin(); it != call_outputs.end(); ) {
        auto prev_it = it;
        it++;
        if (it == call_outputs.end())
            break;
            
        if (it->api_call != api_call)
            continue;
            
        if (!only_check) {
            if (it->data_size != data_size || it->type != type_expected)
                continue;

            if (data)
                memcpy(data, it->data, it->data_size);
        }

        if (io_failure)
            *io_failure = it->io_failure;

        if (!only_check) {
            if (it->data) {
                free(it->data);
                it->data = nullptr;
            }
            call_outputs.erase_after(prev_it);
            it = prev_it;
        }

        result = true;
        break;
    }

    call_output_mutex.unlock();

    return result;
}

bool ApiCallResultIsOutputAvailable(SteamAPICall_t api_call, bool *io_failure)
{
    return ApiCallResultOutput(true, api_call, nullptr, 0, STEAM_CALLBACK_TYPE_ZERO, io_failure);
}

bool ApiCallResultGetOutput(SteamAPICall_t api_call, void *data, int data_size, enum steam_callback_type type_expected, bool *io_failure)
{
    return ApiCallResultOutput(false, api_call, data, data_size, type_expected, io_failure);
}

static bool HandleCallbackOutput(struct call_output *out)
{
    if (out->type >= STEAM_CALLBACK_TYPE_MAX)
        return false;

    if (out->is_api_call)
        return false;

    if (out->is_handled)
        return true;

    callback_mutex[out->type].lock();

    for (auto it = callbacks[out->type].begin(); it != callbacks[out->type].end(); it++) {
        CCallbackBase *callback = *it;
        int size = callback->GetCallbackSizeBytes();
        if (size != out->data_size)
        {
            LOG(LL_ERROR, LCF_STEAM, "Callback %d data size mismatch: expected %u != got %u", out->type, out->data_size, size);
            continue;
        }

        LOG(LL_DEBUG, LCF_STEAM, "   Run callback of type %u", out->type);
        callback->Run(out->data);
        out->is_handled = true;
    }

    callback_mutex[out->type].unlock();
    return out->is_handled;
}

static bool HandleApiCallResultOutput(struct call_output *out)
{
    if (out->type >= STEAM_CALLBACK_TYPE_MAX)
        return false;

    if (!out->is_api_call)
        return false;

    if (out->is_handled)
        return true;

    api_call_result_mutex.lock();

    for (auto it = api_call_results.before_begin(); it != api_call_results.end(); ) {
        auto prev_it = it;
        it++;

        if (it == api_call_results.end())
            break;

        CCallbackBase *callback = it->callback;

        if (callback->m_iCallback != out->type || it->api_call != out->api_call)
            continue;

        int size = callback->GetCallbackSizeBytes();
        if (size != out->data_size) {
            LOG(LL_ERROR, LCF_STEAM, "   Call result #%" PRIu64 " %u data size mismatch: expected %u != got %u", out->api_call, out->type, out->data_size, size);
            continue;
        }

        LOG(LL_DEBUG, LCF_STEAM, "   Run call result #%" PRIu64 " of type %u", out->api_call, out->type);
        callback->Run(out->data, out->io_failure, out->api_call);

        api_call_results.erase_after(prev_it);
        it = prev_it;
        
        callback->m_nCallbackFlags &= ~CCallbackBase::k_ECallbackFlagsRegistered;
        out->is_handled = true;
    }

    api_call_result_mutex.unlock();

    return out->is_handled;
}

void Run(void)
{
    call_output_mutex.lock();

    for (auto it = call_outputs.before_begin(); it != call_outputs.end(); ) {
        auto prev_it = it;
        it++;

        if (it == call_outputs.end())
            break;        

        struct call_output out = *it;

        if (out.type >= STEAM_CALLBACK_TYPE_MAX)
            continue;

        if (out.is_api_call)
            HandleApiCallResultOutput(&out);
        else
            HandleCallbackOutput(&out);

        if (!out.is_handled)
            continue;

        call_outputs.erase_after(prev_it);
        it = prev_it;
    }

    call_output_mutex.unlock();
}

}
}
