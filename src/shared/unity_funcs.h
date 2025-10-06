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
    UNITY_FUTEX_WAIT, // UnityClassic::Baselib_SystemFutex_Wait()
    UNITY6_UJOB_SCHEDULE, // ujob_schedule_job_internal(ujob_control_t*, ujob_handle_t, unsigned int)
    UNITY6_UJOB_SCHEDULE_PARALLEL, // ujob_schedule_parallel_for_internal(ujob_control_t*, JobsCallbackFunctions&, void*, WorkStealingRange*, unsigned int, unsigned int, ujob_handle_t const*, int, unsigned char)
    UNITY6_UJOB_WAIT, // ujob_wait_all(ujob_control_t*, int)
    UNITY6_UJOB_KICK, // ujob_kick_jobs(ujob_control_t*)
    UNITY6_JOB_COMPLETED, // job_completed(ujob_control_t*, ujob_lane_t*, ujob_job_t*, ujob_handle_t)
    UNITY6_UJOB_EXECUTE, // ujob_execute_job(ujob_control_t*, ujob_lane_t*, ujob_job_t*, ujob_handle_t, unsigned int)
    UNITY6_UJOB_ADD, // ujobs_add_to_lane_and_wake_one_thread(ujob_control_t*, ujob_job_t*, ujob_lane_t*)
    UNITY6_UJOB_PARTICIPATE, // ujob_participate(ujob_control_t*, ujob_handle_t, ujob_job_t*&, unsigned int&, ujob_dependency_chain const*)
    UNITY6_JOB_REFLECTION, // JobsUtility_CUSTOM_CreateJobReflectionData(ScriptingBackendNativeObjectPtrOpaque*, ScriptingBackendNativeObjectPtrOpaque*, ScriptingBackendNativeObjectPtrOpaque*, ScriptingBackendNativeObjectPtrOpaque*, ScriptingBackendNativeObjectPtrOpaque*)
    UNITY6_JOB_SCHEDULE, // JobsUtility_CUSTOM_Schedule(JobScheduleParameters&, JobFence&)
    UNITY6_BATCH_JOB, // ScheduleBatchJob(void*, ujob_handle_t)
    UNITY6_JOBQUEUE_SCHEDULE_GROUPS, // JobQueue::ScheduleGroups(JobBatchHandles*, int)
    UNITY6_WORKER_THREAD_ROUTINE, // worker_thread_routine(void*)
    UNITY_JOBQUEUE_SCHEDULE_JOB, // JobQueue::ScheduleJob(void (*)(void*), void*, JobGroupID, JobQueue::JobQueuePriority)
    UNITY_JOBQUEUE_COMPLETE_ALL_JOBS, // JobQueue::CompleteAllJobs()
    UNITY_JOBQUEUE_SCHEDULE_JOB_MULTIPLE, // JobQueue::ScheduleJobMultipleDependencies(void (*)(void*), void*, JobGroupID*, int, MemLabelId)
    UNITY_JOBQUEUE_CREATE_JOB_BATCH, // JobQueue::CreateJobBatch(void (*)(void*), void*, JobGroupID, JobGroup*)
    UNITY_JOBQUEUE_SCHEDULE_GROUPS, // JobQueue::ScheduleGroups(JobGroup*, JobGroup*)
    UNITY_JOBQUEUE_WAIT_JOB_GROUP, // JobQueue::WaitForJobGroupID(JobGroupID, JobQueue::JobQueueWorkStealMode)
    UNITY_JOBQUEUE_EXECUTE, // JobQueue::ExecuteOneJob()
    UNITY_JOBQUEUE_SCHEDULE_GROUP, // JobQueue::ScheduleGroup(JobGroup*, JobQueue::JobQueuePriority)
    UNITY_JOBQUEUE_SCHEDULE_GROUP_INTERNAL, // JobQueue::ScheduleGroupInternal(JobGroup*, JobQueue::JobQueuePriority, bool)
    UNITY_JOBQUEUE_PROCESS, // JobQueue::ProcessJobs(JobQueue::ThreadInfo*, void*)
    UNITY_JOBQUEUE_EXEC, // JobQueue::Exec(JobInfo*, long long, int, bool)
    UNITY_JOBQUEUE_EXECUTE_QUEUE, // JobQueue::ExecuteJobFromQueue(bool)
    UNITY_JOBQUEUE_SCHEDULE_DEPENDENCIES, // JobQueue::ScheduleDependencies(JobGroupID&, JobInfo*, JobInfo*, bool)
    UNITY_BACKGROUND_JOBQUEUE_SCHEDULE, // BackgroundJobQueue::ScheduleJobInternal(void (*)(void*), void*, BackgroundJobQueue::JobFence const&, JobQueue::JobQueuePriority)
    UNITY_BACKGROUND_JOBQUEUE_SCHEDULE_MAIN, // BackgroundJobQueue::ScheduleMainThreadJobInternal(void (*)(void*), void*)
    UNITY_BACKGROUND_JOBQUEUE_EXECUTE, // BackgroundJobQueue::ExecuteMainThreadJobs()
    UNITY_FUNCS_LEN
};

const char* const UNITY_SYMBOLS[] = {
    "_ZN12UnityClassic24Baselib_SystemFutex_WaitEPiij",
    "_ZL26ujob_schedule_job_internalP14ujob_control_t13ujob_handle_tj",
    "_Z35ujob_schedule_parallel_for_internalP14ujob_control_tR21JobsCallbackFunctionsPvP17WorkStealingRangejjPK13ujob_handle_tih",
    "_Z13ujob_wait_allP14ujob_control_ti",
    "_Z14ujob_kick_jobsP14ujob_control_t",
    "_ZL13job_completedP14ujob_control_tP11ujob_lane_tP10ujob_job_t13ujob_handle_t",
    "_ZL16ujob_execute_jobP14ujob_control_tP11ujob_lane_tP10ujob_job_t13ujob_handle_tj",
    "_ZL37ujobs_add_to_lane_and_wake_one_threadP14ujob_control_tP10ujob_job_tP11ujob_lane_t",
    "_Z16ujob_participateP14ujob_control_t13ujob_handle_tRP10ujob_job_tRjPK21ujob_dependency_chain",
    "_Z42JobsUtility_CUSTOM_CreateJobReflectionDataP37ScriptingBackendNativeObjectPtrOpaqueS0_S0_S0_S0_",
    "_Z27JobsUtility_CUSTOM_ScheduleR21JobScheduleParametersR8JobFence",
    "_ZL16ScheduleBatchJobPv13ujob_handle_t",
    "_ZN8JobQueue14ScheduleGroupsEP15JobBatchHandlesi",
    "_ZL21worker_thread_routinePv",
    "_ZN8JobQueue11ScheduleJobEPFvPvES0_10JobGroupIDNS_16JobQueuePriorityE",
    "_ZN8JobQueue15CompleteAllJobsEv",
    "_ZN8JobQueue31ScheduleJobMultipleDependenciesEPFvPvES0_P10JobGroupIDi10MemLabelId",
    "_ZN8JobQueue14CreateJobBatchEPFvPvES0_10JobGroupIDP8JobGroup",
    "_ZN8JobQueue14ScheduleGroupsEP8JobGroupS1_",
    "_ZN8JobQueue17WaitForJobGroupIDE10JobGroupIDNS_21JobQueueWorkStealModeE",
    "_ZN8JobQueue13ExecuteOneJobEv",
    "_ZN8JobQueue13ScheduleGroupEP8JobGroupNS_16JobQueuePriorityE",
    "_ZN8JobQueue21ScheduleGroupInternalEP8JobGroupNS_16JobQueuePriorityEb",
    "_ZN8JobQueue11ProcessJobsEPNS_10ThreadInfoEPv",
    "_ZN8JobQueue4ExecEP7JobInfoxib",
    "_ZN8JobQueue19ExecuteJobFromQueueEb",
    "_ZN8JobQueue20ScheduleDependenciesER10JobGroupIDP7JobInfoS3_b",
    "_ZN18BackgroundJobQueue19ScheduleJobInternalEPFvPvES0_RKNS_8JobFenceEN8JobQueue16JobQueuePriorityE",
    "_ZN18BackgroundJobQueue29ScheduleMainThreadJobInternalEPFvPvES0_",
    "_ZN18BackgroundJobQueue21ExecuteMainThreadJobsEv",
 };


#endif
