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

#include "UnityHacks.h"
#include "logging.h"
#include "frame.h"
#include "GlobalState.h"
#include "global.h"
#include "GameHacks.h"
#include "hookpatch.h"
#ifdef __unix__
#include "checkpoint/ProcSelfMaps.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "checkpoint/MachVmMaps.h"
#endif
#include "checkpoint/MemArea.h"
#include "checkpoint/ThreadManager.h"
#include "checkpoint/ThreadInfo.h"
#include "renderhud/UnityDebug.h"
#include "../shared/unity_funcs.h"

#include <unistd.h>
#include <memory>
#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
#include <sys/mman.h> // PROT_READ, PROT_WRITE, etc.
#include <atomic>

namespace libtas {

static bool unity = false;

void UnityHacks::setUnity()
{
    if (!unity) {
        LOG(LL_DEBUG, LCF_HOOK, "   detected Unity engine");
        unity = true;
    }
}

bool UnityHacks::isUnity()
{
    return unity;
}

typedef void UnityVersion;
class JobQueue;
class BackgroundJobQueue;
typedef int JobQueue_JobQueuePriority;
typedef int MemLabelId;
typedef void JobGroup;
typedef void JobQueue_ThreadInfo;
typedef long BackgroundJobQueue_JobFence;
typedef void JobInfo;
typedef void JobBatchHandles;
typedef void AtomicQueue;
typedef void JobScheduler;
typedef void ujob_control_t;
typedef void ujob_lane_t;
typedef void ujob_job_t;
typedef long ujob_handle_t;
typedef void ujob_dependency_chain;
typedef int WorkStealingRange;

struct JobsCallbackFunctions {
    void (*execute)(void*, int);
    void (*completed)(void*);
};

// typedef void (*JobsCallbackFunctions)(void*);
// typedef void (*JobsCallbackFunctionsParallel)(void*, int);
typedef void ScriptingBackendNativeObjectPtrOpaque;
class JobScheduleParameters;
class JobFence;
typedef uint8_t PreloadManager;
typedef long PreloadManager_UpdatePreloadingFlags;

typedef void AsyncReadManagerThreaded;
typedef uint8_t AsyncReadCommand;
typedef int AsyncReadCommand_Status;
typedef long VFS_FileSize;
typedef void AsyncUploadManager;
typedef int AsyncUploadHandler;
typedef void AssetContext;
typedef long FileReadFlags;
typedef void GfxDevice;
typedef void AsyncUploadManagerSettings;

typedef void ArchiveStorageConverter;
typedef void IArchiveStorageConverterListener;
typedef void AssetBundleLoadFromStreamAsyncOperation;

typedef void SoundHandle_Instance;
typedef void SampleClip;
typedef void FMOD_CREATESOUNDEXINFO;

struct Int128 {
    long a;
    long b;
};

struct JobGroupID {
    JobGroup* group;
    int tag;
};

typedef uint8_t PreloadManagerOperation;

class U4_PreloadManagerOperation
{
public:
    virtual ~U4_PreloadManagerOperation();
    virtual bool IsDone(PreloadManagerOperation* po);
    virtual float GetProgress(PreloadManagerOperation* po);
    virtual long GetPriority(PreloadManagerOperation* po);
    virtual void SetPriority(PreloadManagerOperation* po, int p);
    virtual bool GetAllowSceneActivation(PreloadManagerOperation* po);
    virtual void SetAllowSceneActivation(PreloadManagerOperation* po, bool sa);
    virtual void Perform(PreloadManagerOperation* po);
    virtual void IntegrateMainThread(PreloadManagerOperation* po);
    virtual void HasIntegrateMainThread(PreloadManagerOperation* po);
    virtual bool MustCompleteNextFrame(PreloadManagerOperation* po);
    virtual char* GetDebugName(PreloadManagerOperation* po);
};

class U5_PreloadManagerOperation
{
public:
    virtual ~U5_PreloadManagerOperation();
    virtual bool IsDone(PreloadManagerOperation* po);
    virtual float GetProgress(PreloadManagerOperation* po);
    virtual long GetPriority(PreloadManagerOperation* po);
    virtual void SetPriority(PreloadManagerOperation* po, int p);
    virtual bool GetAllowSceneActivation(PreloadManagerOperation* po);
    virtual void SetAllowSceneActivation(PreloadManagerOperation* po, bool sa);
    virtual void Release(PreloadManagerOperation* po);
    virtual void Perform(PreloadManagerOperation* po);
    virtual void IntegrateTimeSliced(PreloadManagerOperation* po, int i);
    virtual void IntegrateMainThread(PreloadManagerOperation* po);
    virtual bool MustCompleteNextFrame(PreloadManagerOperation* po);
    virtual bool GetAllowParallelExecution(PreloadManagerOperation* po);
    virtual char* GetDebugName(PreloadManagerOperation* po);
};

class U2018_PreloadManagerOperation
{
public:
    virtual ~U2018_PreloadManagerOperation();
    virtual bool IsDone(PreloadManagerOperation* po);
    virtual float GetProgress(PreloadManagerOperation* po);
    virtual long GetPriority(PreloadManagerOperation* po);
    virtual void SetPriority(PreloadManagerOperation* po, int p);
    virtual bool GetAllowSceneActivation(PreloadManagerOperation* po);
    virtual void SetAllowSceneActivation(PreloadManagerOperation* po, bool sa);
    virtual void InvokeCoroutine(PreloadManagerOperation* po);
    virtual void SetFinalTiming(PreloadManagerOperation* po, float a, float b, float c, float d);
    virtual void Perform(PreloadManagerOperation* po);
    virtual void IntegrateTimeSliced(PreloadManagerOperation* po, int i);
    virtual void IntegrateMainThread(PreloadManagerOperation* po);
    virtual bool MustCompleteNextFrame(PreloadManagerOperation* po);
    virtual bool GetAllowParallelExecution(PreloadManagerOperation* po);
    virtual char* GetDebugName(PreloadManagerOperation* po);
};

class U6_PreloadManagerOperation
{
public:
    virtual long ExceptionToPropagateToAwaiter();
    virtual ~U6_PreloadManagerOperation();
    virtual bool IsDone(PreloadManagerOperation* po);
    virtual float GetProgress(PreloadManagerOperation* po);
    virtual long GetPriority(PreloadManagerOperation* po);
    virtual void SetPriority(PreloadManagerOperation* po, int p);
    virtual bool GetAllowSceneActivation(PreloadManagerOperation* po);
    virtual void SetAllowSceneActivation(PreloadManagerOperation* po, bool sa);
    virtual void InvokeCoroutine(PreloadManagerOperation* po);
    virtual long Cancel(PreloadManagerOperation* po);
    virtual void SetFinalTiming(PreloadManagerOperation* po, float a, float b, float c, float d);
    virtual void Perform(PreloadManagerOperation* po);
    virtual void IntegrateTimeSliced(PreloadManagerOperation* po, int i);
    virtual void IntegrateMainThread(PreloadManagerOperation* po);
    virtual bool MustCompleteNextFrame(PreloadManagerOperation* po);
    virtual bool CanLoadObjects(PreloadManagerOperation* po);
    virtual bool CanPerformWhileObjectsLoading(PreloadManagerOperation* po);
    virtual bool GetAllowParallelExecution(PreloadManagerOperation* po);
    virtual char* GetDebugName(PreloadManagerOperation* po);
};

namespace orig {
    void (*UnityVersion_UnityVersion)(UnityVersion* u, const char* s) = nullptr;

    void (*U4_JobScheduler_AwakeIdleWorkerThreads)(JobScheduler* t, int x) = nullptr;
    long (*U4_JobScheduler_FetchNextJob)(JobScheduler* t, int* x) = nullptr;
    void (*U4_JobScheduler_ProcessJob)(JobScheduler* t, JobInfo* x, int y) = nullptr;
    int (*U4_JobScheduler_SubmitJob)(JobScheduler* t, int x, void* (*y)(void*), void* z, void* volatile* a) = nullptr;
    void (*U4_JobScheduler_WaitForGroup)(JobScheduler* t, int x) = nullptr;

    void (*U5_BackgroundJobQueue_ExecuteMainThreadJobs)(BackgroundJobQueue *t) = nullptr;
    void (*U5_BackgroundJobQueue_ScheduleJob)(BackgroundJobQueue* t, void (*x)(void*), void* y) = nullptr;
    void (*U5_BackgroundJobQueue_ScheduleMainThreadJob)(BackgroundJobQueue* t, void (*x)(void*), void* y) = nullptr;
    JobGroup* (*U5_JobQueue_CreateJobBatch)(JobQueue *t, void (*func)(void*), void* arg, JobGroup* z, int id, JobGroup* a) = nullptr;
    long (*U5_JobQueue_EnqueueAll)(JobQueue* t, JobGroup* x, JobGroup* y) = nullptr;
    long (*U5_JobQueue_EnqueueAllInternal)(JobQueue* t, JobGroup* x, JobGroup* y, AtomicQueue* z, int* a) = nullptr;
    long (*U5_JobQueue_Exec)(JobQueue* t, JobInfo* x, long long y, int z) = nullptr;
    long (*U5_JobQueue_ExecuteJobFromQueue)(JobQueue* t) = nullptr;
    bool (*U5_JobQueue_ExecuteOneJob)(JobQueue *t) = nullptr;
    long (*U5_JobQueue_MainEnqueueAll)(JobQueue* t, JobGroup* x, JobGroup* y) = nullptr;
    long (*U5_JobQueue_Pop)(JobQueue* t, JobGroupID x) = nullptr;
    long (*U5_JobQueue_ProcessJobs)(JobQueue* x, void* y) = nullptr;
    void (*U5_JobQueue_ScheduleJob)(JobQueue *t, void (*func)(void*), void* arg, JobGroup* z, int a, int b) = nullptr;
    JobGroupID (*U5_JobQueue_ScheduleGroup)(JobQueue *t, JobGroup* x, int y) = nullptr;
    void (*U5_JobQueue_ScheduleGroups)(JobQueue *t, JobGroup* x, JobGroup* y) = nullptr;
    void (*U5_JobQueue_WaitForJobGroup)(JobQueue* t, JobGroup* x, int y, bool z) = nullptr;

    void (*U2K_BackgroundJobQueue_ScheduleJobInternal)(BackgroundJobQueue *t, void (*x)(void*), void* y, BackgroundJobQueue_JobFence* z, JobQueue_JobQueuePriority a) = nullptr;
    void (*U2K_BackgroundJobQueue_ScheduleMainThreadJobInternal)(BackgroundJobQueue *t, void (*x)(void*), void* y) = nullptr;
    void (*U2K_JobQueue_CompleteAllJobs)(JobQueue *t) = nullptr;
    long (*U2K_JobQueue_Exec)(JobQueue *t, JobInfo* x, long long y, int z, bool a) = nullptr;
    long (*U2K_JobQueue_ExecuteJobFromQueue)(JobQueue *t, bool x) = nullptr;
    long (*U2K_JobQueue_ProcessJobs)(JobQueue_ThreadInfo* x, void* y) = nullptr;
    void (*U2K_JobQueue_ScheduleDependencies)(JobQueue *t, JobGroupID *x, JobInfo *y, JobInfo *z, bool a) = nullptr;
    long (*U2K_JobQueue_ScheduleJobMultipleDependencies)(JobQueue *t, void (*x)(void*), void* y, JobGroupID* z, int a, MemLabelId b) = nullptr;
    JobGroupID (*U2K_JobQueue_ScheduleGroupInternal)(JobQueue *t, JobGroup* x, int y, bool z) = nullptr;
    void (*U2K_JobQueue_WaitForJobGroupID)(JobQueue *t, JobGroup *x, int y, bool a) = nullptr;

    int (*U6_job_completed)(ujob_control_t* x, ujob_lane_t* y, ujob_job_t* z, ujob_handle_t a) = nullptr;
    int (*U6_JobsUtility_CUSTOM_CreateJobReflectionData)(ScriptingBackendNativeObjectPtrOpaque* x, ScriptingBackendNativeObjectPtrOpaque* y, ScriptingBackendNativeObjectPtrOpaque* z, ScriptingBackendNativeObjectPtrOpaque* a, ScriptingBackendNativeObjectPtrOpaque* b) = nullptr;
    int (*U6_JobsUtility_CUSTOM_Schedule)(JobScheduleParameters* x, JobFence* y) = nullptr;
    void (*U6_JobQueue_ScheduleGroups)(JobQueue *t, JobBatchHandles* x, int y) = nullptr;
    bool (*U6_lane_guts)(ujob_control_t* x, ujob_lane_t* y, int z, int a, ujob_dependency_chain const* b) = nullptr;
    long (*U6_ScheduleBatchJob)(void* x, ujob_handle_t y) = nullptr;
    void (*U6_ujob_execute_job)(ujob_control_t*, ujob_lane_t*, ujob_job_t*, ujob_handle_t, unsigned int) = nullptr;
    void (*U6_ujob_participate)(ujob_control_t* x, ujob_handle_t y, ujob_job_t* z, int* a, ujob_dependency_chain const* b) = nullptr;
    unsigned long (*U6_ujob_schedule_job_internal)(ujob_control_t* x, ujob_handle_t y, unsigned int z) = nullptr;
    ujob_handle_t (*U6_ujob_schedule_parallel_for_internal)(ujob_control_t* x, JobsCallbackFunctions* y, void* z, WorkStealingRange* a, unsigned int b, unsigned long c, ujob_handle_t const* d, long e) = nullptr;
    void (*U6_ujob_wait_for)(ujob_control_t *x, ujob_handle_t y, int z) = nullptr;
    void (*U6_ujobs_add_to_lane_and_wake_one_thread)(ujob_control_t*, ujob_job_t*, ujob_lane_t*) = nullptr;
    void (*U6_worker_thread_routine)(void* x) = nullptr;

    void (*U6_PreloadManager_AddToQueue)(PreloadManager* m, PreloadManagerOperation* o) = nullptr;
    PreloadManagerOperation* (*U6_PreloadManager_PrepareProcessingPreloadOperation)(PreloadManager* m) = nullptr;
    void (*U6_PreloadManager_ProcessSingleOperation)(PreloadManager* m) = nullptr;
    void (*U6_PreloadManager_UpdatePreloading)(PreloadManager* m) = nullptr;
    long (*U4_PreloadManager_UpdatePreloadingSingleStep)(PreloadManager* m, bool i) = nullptr;
    long (*U6_PreloadManager_UpdatePreloadingSingleStep)(PreloadManager* m, PreloadManager_UpdatePreloadingFlags f, int i) = nullptr;
    void (*U6_PreloadManager_WaitForAllAsyncOperationsToComplete)(PreloadManager* m) = nullptr;
    long (*U6_PreloadManager_Run)(void* p) = nullptr;
    PreloadManagerOperation* (*U2K_PreloadManager_PeekIntegrateQueue)(PreloadManager* m) = nullptr;
    bool (*U2K_PreloadManager_IsLoadingOrQueued)(PreloadManager* m) = nullptr;

    void (*U5_AsyncReadManagerThreaded_WaitDone)(AsyncReadManagerThreaded *t, AsyncReadCommand *c) = nullptr;
    void (*U6_AsyncReadManagerThreaded_Request)(AsyncReadManagerThreaded *t, AsyncReadCommand *c) = nullptr;
    void (*U2K_AsyncReadManagerThreaded_SyncRequest)(AsyncReadManagerThreaded *t, AsyncReadCommand *c) = nullptr;
    void (*U6_AsyncReadManagerManaged_OpenCompleteCallback)(AsyncReadCommand *c, AsyncReadCommand_Status s) = nullptr;
    void (*U6_AsyncReadManagerManaged_ReadCompleteCallback)(AsyncReadCommand *c, AsyncReadCommand_Status s) = nullptr;
    void (*U6_AsyncReadManagerManaged_CloseCompleteCallback)(AsyncReadCommand *c, AsyncReadCommand_Status s) = nullptr;
    void (*U6_AsyncReadManagerManaged_CloseCachedFileCompleteCallback)(AsyncReadCommand *c, AsyncReadCommand_Status s) = nullptr;
    void (*U6_AsyncUploadManager_AsyncReadSuccess)(AsyncUploadManager *t, AsyncReadCommand *c) = nullptr;
    Int128 (*U6_AsyncUploadManager_QueueUploadAsset)(AsyncUploadManager *t, char const* x, VFS_FileSize y, unsigned int z, unsigned int a, AsyncUploadHandler* b, AssetContext *c, unsigned char* d, FileReadFlags e) = nullptr;
    void (*U6_AsyncUploadManager_AsyncResourceUpload)(AsyncUploadManager *t, GfxDevice *x, int y, AsyncUploadManagerSettings *z) = nullptr;
    void (*U6_AsyncUploadManager_AsyncReadCallbackStatic)(AsyncReadCommand *c, AsyncReadCommand_Status s) = nullptr;
    void (*U6_AsyncUploadManager_ScheduleAsyncCommandsInternal)(AsyncUploadManager *t) = nullptr;
    void (*U6_AsyncUploadManager_CloseFile)(AsyncUploadManager *t, char* s) = nullptr;
    void (*U6_SignalCallback)(AsyncReadCommand *c, AsyncReadCommand_Status s) = nullptr;
    void (*U6_SyncReadRequest)(AsyncReadCommand* c) = nullptr;
    
    void (*U6_ArchiveStorageConverter_ArchiveStorageConverter)(ArchiveStorageConverter* c, IArchiveStorageConverterListener* l, bool b) = nullptr;
    int (*U6_ArchiveStorageConverter_ProcessAccumulatedData)(ArchiveStorageConverter* c) = nullptr;
    int (*U6_AssetBundleLoadFromStreamAsyncOperation_FeedStream)(AssetBundleLoadFromStreamAsyncOperation *o, void const* x, unsigned long y) = nullptr;
    int (*U6_LoadFMODSound)(SoundHandle_Instance** si, char const* s, unsigned int f, SampleClip* c, unsigned int i, VFS_FileSize fs, FMOD_CREATESOUNDEXINFO* in) = nullptr;
}

#include <signal.h>

static char* unity_version;
static int unity_version_int[3] = {0, 0, 0};

static void UnityVersion_UnityVersion(UnityVersion* u, const char* s)
{
    LOG(LL_TRACE, LCF_HACKS, "UnityVersion_UnityVersion called with version %s", s);
    if (unity_version_int[0] == 0) {
        unity_version = strdup(s);
        
        char* t = strtok(unity_version, ".");
        
        for (int i=0; (i < 3) && (t != nullptr); i++) {
            unity_version_int[i] = atoi(t);
            LOG(LL_TRACE, LCF_HACKS, "Version %d", unity_version_int[i]);
            t = strtok(nullptr, ".");
        }
    }

    return orig::UnityVersion_UnityVersion(u, s);
}

static int GetUnityVersionMaj()
{
    if (unity_version_int[0] != 0)
        return unity_version_int[0];
    else {
        /* Try to guess the version from the hooked functions */
        if (orig::U4_JobScheduler_ProcessJob)
            unity_version_int[0] = 4;
        else if (orig::U5_JobQueue_Exec)
            unity_version_int[0] = 5;
        else if (orig::U6_ujob_execute_job)
            unity_version_int[0] = 6000;
        else
            unity_version_int[0] = 2018; // TODO!
    }
    return unity_version_int[0];
}

static long U4_JobScheduler_FetchNextJob(JobScheduler* t, int* x)
{
    LOGTRACE(LCF_HACKS);
    ThreadInfo* thread = ThreadManager::getCurrentThread();
    thread->unityThread = true;
    
    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_JOBS) {
        /* Return 0 to prevent worker threads from executing a job */
        return 0;
    }

    return orig::U4_JobScheduler_FetchNextJob(t, x);
}

static void U4_JobScheduler_ProcessJob(JobScheduler* t, JobInfo* x, int y)
{
    LOGTRACE(LCF_HACKS);
    return orig::U4_JobScheduler_ProcessJob(t, x, y);
}

static void U4_JobScheduler_WaitForGroup(JobScheduler* t, int x)
{
    LOGTRACE(LCF_HACKS);
    return orig::U4_JobScheduler_WaitForGroup(t, x);
}

static void U4_JobScheduler_AwakeIdleWorkerThreads(JobScheduler* t, int x)
{
    LOGTRACE(LCF_HACKS);
    return orig::U4_JobScheduler_AwakeIdleWorkerThreads(t, x);
}

/* We need to wrap the job, to know when it is executed */
struct U4_Job
{
    void* (*func)(void*);
    void* arg;
};

static void* U4_ExecJob(void* arg)
{
    LOGTRACE(LCF_HACKS);
    ThreadInfo* th = ThreadManager::getCurrentThread();
    th->unityJobCount++;
    
    PROFILE_SCOPE("Job", PROFILER_INFO_UNITY);
    U4_Job* j = reinterpret_cast<U4_Job*>(arg);
    void* ret = j->func(j->arg);
    delete j;
    return ret;
}

static int U4_JobScheduler_SubmitJob(JobScheduler* t, int x, void* (*y)(void*), void* z, void* volatile* a)
{
    LOGTRACE(LCF_HACKS);
    U4_Job *j = new U4_Job;
    j->func = y;
    j->arg = z;
    return orig::U4_JobScheduler_SubmitJob(t, x, &U4_ExecJob, reinterpret_cast<void*>(j), a);
}

static void U5_BackgroundJobQueue_ExecuteMainThreadJobs(BackgroundJobQueue *t)
{
    LOGTRACE(LCF_HACKS);
    return orig::U5_BackgroundJobQueue_ExecuteMainThreadJobs(t);
}

static void U5_BackgroundJobQueue_ScheduleJob(BackgroundJobQueue* t, void (*x)(void*), void* y)
{
    LOGTRACE(LCF_HACKS);
    return orig::U5_BackgroundJobQueue_ScheduleJob(t, x, y);
}

static void U5_BackgroundJobQueue_ScheduleMainThreadJob(BackgroundJobQueue* t, void (*x)(void*), void* y)
{
    LOGTRACE(LCF_HACKS);
    return orig::U5_BackgroundJobQueue_ScheduleMainThreadJob(t, x, y);
}

static JobGroup* U5_JobQueue_CreateJobBatch(JobQueue *t, void (*func)(void*), void* arg, JobGroup* z, int id, JobGroup* a)
{
    LOGTRACE(LCF_HACKS);
    return orig::U5_JobQueue_CreateJobBatch(t, func, arg, z, id, a);
}

static long U5_JobQueue_EnqueueAll(JobQueue* t, JobGroup* x, JobGroup* y)
{
    LOGTRACE(LCF_HACKS);
    return orig::U5_JobQueue_EnqueueAll(t, x, y);
}

static long U5_JobQueue_EnqueueAllInternal(JobQueue* t, JobGroup* x, JobGroup* y, AtomicQueue* z, int* a)
{
    LOGTRACE(LCF_HACKS);
    return orig::U5_JobQueue_EnqueueAllInternal(t, x, y, z, a);
}

static long U5_JobQueue_Exec(JobQueue* t, JobInfo* x, long long y, int z)
{
    LOG(LL_TRACE, LCF_HACKS, "U5_JobQueue_Exec called with JobInfo %p", x);
    PROFILE_SCOPE("Job", PROFILER_INFO_UNITY);
    long executed = orig::U5_JobQueue_Exec(t, x, y, z);
    if (executed) {
        ThreadInfo* th = ThreadManager::getCurrentThread();
        th->unityJobCount++;
    }
    
    return executed;
}

static long U5_JobQueue_ExecuteJobFromQueue(JobQueue* t)
{
    LOGTRACE(LCF_HACKS);
    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_JOBS)
        return 0;
    
    return orig::U5_JobQueue_ExecuteJobFromQueue(t);
}

static bool U5_JobQueue_ExecuteOneJob(JobQueue *t)
{
    LOGTRACE(LCF_HACKS);
    return orig::U5_JobQueue_ExecuteOneJob(t);
}

static long U5_JobQueue_MainEnqueueAll(JobQueue* t, JobGroup* x, JobGroup* y)
{
    LOGTRACE(LCF_HACKS);
    return orig::U5_JobQueue_MainEnqueueAll(t, x, y);
}

static long U5_JobQueue_Pop(JobQueue* t, JobGroupID x)
{
    LOGTRACE(LCF_HACKS);
    return orig::U5_JobQueue_Pop(t, x);
}

static long U5_JobQueue_ProcessJobs(JobQueue* t, void* x)
{
    LOGTRACE(LCF_HACKS);
    
    ThreadInfo* thread = ThreadManager::getCurrentThread();
    thread->unityThread = true;

    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_JOBS) {
        /* Calling U5_JobQueue_ProcessJobs() in worker threads may call
         * U5_JobQueue_Exec() directly without fetching a job queue with 
         * U5_JobQueue_ExecuteJobFromQueue(), so we wait indefinitively here.
         * To call jobs pushed in this worker thread, we will use the WorkerSteal
         * feature elsewhere.
         */
        if (!ThreadManager::isMainThread()) {
            while (!Global::is_exiting) {
                NATIVECALL(sleep(1));
            }
        }
    }

    return orig::U5_JobQueue_ProcessJobs(t, x);
}

static void U5_JobQueue_ScheduleJob(JobQueue *t, void (*func)(void*), void* arg, JobGroup* z, int a, int b)
{
    LOG(LL_TRACE, LCF_HACKS, "U5_JobQueue_ScheduleJob called with func %p, arg %p, JobGroup %p, JobGroup tag %d and priority %d", func, arg, z, a, b);
    return orig::U5_JobQueue_ScheduleJob(t, func, arg, z, a, b);
}

static JobGroupID U5_JobQueue_ScheduleGroup(JobQueue *t, JobGroup* x, int y)
{
    LOG(LL_TRACE, LCF_HACKS, "U5_JobQueue_ScheduleGroup called with JobGroup %p and priority %d", x, y);

    /* Return value is 16 bytes (accross registers RDX:RAX), so we need to use
     * a 16-byte struct to recover it. */
    JobGroupID j = orig::U5_JobQueue_ScheduleGroup(t, x, y);
    LOG(LL_DEBUG, LCF_HACKS, "    returns JobGroup %p and JobGroup tag %d", j.group, j.tag);

    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_JOBS) {
        /* Immediatly wait for the job */
        if (orig::U5_JobQueue_WaitForJobGroup)
            orig::U5_JobQueue_WaitForJobGroup(t, j.group, j.tag, true);
    }

    return j;
}

static void U5_JobQueue_ScheduleGroups(JobQueue *t, JobGroup* x, JobGroup* y)
{
    LOGTRACE(LCF_HACKS);
    return orig::U5_JobQueue_ScheduleGroups(t, x, y);
}

static void U5_JobQueue_WaitForJobGroup(JobQueue* t, JobGroup* x, int y, bool z)
{
    LOG(LL_TRACE, LCF_HACKS, "U5_JobQueue_WaitForJobGroup called with JobGroup %p, JobGroup tag %d and steal mode %d", x, y, z);
    return orig::U5_JobQueue_WaitForJobGroup(t, x, y, z);
}

static void U2K_BackgroundJobQueue_ScheduleJobInternal(BackgroundJobQueue *t, void (*x)(void*), void* y, BackgroundJobQueue_JobFence* z, JobQueue_JobQueuePriority a)
{
    LOGTRACE(LCF_HACKS);
    return orig::U2K_BackgroundJobQueue_ScheduleJobInternal(t, x, y, z, a);
}

static void U2K_BackgroundJobQueue_ScheduleMainThreadJobInternal(BackgroundJobQueue *t, void (*x)(void*), void* y)
{
    LOGTRACE(LCF_HACKS);
    return orig::U2K_BackgroundJobQueue_ScheduleMainThreadJobInternal(t, x, y);
}

static void U2K_JobQueue_CompleteAllJobs(JobQueue *t)
{
    LOGTRACE(LCF_HACKS);
    return orig::U2K_JobQueue_CompleteAllJobs(t);
}

static long U2K_JobQueue_Exec(JobQueue *t, JobInfo* x, long long y, int z, bool a)
{
    LOG(LL_TRACE, LCF_HACKS, "U2K_JobQueue_Exec called with JobInfo %p and sync %d", x, a);
    PROFILE_SCOPE("Job", PROFILER_INFO_UNITY);
    long executed = orig::U2K_JobQueue_Exec(t, x, y, z, a);
    if (executed) {
        ThreadInfo* th = ThreadManager::getCurrentThread();
        th->unityJobCount++;
    }
    return executed;
}

static long U2K_JobQueue_ExecuteJobFromQueue(JobQueue *t, bool x)
{
    LOG(LL_TRACE, LCF_HACKS, "U2K_JobQueue_ExecuteJobFromQueue called with sync %d", x);
    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_JOBS)
        return 0;
        
    return orig::U2K_JobQueue_ExecuteJobFromQueue(t, x);
}

static long U2K_JobQueue_ProcessJobs(JobQueue_ThreadInfo* x, void* y)
{
    LOGTRACE(LCF_HACKS);
    ThreadInfo* thread = ThreadManager::getCurrentThread();
    thread->unityThread = true;

    return orig::U2K_JobQueue_ProcessJobs(x, y);
}

static void U2K_JobQueue_ScheduleDependencies(JobQueue *t, JobGroupID *x, JobInfo *y, JobInfo *z, bool a)
{
    LOG(LL_TRACE, LCF_HACKS, "U2K_JobQueue_ScheduleDependencies called with sync %d", a);
    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_JOBS) {
        /* We enforce the worker steal flag, to be able to execute jobs from other threads */
        return orig::U2K_JobQueue_ScheduleDependencies(t, x, y, z, true);
    }

    return orig::U2K_JobQueue_ScheduleDependencies(t, x, y, z, a);
}

static long U2K_JobQueue_ScheduleJobMultipleDependencies(JobQueue *t, void (*x)(void*), void* y, JobGroupID* z, int a, MemLabelId b)
{
    LOGTRACE(LCF_HACKS);
    return orig::U2K_JobQueue_ScheduleJobMultipleDependencies(t, x, y, z, a, b);
}

static JobGroupID U2K_JobQueue_ScheduleGroupInternal(JobQueue *t, JobGroup *x, int y, bool z)
{
    LOG(LL_TRACE, LCF_HACKS, "U2K_JobQueue_ScheduleGroupInternal called with JobGroup %p, priority %d and sync %d", x, y, z);

    /* Return value is 16 bytes (accross registers RDX:RAX), so we need to use
     * a 16-byte struct to recover it. */
    // JobGroupID j = orig::U2K_JobQueue_ScheduleGroupInternal(t, x, y, z);
    JobGroupID j = orig::U2K_JobQueue_ScheduleGroupInternal(t, x, y, true);
    LOG(LL_DEBUG, LCF_HACKS, "    returns JobGroup %p and JobGroup tag %d", j.group, j.tag);

    /* Immediatly wait for the job */
    // U2K_JobQueue_WaitForJobGroupID(t, j.group, j.tag, true);
    return j;
}

static void U2K_JobQueue_WaitForJobGroupID(JobQueue* /* or ujob_control_t* */ t , JobGroup* /* or ujob_handle_t */ x, int y, bool z)
{
    LOG(LL_TRACE, LCF_HACKS, "U2K_JobQueue_WaitForJobGroupID called with JobGroup %p, JobGroup tag %d and steal mode %d", x, y, z);
    return orig::U2K_JobQueue_WaitForJobGroupID(t, x, y, z);
}

static int U6_job_completed(ujob_control_t* x, ujob_lane_t* y, ujob_job_t* z, ujob_handle_t a)
{
    // LOG(LL_TRACE, LCF_HACKS, "U6_job_completed called with job %p and handle %p", z, a);
    int ret = orig::U6_job_completed(x, y, z, a);
    return ret;
}

static void U6_JobQueue_ScheduleGroups(JobQueue *t, JobBatchHandles* x, int y)
{
    LOGTRACE(LCF_HACKS);
    return orig::U6_JobQueue_ScheduleGroups(t, x, y);
}

static int U6_JobsUtility_CUSTOM_CreateJobReflectionData(ScriptingBackendNativeObjectPtrOpaque* x, ScriptingBackendNativeObjectPtrOpaque* y, ScriptingBackendNativeObjectPtrOpaque* z, ScriptingBackendNativeObjectPtrOpaque* a, ScriptingBackendNativeObjectPtrOpaque* b)
{
    LOGTRACE(LCF_HACKS);
    return orig::U6_JobsUtility_CUSTOM_CreateJobReflectionData(x, y, z, a, b);
}

static int U6_JobsUtility_CUSTOM_Schedule(JobScheduleParameters* x, JobFence* y)
{
    LOGTRACE(LCF_HACKS);
    return orig::U6_JobsUtility_CUSTOM_Schedule(x, y);
}

static bool U6_lane_guts(ujob_control_t* x, ujob_lane_t* y, int z, int a, ujob_dependency_chain const* b)
{
    // LOGTRACE(LCF_HACKS);
    return orig::U6_lane_guts(x, y, z, a, b);
}

static long U6_ScheduleBatchJob(void* x, ujob_handle_t y)
{
    LOGTRACE(LCF_HACKS);
    return orig::U6_ScheduleBatchJob(x, y);
}

static void U6_ujobs_add_to_lane_and_wake_one_thread(ujob_control_t* x, ujob_job_t* y, ujob_lane_t* z)
{
    // LOGTRACE(LCF_HACKS);
    orig::U6_ujobs_add_to_lane_and_wake_one_thread(x, y, z);
}

static void U6_ujob_execute_job(ujob_control_t* x, ujob_lane_t* y, ujob_job_t* z, ujob_handle_t a, unsigned int b)
{
    LOGTRACE(LCF_HACKS);
    ThreadInfo* th = ThreadManager::getCurrentThread();
    th->unityJobCount++;
    PROFILE_SCOPE("Job", PROFILER_INFO_UNITY);
    return orig::U6_ujob_execute_job(x, y, z, a, b);
}

static void U6_ujob_participate(ujob_control_t* x, ujob_handle_t y, ujob_job_t** z, int* a, ujob_dependency_chain const* b)
{
    // LOGTRACE(LCF_HACKS);
    orig::U6_ujob_participate(x, y, z, a, b);
}

static unsigned long U6_ujob_schedule_job_internal(ujob_control_t* x, ujob_handle_t y, unsigned int z)
{
    LOGTRACE(LCF_HACKS);
    unsigned long ret = orig::U6_ujob_schedule_job_internal(x, y, z);
    
    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_JOBS) {
        /* In newer Unity 6 versions, there is a dedicated internal function for
         * waiting on a job */
        if (orig::U6_ujob_wait_for)
            orig::U6_ujob_wait_for(x, y, 1);
        else if (orig::U2K_JobQueue_WaitForJobGroupID)
            orig::U2K_JobQueue_WaitForJobGroupID(reinterpret_cast<JobQueue*>(x), reinterpret_cast<JobGroup*>(y), 0, true);
    }

    return ret;
}

struct callback_args_t {
    int count;
    JobsCallbackFunctions funcs;
    void* arg;
    // std::mutex mutex;
    // std::condition_variable cv;
    // bool completed;
};

static void job_callback_execute(void* arg, int)
{
    callback_args_t* args = reinterpret_cast<callback_args_t*>(arg);

    /* Perform all iterations of the loop inside this job */
    for (int i = 0; i < args->count; i++)
        args->funcs.execute(args->arg, i);
}

static void job_callback_completed(void* arg)
{
    callback_args_t* args = reinterpret_cast<callback_args_t*>(arg);
    if (args->funcs.completed) {
        args->funcs.completed(args->arg);
    }

    // std::unique_lock<std::mutex> lock(args->mutex);
    // args->completed = true;
    // args->cv.notify_all();
}

/* The function parameters from the symbol are *wrong*! 6th parameter must be 
 * 64-bit instead of 32-bit, and the last two parameters are not (int, uchar),
 * but a full long instead. Games crash instantly without this modification.
 * Original signature:
 * long ujob_schedule_parallel_for_internal
 *    (ujob_control_t *param_1, JobsCallbackFunctions *param_2, void *param_3,
 *     WorkStealingRange *param_4, uint param_5, uint param_6,
 *     ujob_handle_t *param_7, int param_8, uchar param_9)
 */ 
static ujob_handle_t U6_ujob_schedule_parallel_for_internal(ujob_control_t* x, JobsCallbackFunctions* y, void* job_callback_arg, WorkStealingRange* a, unsigned int count, unsigned long c, ujob_handle_t const* d, long e)
{
    LOG(LL_TRACE, LCF_HACKS, "U6_ujob_schedule_parallel_for_internal called with callback args %p , steal mode %d, count %d, ujob_handle_t %p", job_callback_arg, a?(*a):0, count, d);

    if (!(Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_JOBS))
        return orig::U6_ujob_schedule_parallel_for_internal(x, y, job_callback_arg, a, count, c, d, e);

    ujob_handle_t ret = 0;
    static JobsCallbackFunctions loop_callbacks = {&job_callback_execute, job_callback_completed};
    /* Instead of scheduling all the jobs in one call, we schedule one
     * individual job that will perform all the iterations in order. The job may still
     * run on a worker thread, but it should be fine for determinism.
     * Normally, the job callback function is receiving the loop index as 
     * second argument, so we pass our own callback function, which receives
     * the original callback, the original callback argument, and the iteration 
     * count. */
    callback_args_t* args = new callback_args_t;
    args->count = count;
    args->funcs.execute = y->execute;
    args->funcs.completed = y->completed;
    args->arg = job_callback_arg;
    // args->completed = false;
    
    // std::unique_lock<std::mutex> lock(args->mutex);
    
    ret = orig::U6_ujob_schedule_parallel_for_internal(x, &loop_callbacks, args, a, 1, c, d, e);

    /* Manually waiting on all jobs to execute */
    // args->cv.wait(lock, [&args] { return args->completed; });

    if (orig::U6_ujob_wait_for)
        orig::U6_ujob_wait_for(x, ret, 1);
    else if (orig::U2K_JobQueue_WaitForJobGroupID)
        orig::U2K_JobQueue_WaitForJobGroupID(reinterpret_cast<JobQueue*>(x), reinterpret_cast<JobGroup*>(ret), 0, true);

    /* It should be safe to delete our custom callback argument here */
    // delete args->loop_index;
    delete args;

    return ret;
}

static void U6_ujob_wait_for(ujob_control_t *x, ujob_handle_t y, int z)
{
    LOGTRACE(LCF_HACKS);
    return orig::U6_ujob_wait_for(x, y, z);
}

static void U6_worker_thread_routine(void* x)
{
    LOGTRACE(LCF_HACKS);
    ThreadInfo* thread = ThreadManager::getCurrentThread();
    thread->unityThread = true;
    return orig::U6_worker_thread_routine(x);
}

/* This is what I understand from how operations are handled:
 *
 * PreloadManager::AddToQueue:
 *   an operation is pushed inside a list of pending operations located in
 *   the PreloadManager struct (located in pm+0x318).
 *
 * PreloadManager::Run:
 *   Function ran by the Loading.Preload thread. It is constantly choosing,
 *   among the list of pending operations, the
 *   operation with the highest priority (PreloadManagerOperation::GetPriority),
 *   and then it pulls the operation from the list, and pushed it into a 
 *   second list of active operations (pm+0x338). This code is performed by the
 *   function PreloadManager::PrepareProcessingPreloadOperation() when it exists
 *
 *   Then it does some waiting, depending on some
 *   function results (CanLoadObjects, CanPerformWhileObjectsLoading).
 *
 *   Then it calls PreloadManagerOperation::Perform to initiate the operation.
 *
 *   It waits even more if PreloadManagerOperation::GetAllowParallelExecution
 *   returns true.
 *
 * PreloadManager::UpdatePreloadingSingleStep:
 *   Called by either PreloadManager::UpdatePreloading or 
 *   PreloadManager::WaitForAllAsyncOperationsToComplete.
 *
 *   Has an UpdatePreloadingFlags parameter which contains two flags:
 *   - if bit 0 is set, the update will not wake (or even create) the 
 *     Loading.Preload thread. So no operation will be moved from pending to active
 *   - if bit 1 is set, all finished operations are processed. Otherwise some of
 *     them are left in the active list
 *     (depending on PreloadManagerOperation::GetAllowSceneActivation?)
 *
 *   It takes the first active operation and calls PreloadManagerOperation::IntegrateTimeSliced.
 *
 *   If this function returns non-zero, I think it means that the operation
 *   was completed. In that case, it finishes processing the
 *   operation only under certain conditions: either 
 *   PreloadManagerOperation::GetAllowSceneActivation returns true, or 
 *   passed UpdatePreloadingFlags was non-zero.
 *
 *   In that case, it pulls the operation from the list of active operations,
 *   it calls PreloadManagerOperation::IntegrateMainThread, and finally
 *   PreloadManagerOperation::InvokeCoroutine.
 *
 *   It returns some value and bit 0 set (to guarantee a non-zero value).
 *
 * PreloadManager::UpdatePreloading:
 *   Called once each frame.
 *   Looks for all pending and active operations. If one returns true for
 *   PreloadManagerOperation::MustCompleteNextFrame, then it calls 
 *   PreloadManager::WaitForAllAsyncOperationsToComplete.
 *
 *   Otherwise, it calls UpdatePreloadingSingleStep in a loop until this 
 *   function returns 0.
 *
 * PreloadManager::WaitForAllAsyncOperationsToComplete:
 *   Calls PreloadManager::UpdatePreloadingSingleStep with UpdatePreloadingFlags
 *   being 2, in a loop while this function
 *   returns non-zero. If this function returns zero twice in a row, then it
 *   goes into wait mode (baselib::UnityClassic::CappedSemaphore::TryTimedAcquire()).
 * 
 *   It stops when there is no operation in both the pending list and the active list.
 */

/* For statistics */
static int added_count = 0;
static int processed_count = 0;

static bool is_preload_thread_running = false;

/* These two variables will keep track of the pending queue size and if the 
 * last activate operation was a non-parallel. We avoid as much as possible 
 * using struct internals, only relying on function calls */
static std::atomic<int> pending_queue_size;
static std::atomic<bool> is_current_pending_operation_non_parallel;

static bool Helper_PreloadManager_GetAllowParallelExecution(PreloadManagerOperation* o)
{
    if (GetUnityVersionMaj() == 6000)
        return (reinterpret_cast<U6_PreloadManagerOperation*>(o))->GetAllowParallelExecution(o);
    if ((GetUnityVersionMaj() >= 2017) && (GetUnityVersionMaj() <= 2020))
        return (reinterpret_cast<U2018_PreloadManagerOperation*>(o))->GetAllowParallelExecution(o);
    if (GetUnityVersionMaj() == 5)
        return (reinterpret_cast<U5_PreloadManagerOperation*>(o))->GetAllowParallelExecution(o);
    if (GetUnityVersionMaj() == 4)
        return true;

    LOG(LL_WARN, LCF_HACKS | LCF_FILEIO, "    PreloadManagerOperation struct for this Unity version is not known");
    return true;
}

/* This helper function waits until the pending queue is empty (after the last
 * operation was processed, this is important!), or if the Preload thread is
 * currently waiting on a non-parallel operation */
static void Helper_PreloadManager_WaitForQueueToProcess(PreloadManager* m, PreloadManagerOperation* o)
{
    /* Special case for when the preload thread is not running, hence it cannot
     * process operations. */
    if (!is_preload_thread_running)
        return;

    int i;
    for (i=0; i < 4000; i++) {
        /* If possible, we rely on these two functions to be hooked */
        if (orig::U6_PreloadManager_PrepareProcessingPreloadOperation && orig::U6_PreloadManager_ProcessSingleOperation) {
            if (pending_queue_size == 0)
                break;

            /* Check if the pending operation is non-parallel */
            if (is_current_pending_operation_non_parallel)
                break;
        }
        else {
            /* This sucks, we must rely on internals to get what we want... */
            if (GetUnityVersionMaj() == 6000) {
                bool pending_queue_empty = false;
                if (o) {
                    /* Operation status goes from 0 (queued) -> 1 (active) -> 2 (done) */
                    /* Check if the operation was pushed (status == 1) */
                    pending_queue_empty = (1 == *(int *)(o + 0x40));
                    // pending_queue_empty = (1 == *(int *)(o + 0x48));
                }
                else
                    pending_queue_empty = (0 == *(int *)(m + 0x328));
                if (pending_queue_empty)
                    break;

                /* Get the currently processed operation */
                long active_list_size = *(long *)(m + 0x348);
                if (active_list_size > 0) {
                    PreloadManagerOperation* current_pending_operation = *(PreloadManagerOperation **) (*(long *)(m + 0x338) + (active_list_size - 1) * 8);
                    
                    if (current_pending_operation) {
                        /* Check if the pending operation is non-parallel */
                        if (!Helper_PreloadManager_GetAllowParallelExecution(current_pending_operation)) {
                            LOG(LL_WARN, LCF_HACKS | LCF_FILEIO, "    exiting wait because of non-parallel");
                            break;
                        }
                    }
                }
            }
            else {
                LOG(LL_WARN, LCF_HACKS | LCF_FILEIO, "    does not know how to wait for the queue to be processed");                
            }
        }
        
        NATIVECALL(usleep(1000));
    }
    if (i == 4000) {
        LOG(LL_WARN, LCF_HACKS | LCF_FILEIO, "    timeout waiting for operation to be processed by Loading.Preload thread");
    }
}

static void U6_PreloadManager_AddToQueue(PreloadManager* m, PreloadManagerOperation* o)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    
    orig::U6_PreloadManager_AddToQueue(m, o);
    added_count++;
    pending_queue_size++;

    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_LOADS) {
        /* We must make sure that each operation pushed to the queue is activated
         * by the Loading.Preload thread immediately. If we don't do that, the order of
         * activated operations is not guaranteed. Indeed, the Loading.Preload chooses
         * the next operation to activate based on the PreloadManagerOperation::GetPriority()
         * result. So, depending on which operations were pushed, the order of active
         * operations may vary.
         *
         * There is one extra difficulty: when an operation returns false for
         * PreloadManagerOperation::GetAllowParallelExecution(), then it means that
         * after the operation is activated by the Loading.Preload thread, it
         * waits until the operation is handled by the main thread inside
         * PreloadManager::UpdatePreloadingSingleStep. Until this function is called,
         * any new operation will be queue and not activated.
         *
         * In this case, we cannot wait for all the operations to be activated. We
         * must delay until after PreloadManager::UpdatePreloadingSingleStep call.
         */

        Helper_PreloadManager_WaitForQueueToProcess(m, o);
    }
}

static PreloadManagerOperation* U6_PreloadManager_PrepareProcessingPreloadOperation(PreloadManager* m)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    PreloadManagerOperation* current_pending_operation = orig::U6_PreloadManager_PrepareProcessingPreloadOperation(m);
    
    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_LOADS) {
        /* After the end of this function, we don't know for how long the 
         * operation object will be alive. So we extract the information we want
         * from the operation here */
        if (current_pending_operation)
            is_current_pending_operation_non_parallel = !Helper_PreloadManager_GetAllowParallelExecution(current_pending_operation);
        else
            is_current_pending_operation_non_parallel = false;
    }
    
    return current_pending_operation;
}

static void U6_PreloadManager_ProcessSingleOperation(PreloadManager* m)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    PROFILE_SCOPE("Operation", PROFILER_INFO_UNITY);
    orig::U6_PreloadManager_ProcessSingleOperation(m);
    pending_queue_size--;
}

static void U6_PreloadManager_UpdatePreloading(PreloadManager* m)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);

    orig::U6_PreloadManager_UpdatePreloading(m);

    UnityDebug::update_preload(added_count, processed_count);
    added_count = 0;
    processed_count = 0;
}

static long U4_PreloadManager_UpdatePreloadingSingleStep(PreloadManager* m, bool i)
{
    LOG(LL_TRACE, LCF_HACKS | LCF_FILEIO, "U4_PreloadManager_UpdatePreloadingSingleStep called with bool %d", i);

    long ret = orig::U4_PreloadManager_UpdatePreloadingSingleStep(m, i);
    
    if (!ret) {
        return ret;
    }
    
    /* An operation was finished */
    processed_count++;

    /* In Unity 4, there is no non-parallel operations, so we don't have to 
     * do anything here, all sync is done in U6_PreloadManager_AddToQueue */
    return ret;
}

static long U6_PreloadManager_UpdatePreloadingSingleStep(PreloadManager* m, PreloadManager_UpdatePreloadingFlags f, int i)
{
    LOG(LL_TRACE, LCF_HACKS | LCF_FILEIO, "U6_PreloadManager_UpdatePreloadingSingleStep called with flags %lx and int %d", f, i);

    bool was_pending_queue_size = pending_queue_size;
    bool was_preload_thread_running = is_preload_thread_running;

    long ret = orig::U6_PreloadManager_UpdatePreloadingSingleStep(m, f, i);
    
    /* If no operation was finished, we can return immediately, as there is no
     * waiting to do. */
    if (!ret) {
        return ret;
    }
    
    /* An operation was finished */
    processed_count++;
    
    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_LOADS) {
        /* If there are pending operations, it is likely that this call processed a
         * non-parallel operation. Thus, we must wait for the Preload thread to 
         * activate pending operations. We must also wait for the case of the very
         * first call, as the Preload thread was not created yet. */
        if ((was_pending_queue_size > 0) || !was_preload_thread_running)
            Helper_PreloadManager_WaitForQueueToProcess(m, nullptr);
    }

    return ret;
}

static void U6_PreloadManager_WaitForAllAsyncOperationsToComplete(PreloadManager* m)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    return orig::U6_PreloadManager_WaitForAllAsyncOperationsToComplete(m);
}

static long U6_PreloadManager_Run(void* p)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    is_preload_thread_running = true;
    return orig::U6_PreloadManager_Run(p);
}

static PreloadManagerOperation* U2K_PreloadManager_PeekIntegrateQueue(PreloadManager* m)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    return orig::U2K_PreloadManager_PeekIntegrateQueue(m);
}

static bool U2K_PreloadManager_IsLoadingOrQueued(PreloadManager* m)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    return orig::U2K_PreloadManager_IsLoadingOrQueued(m);
}

static std::mutex async_read_mutex;
static std::condition_variable async_read_condition;
static bool async_read_complete = true;

static void U5_AsyncReadManagerThreaded_WaitDone(AsyncReadManagerThreaded *t, AsyncReadCommand *c)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    return orig::U5_AsyncReadManagerThreaded_WaitDone(t, c);
}

static void U6_AsyncReadManagerThreaded_Request(AsyncReadManagerThreaded *t, AsyncReadCommand *c)
{
    char* filepath = *(char**)c;
    LOG(LL_TRACE, LCF_HACKS | LCF_FILEIO, "U6_AsyncReadManagerThreaded_Request called with file %s", filepath);

    if (!(Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_READS))
        return orig::U6_AsyncReadManagerThreaded_Request(t, c);

    void* complete_callback = *((void**)(c + 0x38));
    // void* complete_callback_arg = *((void**)(c + 0x40));

    /* If possible, we can use this nice helper function that performs a sync
     * read from a command. I didn't experienced any softlock for now. */
    if (orig::U6_SyncReadRequest) {
        return orig::U6_SyncReadRequest(c);
    }

    if (orig::U2K_AsyncReadManagerThreaded_SyncRequest) {
        return orig::U2K_AsyncReadManagerThreaded_SyncRequest(t, c);
    }

    /* This function is less preferable, because it still performs the read in
     * another thread. */
    if (orig::U5_AsyncReadManagerThreaded_WaitDone) {
        orig::U6_AsyncReadManagerThreaded_Request(t, c);
        orig::U5_AsyncReadManagerThreaded_WaitDone(t, c);
        return;
    }

    if (complete_callback != 0) {
        /* We wait until the complete callback is called. */
        std::unique_lock<std::mutex> lock(async_read_mutex);
        async_read_complete = false;

        orig::U6_AsyncReadManagerThreaded_Request(t, c);
        
        async_read_condition.wait(lock, [] { return async_read_complete; });
    }
    else {
        orig::U6_AsyncReadManagerThreaded_Request(t, c);
    }
}

static void U2K_AsyncReadManagerThreaded_SyncRequest(AsyncReadManagerThreaded *t, AsyncReadCommand *c)
{
    char* filepath = *(char**)c;
    LOG(LL_TRACE, LCF_HACKS | LCF_FILEIO, "U2K_AsyncReadManagerThreaded_SyncRequest called with file %s", filepath);
    
    return orig::U2K_AsyncReadManagerThreaded_SyncRequest(t, c);
}

/* These are all AsyncReadCommand complete callback used */

static void U6_AsyncReadManagerManaged_OpenCompleteCallback(AsyncReadCommand *c, AsyncReadCommand_Status s)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    
    orig::U6_AsyncReadManagerManaged_OpenCompleteCallback(c, s);
    
    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_READS) {
        std::unique_lock<std::mutex> lock(async_read_mutex);
        async_read_complete = true;
        async_read_condition.notify_all();
    }
}

static void U6_AsyncReadManagerManaged_ReadCompleteCallback(AsyncReadCommand *c, AsyncReadCommand_Status s)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);

    orig::U6_AsyncReadManagerManaged_ReadCompleteCallback(c, s);

    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_READS) {
        std::unique_lock<std::mutex> lock(async_read_mutex);
        async_read_complete = true;
        async_read_condition.notify_all();
    }
}

static void U6_AsyncReadManagerManaged_CloseCompleteCallback(AsyncReadCommand *c, AsyncReadCommand_Status s)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);

    orig::U6_AsyncReadManagerManaged_CloseCompleteCallback(c, s);

    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_READS) {
        std::unique_lock<std::mutex> lock(async_read_mutex);
        async_read_complete = true;
        async_read_condition.notify_all();
    }
}

static void U6_AsyncReadManagerManaged_CloseCachedFileCompleteCallback(AsyncReadCommand *c, AsyncReadCommand_Status s)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);

    orig::U6_AsyncReadManagerManaged_CloseCachedFileCompleteCallback(c, s);

    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_READS) {
        std::unique_lock<std::mutex> lock(async_read_mutex);
        async_read_complete = true;
        async_read_condition.notify_all();
    }
}

static void U6_SignalCallback(AsyncReadCommand *c, AsyncReadCommand_Status s)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    
    orig::U6_SignalCallback(c, s);

    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_READS) {
        std::unique_lock<std::mutex> lock(async_read_mutex);
        async_read_complete = true;
        async_read_condition.notify_all();
    }
}

static void U6_AsyncUploadManager_AsyncReadCallbackStatic(AsyncReadCommand *c, AsyncReadCommand_Status s)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);

    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_UNITY_READS) {
        std::unique_lock<std::mutex> lock(async_read_mutex);
        async_read_complete = true;
        async_read_condition.notify_all();
    }

    orig::U6_AsyncUploadManager_AsyncReadCallbackStatic(c, s);
}

/* End of callbacks */

static void U6_AsyncUploadManager_AsyncReadSuccess(AsyncUploadManager *t, AsyncReadCommand *c)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    return orig::U6_AsyncUploadManager_AsyncReadSuccess(t, c);
}

static Int128 U6_AsyncUploadManager_QueueUploadAsset(AsyncUploadManager *t, char const* x, VFS_FileSize y, unsigned int z, unsigned int a, AsyncUploadHandler* b, AssetContext *c, unsigned char* d, FileReadFlags e)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    return orig::U6_AsyncUploadManager_QueueUploadAsset(t, x, y, z, a, b, c, d, e);
}

static void U6_AsyncUploadManager_AsyncResourceUpload(AsyncUploadManager *t, GfxDevice *x, int y, AsyncUploadManagerSettings *z)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    return orig::U6_AsyncUploadManager_AsyncResourceUpload(t, x, y, z);
}

static void U6_AsyncUploadManager_ScheduleAsyncCommandsInternal(AsyncUploadManager *t)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    return orig::U6_AsyncUploadManager_ScheduleAsyncCommandsInternal(t);
}

static void U6_AsyncUploadManager_CloseFile(AsyncUploadManager *t, char* s)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    return orig::U6_AsyncUploadManager_CloseFile(t, s);
}

static void U6_SyncReadRequest(AsyncReadCommand* c)
{
    char* filepath = *(char**)c;
    LOG(LL_TRACE, LCF_HACKS | LCF_FILEIO, "U6_SyncReadRequest called with file %s", filepath);
    
    return orig::U6_SyncReadRequest(c);
}

static void U6_ArchiveStorageConverter_ArchiveStorageConverter(ArchiveStorageConverter* c, IArchiveStorageConverterListener* l, bool b)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    return orig::U6_ArchiveStorageConverter_ArchiveStorageConverter(c, l, b);
}

static int U6_ArchiveStorageConverter_ProcessAccumulatedData(ArchiveStorageConverter* c)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    return orig::U6_ArchiveStorageConverter_ProcessAccumulatedData(c);
}

static int U6_AssetBundleLoadFromStreamAsyncOperation_FeedStream(AssetBundleLoadFromStreamAsyncOperation *o, void const* x, unsigned long y)
{
    LOGTRACE(LCF_HACKS | LCF_FILEIO);
    return orig::U6_AssetBundleLoadFromStreamAsyncOperation_FeedStream(o, x, y);
}

static int U6_LoadFMODSound(SoundHandle_Instance** si, char const* s, unsigned int f, SampleClip* c, unsigned int i, VFS_FileSize fs, FMOD_CREATESOUNDEXINFO* in)
{
    LOG(LL_TRACE, LCF_HACKS | LCF_SOUND, "U6_LoadFMODSound called with file %s and flags %x", s, f);
    return orig::U6_LoadFMODSound(si, s, f, c, i, fs, in);
}

#define FUNC_CASE(FUNC_ENUM, FUNC_SYMBOL) \
case FUNC_ENUM: \
    hook_patch_addr(reinterpret_cast<void*>(address), reinterpret_cast<void**>(&orig::FUNC_SYMBOL), reinterpret_cast<void*>(FUNC_SYMBOL)); \
    break;


void UnityHacks::patch(int func, uint64_t addr)
{
    setUnity();
    
    uintptr_t address = static_cast<uintptr_t>(addr);
    switch(func) {
        FUNC_CASE(UNITY_VERSION, UnityVersion_UnityVersion)

        FUNC_CASE(UNITY4_JOBSCHEDULER_AWAKE, U4_JobScheduler_AwakeIdleWorkerThreads)
        FUNC_CASE(UNITY4_JOBSCHEDULER_FETCH, U4_JobScheduler_FetchNextJob)
        FUNC_CASE(UNITY4_JOBSCHEDULER_PROCESS, U4_JobScheduler_ProcessJob)
        FUNC_CASE(UNITY4_JOBSCHEDULER_SUBMIT, U4_JobScheduler_SubmitJob)
        FUNC_CASE(UNITY4_JOBSCHEDULER_WAIT, U4_JobScheduler_WaitForGroup)

        FUNC_CASE(UNITY5_BACKGROUND_JOBQUEUE_EXECUTE, U5_BackgroundJobQueue_ExecuteMainThreadJobs)
        FUNC_CASE(UNITY5_BACKGROUND_JOBQUEUE_SCHEDULE, U5_BackgroundJobQueue_ScheduleJob)
        FUNC_CASE(UNITY5_BACKGROUND_JOBQUEUE_SCHEDULE_MAIN, U5_BackgroundJobQueue_ScheduleMainThreadJob)
        FUNC_CASE(UNITY5_JOBQUEUE_CREATE_JOB_BATCH, U5_JobQueue_CreateJobBatch)
        FUNC_CASE(UNITY5_JOBQUEUE_ENQUEUEALL, U5_JobQueue_EnqueueAll)
        FUNC_CASE(UNITY5_JOBQUEUE_ENQUEUEALL_INTERNAL, U5_JobQueue_EnqueueAllInternal)
        FUNC_CASE(UNITY5_JOBQUEUE_EXEC, U5_JobQueue_Exec)
        FUNC_CASE(UNITY5_JOBQUEUE_EXECUTE, U5_JobQueue_ExecuteOneJob)
        FUNC_CASE(UNITY5_JOBQUEUE_EXECUTE_QUEUE, U5_JobQueue_ExecuteJobFromQueue)
        FUNC_CASE(UNITY5_JOBQUEUE_MAIN_ENQUEUEALL, U5_JobQueue_MainEnqueueAll)
        FUNC_CASE(UNITY5_JOBQUEUE_POP, U5_JobQueue_Pop)
        FUNC_CASE(UNITY5_JOBQUEUE_PROCESS, U5_JobQueue_ProcessJobs)
        FUNC_CASE(UNITY5_JOBQUEUE_SCHEDULE_JOB, U5_JobQueue_ScheduleJob)
        FUNC_CASE(UNITY5_JOBQUEUE_SCHEDULE_GROUP, U5_JobQueue_ScheduleGroup)
        FUNC_CASE(UNITY5_JOBQUEUE_SCHEDULE_GROUPS, U5_JobQueue_ScheduleGroups)
        FUNC_CASE(UNITY5_JOBQUEUE_WAIT_JOB_GROUP, U5_JobQueue_WaitForJobGroup)
        
        FUNC_CASE(UNITY2K_BACKGROUND_JOBQUEUE_SCHEDULE, U2K_BackgroundJobQueue_ScheduleJobInternal)
        FUNC_CASE(UNITY2K_BACKGROUND_JOBQUEUE_SCHEDULE_MAIN, U2K_BackgroundJobQueue_ScheduleMainThreadJobInternal)
        FUNC_CASE(UNITY2K_JOBQUEUE_COMPLETE_ALL_JOBS, U2K_JobQueue_CompleteAllJobs)
        FUNC_CASE(UNITY2K_JOBQUEUE_EXEC, U2K_JobQueue_Exec)
        FUNC_CASE(UNITY2K_JOBQUEUE_EXECUTE_QUEUE, U2K_JobQueue_ExecuteJobFromQueue)
        FUNC_CASE(UNITY2K_JOBQUEUE_PROCESS, U2K_JobQueue_ProcessJobs)
        FUNC_CASE(UNITY2K_JOBQUEUE_SCHEDULE_DEPENDENCIES, U2K_JobQueue_ScheduleDependencies)
        FUNC_CASE(UNITY2K_JOBQUEUE_SCHEDULE_JOB_MULTIPLE, U2K_JobQueue_ScheduleJobMultipleDependencies)
        FUNC_CASE(UNITY2K_JOBQUEUE_SCHEDULE_GROUP_INTERNAL, U2K_JobQueue_ScheduleGroupInternal)
        FUNC_CASE(UNITY2K_JOBQUEUE_WAIT_JOB_GROUP, U2K_JobQueue_WaitForJobGroupID)

        FUNC_CASE(UNITY6_BATCH_JOB, U6_ScheduleBatchJob)
        FUNC_CASE(UNITY6_JOB_COMPLETED, U6_job_completed)
        FUNC_CASE(UNITY6_JOB_REFLECTION, U6_JobsUtility_CUSTOM_CreateJobReflectionData)
        FUNC_CASE(UNITY6_JOB_SCHEDULE, U6_JobsUtility_CUSTOM_Schedule)
        FUNC_CASE(UNITY6_JOBQUEUE_SCHEDULE_GROUPS, U6_JobQueue_ScheduleGroups)
        FUNC_CASE(UNITY6_LANE_GUTS, U6_lane_guts)
        FUNC_CASE(UNITY6_UJOB_ADD, U6_ujobs_add_to_lane_and_wake_one_thread)
        FUNC_CASE(UNITY6_UJOB_EXECUTE, U6_ujob_execute_job)
        FUNC_CASE(UNITY6_UJOB_PARTICIPATE, U6_ujob_participate)
        FUNC_CASE(UNITY6_UJOB_SCHEDULE, U6_ujob_schedule_job_internal)
        FUNC_CASE(UNITY6_UJOB_SCHEDULE_PARALLEL, U6_ujob_schedule_parallel_for_internal)
        FUNC_CASE(UNITY6_UJOB_WAIT, U6_ujob_wait_for)
        FUNC_CASE(UNITY6_WORKER_THREAD_ROUTINE, U6_worker_thread_routine)

        FUNC_CASE(UNITY6_PRELOADMANAGER_ADD, U6_PreloadManager_AddToQueue)
        FUNC_CASE(UNITY6_PRELOADMANAGER_PREPARE, U6_PreloadManager_PrepareProcessingPreloadOperation)
        FUNC_CASE(UNITY6_PRELOADMANAGER_PROCESS, U6_PreloadManager_ProcessSingleOperation)
        FUNC_CASE(UNITY6_PRELOADMANAGER_UPDATE, U6_PreloadManager_UpdatePreloading)
        FUNC_CASE(UNITY4_PRELOADMANAGER_UPDATE_STEP, U4_PreloadManager_UpdatePreloadingSingleStep)
        FUNC_CASE(UNITY6_PRELOADMANAGER_UPDATE_STEP, U6_PreloadManager_UpdatePreloadingSingleStep)
        FUNC_CASE(UNITY6_PRELOADMANAGER_WAIT, U6_PreloadManager_WaitForAllAsyncOperationsToComplete)
        FUNC_CASE(UNITY6_PRELOADMANAGER_RUN, U6_PreloadManager_Run)
        FUNC_CASE(UNITY2K_PRELOADMANAGER_PEEK, U2K_PreloadManager_PeekIntegrateQueue)
        FUNC_CASE(UNITY2K_PRELOADMANAGER_IS_LOADING, U2K_PreloadManager_IsLoadingOrQueued)

        FUNC_CASE(UNITY6_ASYNCREADMANAGER_REQUEST, U6_AsyncReadManagerThreaded_Request)
        FUNC_CASE(UNITY5_ASYNCREADMANAGER_WAIT_DONE, U5_AsyncReadManagerThreaded_WaitDone)
        FUNC_CASE(UNITY2K_ASYNCREADMANAGER_SYNC_REQUEST, U2K_AsyncReadManagerThreaded_SyncRequest)
        FUNC_CASE(UNITY6_ASYNCREADMANAGER_OPENCOMPLETE_CALLBACK, U6_AsyncReadManagerManaged_OpenCompleteCallback)
        FUNC_CASE(UNITY6_ASYNCREADMANAGER_READCOMPLETE_CALLBACK, U6_AsyncReadManagerManaged_ReadCompleteCallback)
        FUNC_CASE(UNITY6_ASYNCREADMANAGER_CLOSECOMPLETE_CALLBACK, U6_AsyncReadManagerManaged_CloseCompleteCallback)
        FUNC_CASE(UNITY6_ASYNCREADMANAGER_CLOSECACHEDCOMPLETE_CALLBACK, U6_AsyncReadManagerManaged_CloseCachedFileCompleteCallback)
        FUNC_CASE(UNITY6_ASYNCUPLOADMANAGER_ASYNC_READ_SUCCESS, U6_AsyncUploadManager_AsyncReadSuccess)
        FUNC_CASE(UNITY6_ASYNCUPLOADMANAGER_QUEUE_UPLOAD, U6_AsyncUploadManager_QueueUploadAsset)
        FUNC_CASE(UNITY6_ASYNCUPLOADMANAGER_ASYNC_RESOURCE_UPLOAD, U6_AsyncUploadManager_AsyncResourceUpload)
        FUNC_CASE(UNITY6_ASYNCUPLOADMANAGER_ASYNC_READ_CALLBACK, U6_AsyncUploadManager_AsyncReadCallbackStatic)
        FUNC_CASE(UNITY6_ASYNCUPLOADMANAGER_SCHEDULE, U6_AsyncUploadManager_ScheduleAsyncCommandsInternal)
        FUNC_CASE(UNITY6_ASYNCUPLOADMANAGER_CLOSE, U6_AsyncUploadManager_CloseFile)
        FUNC_CASE(UNITY6_SIGNAL_CALLBACK, U6_SignalCallback)
        FUNC_CASE(UNITY6_SYNC_READ, U6_SyncReadRequest)

        FUNC_CASE(UNITY6_ARCHIVESTORAGECONVERTER_CONSTRUCTOR, U6_ArchiveStorageConverter_ArchiveStorageConverter)
        FUNC_CASE(UNITY6_ARCHIVESTORAGECONVERTER_PROCESS_ACCUMULATED, U6_ArchiveStorageConverter_ProcessAccumulatedData)
        FUNC_CASE(UNITY6_ASSETBUNDLELOAD_FEEDSTREAM, U6_AssetBundleLoadFromStreamAsyncOperation_FeedStream)

        FUNC_CASE(UNITY6_LOAD_FMOD_SOUND, U6_LoadFMODSound)
    }
}

}
