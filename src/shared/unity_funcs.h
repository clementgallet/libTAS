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

#ifndef LIBTAS_UNITY_FUNCS_H_INCLUDED
#define LIBTAS_UNITY_FUNCS_H_INCLUDED

/* List of message identification values that is sent from/to the game */
enum {
    UNITY_VERSION, // UnityVersion::UnityVersion(char const*)
    GET_NUMERIC_VERSION, // GetNumericVersion()
    UNITY4_JOBSCHEDULER_FETCH, // JobScheduler::FetchNextJob(int&)
    UNITY4_JOBSCHEDULER_PROCESS, // JobScheduler::ProcessJob(JobInfo&, int)
    UNITY4_JOBSCHEDULER_SUBMIT, // JobScheduler::SubmitJob(int, void* (*)(void*), void*, void* volatile*)
    UNITY4_JOBSCHEDULER_WAIT, // JobScheduler::WaitForGroup(int)
    UNITY5_JOBQUEUE_EXEC, // JobQueue::Exec(JobInfo*, long long, int)
    UNITY5_JOBQUEUE_EXECUTE, // JobQueue::ExecuteOneJob()
    UNITY5_JOBQUEUE_EXECUTE_QUEUE, // JobQueue::ExecuteJobFromQueue()
    UNITY5_JOBQUEUE_PROCESS, // JobQueue::ProcessJobs(void*)
    UNITY5_JOBQUEUE_PROCESS_BOOL, // JobQueue::ProcessJobs(void*, bool*)
    UNITY5_JOBQUEUE_SCHEDULE_GROUP, // JobQueue::ScheduleGroup(JobGroup*, JobQueue::JobQueuePriority)
    UNITY5_JOBQUEUE_SCHEDULE_GROUPS, // JobQueue::ScheduleGroups(JobGroup*, JobGroup*)
    UNITY5_JOBQUEUE_SCHEDULE_JOB, // JobQueue::ScheduleJob(void (*)(void*), void*, JobGroupID, JobQueue::JobQueuePriority)
    UNITY5_JOBQUEUE_SCHEDULE_JOB_MULTIPLE, // JobQueue::ScheduleJobMultipleDependencies(void (*)(void*), void*, JobGroupID*, int)
    UNITY5_JOBQUEUE_WAIT_JOB_GROUP, // JobQueue::WaitForJobGroup(JobGroupID, bool)
    UNITY5_JOBQUEUE_WAIT_JOB_GROUP_ID, // JobQueue::WaitForJobGroupID(JobGroupID)
    UNITY2K_JOBQUEUE_EXEC, // JobQueue::Exec(JobInfo*, long long, int, bool)
    UNITY2K_JOBQUEUE_EXECUTE_QUEUE, // JobQueue::ExecuteJobFromQueue(bool)
    UNITY2K_JOBQUEUE_PROCESS, // JobQueue::ProcessJobs(JobQueue::ThreadInfo*, void*)
    UNITY2K_JOBQUEUE_SCHEDULE_DEPENDENCIES, // JobQueue::ScheduleDependencies(JobGroupID&, JobInfo*, JobInfo*, bool)
    UNITY2K_JOBQUEUE_SCHEDULE_GROUP_INTERNAL, // JobQueue::ScheduleGroupInternal(JobGroup*, JobQueue::JobQueuePriority, bool)
    UNITY2K_JOBQUEUE_WAIT_JOB_GROUP, // JobQueue::WaitForJobGroup(JobGroupID)
    UNITY2K_JOBQUEUE_WAIT_JOB_GROUP_ID, // JobQueue::WaitForJobGroupID(JobGroupID, JobQueue::JobQueueWorkStealMode)
    UNITY6_JOB_COMPLETED, // job_completed(ujob_control_t*, ujob_lane_t*, ujob_job_t*, ujob_handle_t)
    UNITY2K_UJOB_SCHEDULE, // ujob_schedule_job_internal(ujob_control_t*, ujob_handle_t)
    UNITY6_UJOB_SCHEDULE, // ujob_schedule_job_internal(ujob_control_t*, ujob_handle_t, unsigned int)
    UNITY6_UJOB_SCHEDULE_PARALLEL, // ujob_schedule_parallel_for_internal(ujob_control_t*, JobsCallbackFunctions&, void*, WorkStealingRange*, unsigned int, unsigned int, ujob_handle_t const*, int, unsigned char)
    UNITY6_UJOB_WAIT, // ujob_wait_for(ujob_control_t*, ujob_handle_t, int)
    UNITY6_UJOB_EXECUTE, // ujob_execute_job(ujob_control_t*, ujob_lane_t*, ujob_job_t*, ujob_handle_t, unsigned int)
    UNITY6_WORKER_THREAD_ROUTINE, // worker_thread_routine(void*)
    UNITY6_PRELOADMANAGER_ADD, // PreloadManager::AddToQueue(PreloadManagerOperation*)
    UNITY6_PRELOADMANAGER_PREPARE, // PreloadManager::PrepareProcessingPreloadOperation()
    UNITY6_PRELOADMANAGER_PROCESS, // PreloadManager::ProcessSingleOperation()
    UNITY6_PRELOADMANAGER_UPDATE, // PreloadManager::UpdatePreloading
    UNITY4_PRELOADMANAGER_UPDATE_STEP, // PreloadManager::UpdatePreloadingSingleStep(bool)
    UNITY6_PRELOADMANAGER_UPDATE_STEP, // PreloadManager::UpdatePreloadingSingleStep(PreloadManager::UpdatePreloadingFlags, int)
    UNITY6_PRELOADMANAGER_WAIT, // PreloadManager::WaitForAllAsyncOperationsToComplete()
    UNITY6_PRELOADMANAGER_RUN, // PreloadManager::Run(void*)
    UNITY6_ASYNCREADMANAGER_REQUEST, // AsyncReadManagerThreaded::Request(AsyncReadCommand*)
    UNITY5_ASYNCREADMANAGER_WAIT_DONE, // AsyncReadManagerThreaded::WaitDone(AsyncReadCommand*)
    UNITY2K_ASYNCREADMANAGER_SYNC_REQUEST, // AsyncReadManagerThreaded::SyncRequest(AsyncReadCommand*)
    UNITY6_SYNC_READ, // SyncReadRequest(AsyncReadCommand*)
    UNITY6_LOAD_FMOD_SOUND, // LoadFMODSound(SoundHandle::Instance**, char const*, unsigned int, SampleClip*, unsigned int, VFS::FileSize, FMOD_CREATESOUNDEXINFO*)
    UNITY5_ANALYTICS_UPDATE, // BaseUnityAnalytics::UpdateConfigFromServer()
    UNITY2K_CONNECTCLIENT_UPDATE, // BaseUnityConnectClient::UpdateConfigFromServer()
    UNITY2K_VIDEOPLAYBACKMGR_CREATE_VIDEO_PLAYBACK_FORMAT, // VideoPlaybackMgr::CreateVideoPlayback(core::basic_string<char, core::StringStorageDefault<char> > const&, VideoMediaFormat, bool, void (*)(void*, core::basic_string<char, core::StringStorageDefault<char> > const&), void (*)(void*), void (*)(void*), void*)
    UNITY2K_VIDEOPLAYBACKMGR_CREATE_VIDEO_PLAYBACK_PATHS, // VideoPlaybackMgr::CreateVideoPlayback(core::basic_string<char, core::StringStorageDefault<char> > const&, core::basic_string<char, core::StringStorageDefault<char> > const&, unsigned long, unsigned long, VideoMediaFormat, bool, bool, void (*)(void*, core::basic_string<char, core::StringStorageDefault<char> > const&), void (*)(void*), void (*)(void*), void*)
    UNITY2K_VIDEOPLAYBACKMGR_DECODERTHREAD_RUN, // VideoPlaybackMgr::DecoderThread::Run()
    UNITY2K_VIDEOCLIPPLAYBACK_IS_READY, // VideoClipPlayback::IsReady()
    UNITY2K_VIDEOCLIPPLAYBACK_EXECUTE_DECODE, // VideoClipPlayback::ExecuteDecode()
    UNITY2K_VIDEOCLIPPLAYBACK_GET_STATUS, // VideoClipPlayback::GetStatus()

    PHYSX_CM_FANOUTTASK_REMOVEREFERENCE, // physx::Cm::FanoutTask::removeReference()
    PHYSX_SC_SCENE_COLLIDESTEP, // physx::Sc::Scene::collideStep()
    SC_SIMULATION_UPDATE_SC_BODY, // ScSimulationControllerCallback::updateScBodyAndShapeSim()
    UNITY_FUNCS_LEN
};

#endif
