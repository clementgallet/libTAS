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
    UNITY4_JOBSCHEDULER_AWAKE, // JobScheduler::AwakeIdleWorkerThreads(int)
    UNITY4_JOBSCHEDULER_FETCH, // JobScheduler::FetchNextJob(int&)
    UNITY4_JOBSCHEDULER_PROCESS, // JobScheduler::ProcessJob(JobInfo&, int)
    UNITY4_JOBSCHEDULER_SUBMIT, // JobScheduler::SubmitJob(int, void* (*)(void*), void*, void* volatile*)
    UNITY4_JOBSCHEDULER_WAIT, // JobScheduler::WaitForGroup(int)
    UNITY5_BACKGROUND_JOBQUEUE_EXECUTE, // BackgroundJobQueue::ExecuteMainThreadJobs()
    UNITY5_BACKGROUND_JOBQUEUE_SCHEDULE, //  BackgroundJobQueue::ScheduleJob(void (*)(void*), void*)
    UNITY5_BACKGROUND_JOBQUEUE_SCHEDULE_MAIN, // BackgroundJobQueue::ScheduleMainThreadJob(void (*)(void*), void*)
    UNITY5_JOBQUEUE_CREATE_JOB_BATCH, // JobQueue::CreateJobBatch(void (*)(void*), void*, JobGroupID, JobGroup*)
    UNITY5_JOBQUEUE_ENQUEUEALL, // JobQueue::EnqueueAll(JobGroup*, JobGroup*)
    UNITY5_JOBQUEUE_ENQUEUEALL_INTERNAL, // JobQueue::EnqueueAllInternal(JobGroup*, JobGroup*, AtomicQueue*, int*)
    UNITY5_JOBQUEUE_EXEC, // JobQueue::Exec(JobInfo*, long long, int)
    UNITY5_JOBQUEUE_EXECUTE, // JobQueue::ExecuteOneJob()
    UNITY5_JOBQUEUE_EXECUTE_QUEUE, // JobQueue::ExecuteJobFromQueue()
    UNITY5_JOBQUEUE_MAIN_ENQUEUEALL, // JobQueue::MainEnqueueAll(JobGroup*, JobGroup*)
    UNITY5_JOBQUEUE_POP, // JobQueue::Pop(JobGroupID)
    UNITY5_JOBQUEUE_PROCESS, // JobQueue::ProcessJobs(void*)
    UNITY5_JOBQUEUE_SCHEDULE_GROUP, // JobQueue::ScheduleGroup(JobGroup*, JobQueue::JobQueuePriority)
    UNITY5_JOBQUEUE_SCHEDULE_GROUPS, // JobQueue::ScheduleGroups(JobGroup*, JobGroup*)
    UNITY5_JOBQUEUE_SCHEDULE_JOB, // JobQueue::ScheduleJob(void (*)(void*), void*, JobGroupID, JobQueue::JobQueuePriority)
    UNITY5_JOBQUEUE_WAIT_JOB_GROUP, // JobQueue::WaitForJobGroup(JobGroupID, bool)
    UNITY2K_BACKGROUND_JOBQUEUE_SCHEDULE, // BackgroundJobQueue::ScheduleJobInternal(void (*)(void*), void*, BackgroundJobQueue::JobFence const&, JobQueue::JobQueuePriority)
    UNITY2K_BACKGROUND_JOBQUEUE_SCHEDULE_MAIN, // BackgroundJobQueue::ScheduleMainThreadJobInternal(void (*)(void*), void*)
    UNITY2K_JOBQUEUE_COMPLETE_ALL_JOBS, // JobQueue::CompleteAllJobs()
    UNITY2K_JOBQUEUE_EXEC, // JobQueue::Exec(JobInfo*, long long, int, bool)
    UNITY2K_JOBQUEUE_EXECUTE_QUEUE, // JobQueue::ExecuteJobFromQueue(bool)
    UNITY2K_JOBQUEUE_PROCESS, // JobQueue::ProcessJobs(JobQueue::ThreadInfo*, void*)
    UNITY2K_JOBQUEUE_SCHEDULE_DEPENDENCIES, // JobQueue::ScheduleDependencies(JobGroupID&, JobInfo*, JobInfo*, bool)
    UNITY2K_JOBQUEUE_SCHEDULE_JOB_MULTIPLE, // JobQueue::ScheduleJobMultipleDependencies(void (*)(void*), void*, JobGroupID*, int, MemLabelId)
    UNITY2K_JOBQUEUE_SCHEDULE_GROUP_INTERNAL, // JobQueue::ScheduleGroupInternal(JobGroup*, JobQueue::JobQueuePriority, bool)
    UNITY2K_JOBQUEUE_WAIT_JOB_GROUP, // JobQueue::WaitForJobGroupID(JobGroupID, JobQueue::JobQueueWorkStealMode)
    UNITY6_BATCH_JOB, // ScheduleBatchJob(void*, ujob_handle_t)
    UNITY6_JOB_COMPLETED, // job_completed(ujob_control_t*, ujob_lane_t*, ujob_job_t*, ujob_handle_t)
    UNITY6_JOB_REFLECTION, // JobsUtility_CUSTOM_CreateJobReflectionData(ScriptingBackendNativeObjectPtrOpaque*, ScriptingBackendNativeObjectPtrOpaque*, ScriptingBackendNativeObjectPtrOpaque*, ScriptingBackendNativeObjectPtrOpaque*, ScriptingBackendNativeObjectPtrOpaque*)
    UNITY6_JOB_SCHEDULE, // JobsUtility_CUSTOM_Schedule(JobScheduleParameters&, JobFence&)
    UNITY6_JOBQUEUE_SCHEDULE_GROUPS, // JobQueue::ScheduleGroups(JobBatchHandles*, int)
    UNITY6_LANE_GUTS, // lane_guts(ujob_control_t*, ujob_lane_t*, int, int, ujob_dependency_chain const*)
    UNITY6_UJOB_SCHEDULE, // ujob_schedule_job_internal(ujob_control_t*, ujob_handle_t, unsigned int)
    UNITY6_UJOB_SCHEDULE_PARALLEL, // ujob_schedule_parallel_for_internal(ujob_control_t*, JobsCallbackFunctions&, void*, WorkStealingRange*, unsigned int, unsigned int, ujob_handle_t const*, int, unsigned char)
    UNITY6_UJOB_WAIT, // ujob_wait_for(ujob_control_t*, ujob_handle_t, int)
    UNITY6_UJOB_WAIT_ALL, // ujob_wait_all(ujob_control_t*, int)
    UNITY6_UJOB_KICK, // ujob_kick_jobs(ujob_control_t*)
    UNITY6_UJOB_EXECUTE, // ujob_execute_job(ujob_control_t*, ujob_lane_t*, ujob_job_t*, ujob_handle_t, unsigned int)
    UNITY6_UJOB_ADD, // ujobs_add_to_lane_and_wake_one_thread(ujob_control_t*, ujob_job_t*, ujob_lane_t*)
    UNITY6_UJOB_PARTICIPATE, // ujob_participate(ujob_control_t*, ujob_handle_t, ujob_job_t*&, unsigned int&, ujob_dependency_chain const*)
    UNITY6_WORKER_THREAD_ROUTINE, // worker_thread_routine(void*)
    UNITY6_PRELOADMANAGER_ADD, // PreloadManager::AddToQueue(PreloadManagerOperation*)
    UNITY6_PRELOADMANAGER_PREPARE, // PreloadManager::PrepareProcessingPreloadOperation()
    UNITY6_PRELOADMANAGER_PROCESS, // PreloadManager::ProcessSingleOperation()
    UNITY6_PRELOADMANAGER_UPDATE, // PreloadManager::UpdatePreloading
    UNITY6_PRELOADMANAGER_UPDATE_STEP, // PreloadManager::UpdatePreloadingSingleStep(PreloadManager::UpdatePreloadingFlags, int)
    UNITY6_PRELOADMANAGER_WAIT, // PreloadManager::WaitForAllAsyncOperationsToComplete()
    UNITY6_PRELOADMANAGER_RUN, // PreloadManager::Run(void*)
    UNITY6_ASYNCREADMANAGER_REQUEST, // AsyncReadManagerThreaded::Request(AsyncReadCommand*)
    UNITY6_ASYNCREADMANAGER_OPENCOMPLETE_CALLBACK, // AsyncReadManagerManaged::OpenCompleteCallback(AsyncReadCommand&, AsyncReadCommand::Status)
    UNITY6_ASYNCREADMANAGER_READCOMPLETE_CALLBACK, // AsyncReadManagerManaged::ReadCompleteCallback(AsyncReadCommand&, AsyncReadCommand::Status)
    UNITY6_ASYNCREADMANAGER_CLOSECOMPLETE_CALLBACK, // AsyncReadManagerManaged::CloseCompleteCallback(AsyncReadCommand&, AsyncReadCommand::Status)
    UNITY6_ASYNCREADMANAGER_CLOSECACHEDCOMPLETE_CALLBACK, // AsyncReadManagerManaged::CloseCachedFileCompleteCallback(AsyncReadCommand&, AsyncReadCommand::Status)
    UNITY6_ASYNCUPLOADMANAGER_ASYNC_READ_SUCCESS, // AsyncUploadManager::AsyncReadSuccess(AsyncCommand&)
    UNITY6_ASYNCUPLOADMANAGER_QUEUE_UPLOAD, // AsyncUploadManager::QueueUploadAsset(char const*, VFS::FileSize, unsigned int, unsigned int, AsyncUploadHandler const&, AssetContext const&, unsigned char*, FileReadFlags)
    UNITY6_ASYNCUPLOADMANAGER_ASYNC_RESOURCE_UPLOAD, // AsyncUploadManager::AsyncResourceUpload(GfxDevice&, int, AsyncUploadManagerSettings const&)
    UNITY6_ASYNCUPLOADMANAGER_ASYNC_READ_CALLBACK, // AsyncUploadManager::AsyncReadCallbackStatic(AsyncReadCommand&, AsyncReadCommand::Status)
    UNITY6_ASYNCUPLOADMANAGER_SCHEDULE, // AsyncUploadManager::ScheduleAsyncCommandsInternal()
    UNITY6_ASYNCUPLOADMANAGER_CLOSE, // AsyncUploadManager::CloseFile(core::basic_string<char, core::StringStorageDefault<char> > const&)
    UNITY6_SIGNAL_CALLBACK, // SignalCallback(AsyncReadCommand&, AsyncReadCommand::Status)
    UNITY6_SYNC_READ, // SyncReadRequest(AsyncReadCommand*)
    UNITY_FUNCS_LEN
};

#endif
