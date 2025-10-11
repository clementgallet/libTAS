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

#ifndef LIBTAS_UNITY_FUNCS_H_INCLUDED
#define LIBTAS_UNITY_FUNCS_H_INCLUDED

/* List of message identification values that is sent from/to the game */
enum {
    UNITY6_UJOB_SCHEDULE, // ujob_schedule_job_internal(ujob_control_t*, ujob_handle_t, unsigned int)
    UNITY6_UJOB_SCHEDULE_PARALLEL, // ujob_schedule_parallel_for_internal(ujob_control_t*, JobsCallbackFunctions&, void*, WorkStealingRange*, unsigned int, unsigned int, ujob_handle_t const*, int, unsigned char)
    UNITY6_UJOB_WAIT, // ujob_wait_all(ujob_control_t*, int)
    UNITY6_UJOB_KICK, // ujob_kick_jobs(ujob_control_t*)
    UNITY6_JOB_COMPLETED, // job_completed(ujob_control_t*, ujob_lane_t*, ujob_job_t*, ujob_handle_t)
    UNITY6_UJOB_EXECUTE, // ujob_execute_job(ujob_control_t*, ujob_lane_t*, ujob_job_t*, ujob_handle_t, unsigned int)
    UNITY6_UJOB_ADD, // ujobs_add_to_lane_and_wake_one_thread(ujob_control_t*, ujob_job_t*, ujob_lane_t*)
    UNITY6_UJOB_PARTICIPATE, // ujob_participate(ujob_control_t*, ujob_handle_t, ujob_job_t*&, unsigned int&, ujob_dependency_chain const*)
    UNITY6_LANE_GUTS, // lane_guts(ujob_control_t*, ujob_lane_t*, int, int, ujob_dependency_chain const*)
    UNITY6_JOB_REFLECTION, // JobsUtility_CUSTOM_CreateJobReflectionData(ScriptingBackendNativeObjectPtrOpaque*, ScriptingBackendNativeObjectPtrOpaque*, ScriptingBackendNativeObjectPtrOpaque*, ScriptingBackendNativeObjectPtrOpaque*, ScriptingBackendNativeObjectPtrOpaque*)
    UNITY6_JOB_SCHEDULE, // JobsUtility_CUSTOM_Schedule(JobScheduleParameters&, JobFence&)
    UNITY6_BATCH_JOB, // ScheduleBatchJob(void*, ujob_handle_t)
    UNITY6_JOBQUEUE_SCHEDULE_GROUPS, // JobQueue::ScheduleGroups(JobBatchHandles*, int)
    UNITY6_WORKER_THREAD_ROUTINE, // worker_thread_routine(void*)
    UNITY5_JOBQUEUE_SCHEDULE_JOB, // JobQueue::ScheduleJob(void (*)(void*), void*, JobGroupID, JobQueue::JobQueuePriority)
    UNITY2K_JOBQUEUE_COMPLETE_ALL_JOBS, // JobQueue::CompleteAllJobs()
    UNITY2K_JOBQUEUE_SCHEDULE_JOB_MULTIPLE, // JobQueue::ScheduleJobMultipleDependencies(void (*)(void*), void*, JobGroupID*, int, MemLabelId)
    UNITY5_JOBQUEUE_CREATE_JOB_BATCH, // JobQueue::CreateJobBatch(void (*)(void*), void*, JobGroupID, JobGroup*)
    UNITY5_JOBQUEUE_SCHEDULE_GROUPS, // JobQueue::ScheduleGroups(JobGroup*, JobGroup*)
    UNITY2K_JOBQUEUE_WAIT_JOB_GROUP, // JobQueue::WaitForJobGroupID(JobGroupID, JobQueue::JobQueueWorkStealMode)
    UNITY5_JOBQUEUE_EXECUTE, // JobQueue::ExecuteOneJob()
    UNITY5_JOBQUEUE_SCHEDULE_GROUP, // JobQueue::ScheduleGroup(JobGroup*, JobQueue::JobQueuePriority)
    UNITY2K_JOBQUEUE_SCHEDULE_GROUP_INTERNAL, // JobQueue::ScheduleGroupInternal(JobGroup*, JobQueue::JobQueuePriority, bool)
    UNITY2K_JOBQUEUE_PROCESS, // JobQueue::ProcessJobs(JobQueue::ThreadInfo*, void*)
    UNITY2K_JOBQUEUE_EXEC, // JobQueue::Exec(JobInfo*, long long, int, bool)
    UNITY2K_JOBQUEUE_EXECUTE_QUEUE, // JobQueue::ExecuteJobFromQueue(bool)
    UNITY2K_JOBQUEUE_SCHEDULE_DEPENDENCIES, // JobQueue::ScheduleDependencies(JobGroupID&, JobInfo*, JobInfo*, bool)
    UNITY2K_BACKGROUND_JOBQUEUE_SCHEDULE, // BackgroundJobQueue::ScheduleJobInternal(void (*)(void*), void*, BackgroundJobQueue::JobFence const&, JobQueue::JobQueuePriority)
    UNITY2K_BACKGROUND_JOBQUEUE_SCHEDULE_MAIN, // BackgroundJobQueue::ScheduleMainThreadJobInternal(void (*)(void*), void*)
    UNITY5_BACKGROUND_JOBQUEUE_EXECUTE, // BackgroundJobQueue::ExecuteMainThreadJobs()
    UNITY5_BACKGROUND_JOBQUEUE_SCHEDULE, //  BackgroundJobQueue::ScheduleJob(void (*)(void*), void*)
    UNITY5_JOBQUEUE_EXECUTE_QUEUE, // JobQueue::ExecuteJobFromQueue()
    UNITY5_JOBQUEUE_PROCESS, // JobQueue::ProcessJobs(void*)
    UNITY5_JOBQUEUE_EXEC, // JobQueue::Exec(JobInfo*, long long, int)
    UNITY5_JOBQUEUE_ENQUEUEALL, // JobQueue::EnqueueAll(JobGroup*, JobGroup*)
    UNITY5_JOBQUEUE_POP, // JobQueue::Pop(JobGroupID)
    UNITY5_JOBQUEUE_ENQUEUEALL_INTERNAL, // JobQueue::EnqueueAllInternal(JobGroup*, JobGroup*, AtomicQueue*, int*)
    UNITY5_JOBQUEUE_MAIN_ENQUEUEALL, // JobQueue::MainEnqueueAll(JobGroup*, JobGroup*)
    UNITY5_BACKGROUND_JOBQUEUE_SCHEDULE_MAIN, // BackgroundJobQueue::ScheduleMainThreadJob(void (*)(void*), void*)
    UNITY5_JOBQUEUE_WAIT_JOB_GROUP, // JobQueue::WaitForJobGroup(JobGroupID, bool)
    UNITY4_JOBSCHEDULER_FETCH, // JobScheduler::FetchNextJob(int&)
    UNITY4_JOBSCHEDULER_PROCESS, // JobScheduler::ProcessJob(JobInfo&, int)
    UNITY4_JOBSCHEDULER_WAIT, // JobScheduler::WaitForGroup(int)
    UNITY4_JOBSCHEDULER_AWAKE, // JobScheduler::AwakeIdleWorkerThreads(int)
    UNITY4_JOBSCHEDULER_SUBMIT, // JobScheduler::SubmitJob(int, void* (*)(void*), void*, void* volatile*)
    UNITY_FUNCS_LEN
};

struct usymbol_t {
    int id;
    const char* name;
    const char* symbol;
    const char* signature32;
    const char* signature64;
};

usymbol_t UNITY_SYMBOLS[] = {
    {
        UNITY6_UJOB_SCHEDULE,
        "ujob_schedule_job_internal",
        "_ZL26ujob_schedule_job_internalP14ujob_control_t13ujob_handle_tj",
        "", 
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 28 48 89 F3 49 89 FC 48 89 F1"
    },
    {
        UNITY6_UJOB_SCHEDULE_PARALLEL,
        "ujob_schedule_parallel_for_internal",
        "_Z35ujob_schedule_parallel_for_internalP14ujob_control_tR21JobsCallbackFunctionsPvP17WorkStealingRangejjPK13ujob_handle_tih", 
        "", 
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 68 4D 89 CE 45 89 C4"
    },
    {
        UNITY6_UJOB_WAIT,
        "ujob_wait_all",
        "_Z13ujob_wait_allP14ujob_control_ti",
        "",
        ""
    },
    {
        UNITY6_UJOB_KICK,
        "ujob_kick_jobs",
        "_Z14ujob_kick_jobsP14ujob_control_t",
        "",
        ""
    },
    {
        UNITY6_JOB_COMPLETED,
        "job_completed",
        "_ZL13job_completedP14ujob_control_tP11ujob_lane_tP10ujob_job_t13ujob_handle_t",
        "",
        "55 41 57 41 56 41 55 41 54 53 48 81 EC 38 20 00 00 49 89 CC"
    },
    {
        UNITY6_UJOB_EXECUTE,
        "ujob_execute_job",
        "_ZL16ujob_execute_jobP14ujob_control_tP11ujob_lane_tP10ujob_job_t13ujob_handle_tj",
        "",
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 48 45 89 C3 48 89 CB"
    },
    {
        UNITY6_UJOB_ADD,
        "ujobs_add_to_lane_and_wake_one_thread",
        "_ZL37ujobs_add_to_lane_and_wake_one_threadP14ujob_control_tP10ujob_job_tP11ujob_lane_t",
        "",
        "41 57 41 56 53 48 89 D3 49 89 FE 48 8B 46 20"
    },
    {
        UNITY6_UJOB_PARTICIPATE,
        "ujob_participate",
        "_Z16ujob_participateP14ujob_control_t13ujob_handle_tRP10ujob_job_tRjPK21ujob_dependency_chain",
        "",
        "55 41 57 41 56 41 55 41 54 53 4C 89 44 24 F8 48 89 4C 24 E8 49 89 F2"
    },
    {
        UNITY6_LANE_GUTS,
        "lane_guts",
        "_ZL9lane_gutsP14ujob_control_tP11ujob_lane_tiiPK21ujob_dependency_chain",
        "",
        "55 41 57 41 56 41 55 41 54 53 48 81 EC 88 00 00 00 4C 89 44 24 58 41 89 CD"
    },
    {
        UNITY6_JOB_REFLECTION,
        "JobsUtility_CUSTOM_CreateJobReflectionData",
        "_Z42JobsUtility_CUSTOM_CreateJobReflectionDataP37ScriptingBackendNativeObjectPtrOpaqueS0_S0_S0_S0_",
        "",
        ""
    },
    {
        UNITY6_JOB_SCHEDULE,
        "JobsUtility_CUSTOM_Schedule",
        "_Z27JobsUtility_CUSTOM_ScheduleR21JobScheduleParametersR8JobFence",
        "",
        ""
    },
    {
        UNITY6_BATCH_JOB,
        "ScheduleBatchJob",
        "_ZL16ScheduleBatchJobPv13ujob_handle_t",
        "",
        ""
    },
    {
        UNITY6_JOBQUEUE_SCHEDULE_GROUPS,
        "JobQueue::ScheduleGroups",
        "_ZN8JobQueue14ScheduleGroupsEP15JobBatchHandlesi",
        "",
        ""
    },
    {
        UNITY6_WORKER_THREAD_ROUTINE,
        "worker_thread_routine",
        "_ZL21worker_thread_routinePv",
        "",
        "41 57 41 56 53 49 89 FE 48 8B 5F 08 48 63 07"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_JOB,
        "JobQueue::ScheduleJob",
        "_ZN8JobQueue11ScheduleJobEPFvPvES0_10JobGroupIDNS_16JobQueuePriorityE",
        "",
        // Signature for Unity 2020
        "55 41 57 41 56 41 54 53 45 89 CE 49 89 D7 49 89 F4 48 89 FB BE 01 00 00 00"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_JOB,
        "JobQueue::ScheduleJob",
        "",
        "",
        // Signature for Unity 2018
        "55 44 89 CD 53 48 89 fb 48 83 ec 08 e8 ?? ?? ?? ?? 89 ea 48 89 df 48 89 c6 e8"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_JOB,
        "JobQueue::ScheduleJob",
        "",
        "",
        // Signature for Unity 5
        "48 89 5C 24 D8 48 89 6C 24 E0 48 89 F5 4C 89 64 24 E8 4C 89 6C 24 F0 49 89 D4 4c 89 74 24 f8 48 83 EC 68 48 89 CA 44 89 44 24 18 48 89 4C 24 10 BE 01 00 00 00"
    },
    {
        UNITY2K_JOBQUEUE_COMPLETE_ALL_JOBS,
        "JobQueue::CompleteAllJobs",
        "_ZN8JobQueue15CompleteAllJobsEv",
        "",
        // Signature for Unity 2020
        "55 41 56 53 49 89 FE 66 0F 1F 84 00 00 00 00 00 49 8B 86 38 01 00 00"
    },
    {
        UNITY2K_JOBQUEUE_SCHEDULE_JOB_MULTIPLE,
        "JobQueue::ScheduleJobMultipleDependencies",
        "_ZN8JobQueue31ScheduleJobMultipleDependenciesEPFvPvES0_P10JobGroupIDi10MemLabelId",
        "",
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 38 45 89 CD 44 89 C3 48 89 4C 24 18 48 89 D5"
    },
    {
        UNITY5_JOBQUEUE_CREATE_JOB_BATCH,
        "JobQueue::CreateJobBatch",
        "_ZN8JobQueue14CreateJobBatchEPFvPvES0_10JobGroupIDP8JobGroup",
        "",
        "41 57 41 56 41 55 41 54 53 4D 89 CE 49 89 D7 49 89 F4 49 89 FD BE 01 00 00 00"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_GROUPS,
        "JobQueue::ScheduleGroups",
        "_ZN8JobQueue14ScheduleGroupsEP8JobGroupS1_",
        "",
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 18 49 89 FF 8A 87 5A 01 00 00 45 31 E4 88 44 24 0F"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_GROUPS,
        "JobQueue::ScheduleGroups",
        "",
        "",
        // Signature for Unity 5
        "41 55 41 54 49 89 FC 55 53 31 DB 48 83 EC 28 48 85 F6 48 8B 7F 08 74 ?? 48 89 F1 EB ?? ?? ?? ?? 48 8b 41 30"
    },
    {
        UNITY2K_JOBQUEUE_WAIT_JOB_GROUP,
        "JobQueue::WaitForJobGroupID",
        "_ZN8JobQueue17WaitForJobGroupIDE10JobGroupIDNS_21JobQueueWorkStealModeE",
        "",
        // Signature for Unity 6
        "55 41 57 41 56 41 55 41 54 53 48 81 EC C8 00 00 00 49 89 F7 C7 44 24 2C 00 00 00 00"
    },
    {
        UNITY2K_JOBQUEUE_WAIT_JOB_GROUP,
        "JobQueue::WaitForJobGroupID",
        "",
        "",
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 48 48 89 54 24 30 48 85 F6 0F 84"
    },
    {
        UNITY5_JOBQUEUE_EXECUTE,
        "JobQueue::ExecuteOneJob",
        "_ZN8JobQueue13ExecuteOneJobEv",
        "",
        // Signature for Unity 2020
        "55 41 56 53 48 89 FB 40 8A AF 5A 01 00 00 48 8B 3F"
    },
    {
        UNITY5_JOBQUEUE_EXECUTE,
        "JobQueue::ExecuteOneJob",
        "",
        "",
        // Signature for Unity 2018
        "53 48 89 FB E8 ?? ?? ?? ?? 84 C0 74 ?? B8 01 00 00 00 5B C3 0F 1F 40 00 48 89 df 5B E9"
    },
    {
        UNITY5_JOBQUEUE_EXECUTE,
        "JobQueue::ExecuteOneJob",
        "",
        "",
        // Signature for Unity 5
        "48 89 5C 24 F0 48 89 6C 24 F8 48 89 FB 48 83 EC 18 48 8B 3F E8 ?? ?? ?? ?? 48 85 C0"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_GROUP,
        "JobQueue::ScheduleGroup",
        "_ZN8JobQueue13ScheduleGroupEP8JobGroupNS_16JobQueuePriorityE",
        "",
        // Signature for Unity 2018
        "41 57 41 56 41 55 49 89 FD 48 89 f7 41 54 49 89 F4 55 89 D5 53 48 83 ec 58"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_GROUP,
        "JobQueue::ScheduleGroup",
        "",
        "",
        // Signature for Unity 5
        "41 56 41 55 41 89 D5 41 54 49 89 fc 48 89 f7 55 53 48 89 f3 48 83 c4 80"
    },
    {
        UNITY2K_JOBQUEUE_SCHEDULE_GROUP_INTERNAL,
        "JobQueue::ScheduleGroupInternal",
        "_ZN8JobQueue21ScheduleGroupInternalEP8JobGroupNS_16JobQueuePriorityEb",
        "",
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 18 41 89 CC 41 89 D5 49 89 F6 48 89 FB 48 89 F7"
    },
    {
        UNITY2K_JOBQUEUE_PROCESS,
        "JobQueue::ProcessJobs",
        "_ZN8JobQueue11ProcessJobsEPNS_10ThreadInfoEPv",
        "",
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 50 48 89 FB 48 8B 3D"
    },
    {
        UNITY2K_JOBQUEUE_EXEC,
        "JobQueue::Exec",
        "_ZN8JobQueue4ExecEP7JobInfoxib",
        "",
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 18 89 CB 49 89 F6 49 89 FD"
    },
    {
        UNITY2K_JOBQUEUE_EXECUTE_QUEUE,
        "JobQueue::ExecuteJobFromQueue",
        "_ZN8JobQueue19ExecuteJobFromQueueEb",
        "",
        // Signature for Unity 2020
        "55 41 57 41 56 53 48 83 EC 18 89 F5 49 89 FF 48 8B 7F 08"
    },
    {
        UNITY2K_JOBQUEUE_EXECUTE_QUEUE,
        "JobQueue::ExecuteJobFromQueue",
        "",
        "",
        // Signature for Unity 2018
        "41 55 49 89 FD 41 54 55 53 48 83 EC 18 48 8B 7F 08 E8 ?? ?? ?? ?? 49 89 C4"
    },
    {
        UNITY2K_JOBQUEUE_SCHEDULE_DEPENDENCIES,
        "JobQueue::ScheduleDependencies",
        "_ZN8JobQueue20ScheduleDependenciesER10JobGroupIDP7JobInfoS3_b",
        "",
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 28 48 89 D3 49 89 FE 48 8B 2E 48 85 ED"
    },
    {
        UNITY2K_BACKGROUND_JOBQUEUE_SCHEDULE,
        "BackgroundJobQueue::ScheduleJobInternal",
        "_ZN18BackgroundJobQueue19ScheduleJobInternalEPFvPvES0_RKNS_8JobFenceEN8JobQueue16JobQueuePriorityE",
        "",
        ""
    },
    {
        UNITY2K_BACKGROUND_JOBQUEUE_SCHEDULE_MAIN,
        "BackgroundJobQueue::ScheduleMainThreadJobInternal",
        "_ZN18BackgroundJobQueue29ScheduleMainThreadJobInternalEPFvPvES0_",
        "",
        ""
    },
    {
        UNITY5_BACKGROUND_JOBQUEUE_EXECUTE,
        "BackgroundJobQueue::ExecuteMainThreadJobs",
        "_ZN18BackgroundJobQueue21ExecuteMainThreadJobsEv",
        "",
        ""
    },
    {
        UNITY5_BACKGROUND_JOBQUEUE_SCHEDULE,
        "BackgroundJobQueue::ScheduleJob",
        "_ZN18BackgroundJobQueue11ScheduleJobEPFvPvES0_",
        "",
        ""
    },
    {
        UNITY5_JOBQUEUE_EXECUTE_QUEUE,
        "JobQueue::ExecuteJobFromQueue",
        "_ZN8JobQueue19ExecuteJobFromQueueEv",
        "",
        "41 54 55 48 89 FD 53 48 83 EC 10 48 8B 7F 08 E8"
    },
    {
        UNITY5_JOBQUEUE_PROCESS,
        "JobQueue::ProcessJobs",
        "_ZN8JobQueue11ProcessJobsEPv",
        "",
        // Signature for Unity 2018
        "41 57 41 56 41 55 41 54 55 53 48 89 FB 48 83 EC 48 48 8B 35 ?? ?? ?? ?? 48 8b 3d"
    },
    {
        UNITY5_JOBQUEUE_PROCESS,
        "JobQueue::ProcessJobs",
        "",
        "",
        // Signature for Unity 5
        "41 55 41 54 55 48 8d 6f 74 53 48 89 fb 48 83 ec 28 f0 83 45 00 01 4c 8d 67 50"
    },
    {
        UNITY5_JOBQUEUE_EXEC,
        "JobQueue::Exec",
        "_ZN8JobQueue4ExecEP7JobInfoxi",
        "",
        // Signature for Unity 2018
        "41 57 41 56 49 89 D6 41 55 41 54 41 89 CC 55 48 89 FD 53 48 89 F3 48 83 ec 18 f0 83 6f 68 01"
    },
    {
        UNITY5_JOBQUEUE_EXEC,
        "JobQueue::Exec",
        "",
        "",
        // Signature for Unity 5
        "41 57 48 8D 47 70 41 56 41 89 CE 41 55 49 89 D5 41 54 55 48 89 F5 53"
    },
    {
        UNITY5_JOBQUEUE_ENQUEUEALL,
        "JobQueue::EnqueueAll",
        "_ZN8JobQueue10EnqueueAllEP8JobGroupS1_",
        "",
        ""
    },
    {
        UNITY5_JOBQUEUE_POP,
        "JobQueue::Pop",
        "_ZN8JobQueue3PopE10JobGroupID",
        "",
        ""
    },
    {
        UNITY5_JOBQUEUE_ENQUEUEALL_INTERNAL,
        "JobQueue::EnqueueAllInternal",
        "_ZN8JobQueue18EnqueueAllInternalEP8JobGroupS1_P11AtomicQueuePi",
        "",
        ""
    },
    {
        UNITY5_JOBQUEUE_MAIN_ENQUEUEALL,
        "JobQueue::MainEnqueueAll",
        "_ZN8JobQueue14MainEnqueueAllEP8JobGroupS1_",
        "",
        ""
    },
    {
        UNITY5_BACKGROUND_JOBQUEUE_SCHEDULE_MAIN,
        "BackgroundJobQueue::ScheduleMainThreadJob",
        "_ZN18BackgroundJobQueue21ScheduleMainThreadJobEPFvPvES0_",
        "",
        ""
    },
    {
        UNITY5_JOBQUEUE_WAIT_JOB_GROUP,
        "JobQueue::WaitForJobGroup",
        "_ZN8JobQueue15WaitForJobGroupE10JobGroupIDb",
        "",
        "41 57 41 89 cf 41 56 41 55 41 89 d5 41 54 55 48 89 f5 53 48 89 fb 48 83 ec 78 89 54 24 38"
    },
    {
        UNITY4_JOBSCHEDULER_FETCH,
        "JobScheduler::FetchNextJob",
        "_ZN12JobScheduler12FetchNextJobERi",
        "",
        "41 54 55 48 89 F5 53 48 89 FB 48 83 EC 30 48 63 06 8B 57 7C 39 D0"
    },
    {
        UNITY4_JOBSCHEDULER_PROCESS,
        "JobScheduler::ProcessJob",
        "_ZN12JobScheduler10ProcessJobER7JobInfoi",
        "",
        "48 89 5C 24 E8 48 89 F3 48 89 6C 24 F0 4C 89 64 24 F8 89 D5 48 83 EC 48"
    },
    {
        UNITY4_JOBSCHEDULER_WAIT,
        "JobScheduler::WaitForGroup",
        "_ZN12JobScheduler12WaitForGroupEi",
        "",
        "41 55 49 89 FD 41 54 55 53 48 83 EC 38 39 77 08 0F 8E"
    },
    {
        UNITY4_JOBSCHEDULER_AWAKE,
        "JobScheduler::AwakeIdleWorkerThreads",
        "_ZN12JobScheduler22AwakeIdleWorkerThreadsEi",
        "",
        ""
    },
    {
        UNITY4_JOBSCHEDULER_SUBMIT,
        "JobScheduler::SubmitJob",
        "_ZN12JobScheduler9SubmitJobEiPFPvS0_ES0_PVS0_",
        "",
        "48 83 EC 28 44 8b 4f 0c 45 85 c9 0f 8e ?? ?? ?? ?? 39 77 08"
    },
    {
        UNITY_FUNCS_LEN,
        "",
        "",
        "",
        ""
    },
};

#endif
