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
#include "hookpatch.h"
#ifdef __unix__
#include "checkpoint/ProcSelfMaps.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "checkpoint/MachVmMaps.h"
#endif
#include "checkpoint/MemArea.h"
#include "checkpoint/ThreadManager.h"
#include "checkpoint/ThreadInfo.h"
#include "../shared/unity_funcs.h"

#include <unistd.h>
#include <memory>
#include <vector>
#include <map>
#include <condition_variable>
#include <sys/mman.h> // PROT_READ, PROT_WRITE, etc.

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
typedef void WorkStealingRange;
typedef void JobsCallbackFunctions;
typedef void ScriptingBackendNativeObjectPtrOpaque;
class JobScheduleParameters;
class JobFence;

struct JobGroupID {
    JobGroup* group;
    int tag;
};

namespace orig {
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
    long (*U6_ujob_schedule_parallel_for_internal)(ujob_control_t* x, JobsCallbackFunctions* y, void* z, WorkStealingRange* a, unsigned int b, unsigned long c, ujob_handle_t const* d, long e) = nullptr;
    void (*U6_ujobs_add_to_lane_and_wake_one_thread)(ujob_control_t*, ujob_job_t*, ujob_lane_t*) = nullptr;
    void (*U6_worker_thread_routine)(void* x) = nullptr;
}

#include <signal.h>

static long U4_JobScheduler_FetchNextJob(JobScheduler* t, int* x)
{
    LOGTRACE(LCF_HACKS);
    ThreadInfo* thread = ThreadManager::getCurrentThread();
    thread->unityThread = true;
    
    /* Return 0 to prevent worker threads from executing a job */
    return 0;
    // return orig::U4_JobScheduler_FetchNextJob(t, x);
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
    return 0;
    // return orig::U5_JobQueue_ExecuteJobFromQueue(t);
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

    /* Calling U5_JobQueue_ProcessJobs() in worker threads may call
     * U5_JobQueue_Exec() directly without fetching a job queue with 
     * U5_JobQueue_ExecuteJobFromQueue(), so we wait indefinitively here.
     * To call jobs pushed in this worker thread, we will use the WorkerSteal
     * feature elsewhere.
     */
    if (!ThreadManager::isMainThread()) {
        while (!Global::is_exiting) {
            sleep(1);
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

    /* Immediatly wait for the job */
    if (orig::U5_JobQueue_WaitForJobGroup)
        orig::U5_JobQueue_WaitForJobGroup(t, j.group, j.tag, true);

    return j;
    // return orig::U5_JobQueue_ScheduleGroup(t, x, y);
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
    return 0;
    // return orig::U2K_JobQueue_ExecuteJobFromQueue(t, x);
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
    // return orig::U2K_JobQueue_ScheduleDependencies(t, x, y, z, a);
    return orig::U2K_JobQueue_ScheduleDependencies(t, x, y, z, true);
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
    LOGTRACE(LCF_HACKS);
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
    LOGTRACE(LCF_HACKS);
    return orig::U6_lane_guts(x, y, z, a, b);
}

static long U6_ScheduleBatchJob(void* x, ujob_handle_t y)
{
    LOGTRACE(LCF_HACKS);
    return orig::U6_ScheduleBatchJob(x, y);
}

static void U6_ujobs_add_to_lane_and_wake_one_thread(ujob_control_t* x, ujob_job_t* y, ujob_lane_t* z)
{
    LOGTRACE(LCF_HACKS);
    orig::U6_ujobs_add_to_lane_and_wake_one_thread(x, y, z);
}

static void U6_ujob_execute_job(ujob_control_t* x, ujob_lane_t* y, ujob_job_t* z, ujob_handle_t a, unsigned int b)
{
    LOGTRACE(LCF_HACKS);
    ThreadInfo* th = ThreadManager::getCurrentThread();
    th->unityJobCount++;
    return orig::U6_ujob_execute_job(x, y, z, a, b);
}

static void U6_ujob_participate(ujob_control_t* x, ujob_handle_t y, ujob_job_t** z, int* a, ujob_dependency_chain const* b)
{
    LOGTRACE(LCF_HACKS);
    orig::U6_ujob_participate(x, y, z, a, b);
}

static unsigned long U6_ujob_schedule_job_internal(ujob_control_t* x, ujob_handle_t y, unsigned int z)
{
    LOGTRACE(LCF_HACKS);
    unsigned long ret = orig::U6_ujob_schedule_job_internal(x, y, z);
    
    if (orig::U2K_JobQueue_WaitForJobGroupID)
        orig::U2K_JobQueue_WaitForJobGroupID(reinterpret_cast<JobQueue*>(x), reinterpret_cast<JobGroup*>(y), 0, true);

    return ret;
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
static long U6_ujob_schedule_parallel_for_internal(ujob_control_t* x, JobsCallbackFunctions* y, void* z, WorkStealingRange* a, unsigned int b, unsigned long c, ujob_handle_t const* d, long e)
{
    LOGTRACE(LCF_HACKS);
    return orig::U6_ujob_schedule_parallel_for_internal(x, y, z, a, b, c, d, e);
}

static void U6_worker_thread_routine(void* x)
{
    LOGTRACE(LCF_HACKS);
    ThreadInfo* thread = ThreadManager::getCurrentThread();
    thread->unityThread = true;
    return orig::U6_worker_thread_routine(x);
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
        FUNC_CASE(UNITY6_WORKER_THREAD_ROUTINE, U6_worker_thread_routine)
    }
}

}
