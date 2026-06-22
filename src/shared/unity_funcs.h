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
    UNITY5_JOBQUEUE_SCHEDULE_JOB_MULTIPLE, // JobQueue::ScheduleJobMultipleDependencies(void (*)(void*), void*, JobGroupID*, int)
    UNITY5_JOBQUEUE_WAIT_JOB_GROUP, // JobQueue::WaitForJobGroup(JobGroupID, bool)
    UNITY5_JOBQUEUE_WAIT_JOB_GROUP_ID, // JobQueue::WaitForJobGroupID(JobGroupID)
    UNITY5_JOBQUEUE_HAS_JOB_COMPLETED, // JobQueue::HasJobGroupCompleted(JobGroupID)
    UNITY5_JOBQUEUE_HAS_JOB_ID_COMPLETED, // JobQueue::HasJobGroupIDCompleted(JobGroupID)
    UNITY2K_BACKGROUND_JOBQUEUE_SCHEDULE, // BackgroundJobQueue::ScheduleJobInternal(void (*)(void*), void*, BackgroundJobQueue::JobFence const&, JobQueue::JobQueuePriority)
    UNITY2K_BACKGROUND_JOBQUEUE_SCHEDULE_MAIN, // BackgroundJobQueue::ScheduleMainThreadJobInternal(void (*)(void*), void*)
    UNITY2K_JOBQUEUE_COMPLETE_ALL_JOBS, // JobQueue::CompleteAllJobs()
    UNITY2K_JOBQUEUE_EXEC, // JobQueue::Exec(JobInfo*, long long, int, bool)
    UNITY2K_JOBQUEUE_EXECUTE_QUEUE, // JobQueue::ExecuteJobFromQueue(bool)
    UNITY2K_JOBQUEUE_PROCESS, // JobQueue::ProcessJobs(JobQueue::ThreadInfo*, void*)
    UNITY2K_JOBQUEUE_SCHEDULE_DEPENDENCIES, // JobQueue::ScheduleDependencies(JobGroupID&, JobInfo*, JobInfo*, bool)
    UNITY2K_JOBQUEUE_SCHEDULE_JOB_MULTIPLE, // JobQueue::ScheduleJobMultipleDependencies(void (*)(void*), void*, JobGroupID*, int, MemLabelId)
    UNITY2K_JOBQUEUE_SCHEDULE_GROUP_INTERNAL, // JobQueue::ScheduleGroupInternal(JobGroup*, JobQueue::JobQueuePriority, bool)
    UNITY2K_JOBQUEUE_WAIT_JOB_GROUP, // JobQueue::WaitForJobGroup(JobGroupID)
    UNITY2K_JOBQUEUE_WAIT_JOB_GROUP_ID, // JobQueue::WaitForJobGroupID(JobGroupID, JobQueue::JobQueueWorkStealMode)
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
    UNITY4_PRELOADMANAGER_UPDATE_STEP, // PreloadManager::UpdatePreloadingSingleStep(bool)
    UNITY6_PRELOADMANAGER_UPDATE_STEP, // PreloadManager::UpdatePreloadingSingleStep(PreloadManager::UpdatePreloadingFlags, int)
    UNITY6_PRELOADMANAGER_WAIT, // PreloadManager::WaitForAllAsyncOperationsToComplete()
    UNITY6_PRELOADMANAGER_RUN, // PreloadManager::Run(void*)
    UNITY2K_PRELOADMANAGER_PEEK, // PreloadManager::PeekIntegrateQueue()
    UNITY2K_PRELOADMANAGER_IS_LOADING, // PreloadManager::IsLoadingOrQueued()
    UNITY6_ASYNCREADMANAGER_REQUEST, // AsyncReadManagerThreaded::Request(AsyncReadCommand*)
    UNITY5_ASYNCREADMANAGER_WAIT_DONE, // AsyncReadManagerThreaded::WaitDone(AsyncReadCommand*)
    UNITY2K_ASYNCREADMANAGER_SYNC_REQUEST, // AsyncReadManagerThreaded::SyncRequest(AsyncReadCommand*)
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
    UNITY6_ARCHIVESTORAGECONVERTER_CONSTRUCTOR, // ArchiveStorageConverter::ArchiveStorageConverter(IArchiveStorageConverterListener*, bool)
    UNITY6_ARCHIVESTORAGECONVERTER_PROCESS_ACCUMULATED, // ArchiveStorageConverter::ProcessAccumulatedData()
    UNITY6_ARCHIVESTORAGECONVERTER_PROCESS, // ArchiveStorageConverter::ProcessData(void const*, unsigned long)
    UNITY6_ASSETBUNDLELOAD_FEEDSTREAM, // AssetBundleLoadFromStreamAsyncOperation::FeedStream(void const*, unsigned long)
    UNITY6_LOAD_FMOD_SOUND, // LoadFMODSound(SoundHandle::Instance**, char const*, unsigned int, SampleClip*, unsigned int, VFS::FileSize, FMOD_CREATESOUNDEXINFO*)
    UNITY5_ANALYTICS_UPDATE, // BaseUnityAnalytics::UpdateConfigFromServer()
    UNITY2K_CONNECTCLIENT_UPDATE, // BaseUnityConnectClient::UpdateConfigFromServer()
    UNITY2K_VIDEOPLAYBACK_GET_IMAGE_NO_SEEK, // VideoPlayback::GetImageNoSeek(Texture*, long*)
    UNITY2K_VIDEOPLAYBACK_GET_PIXEL_ASPECT_RATIO_NUMERATOR, // VideoPlayback::GetPixelAspectRatioNumerator() const
    UNITY2K_VIDEOPLAYBACK_GET_PIXEL_ASPECT_RATIO_DENOMINATOR, // VideoPlayback::GetPixelAspectRatioDenominator() const
    UNITY2K_VIDEOPLAYBACK_GET_PLAYBACK_SPEED, // VideoPlayback::GetPlaybackSpeed() const
    UNITY2K_VIDEOPLAYBACK_SET_PLAYBACK_SPEED, // VideoPlayback::SetPlaybackSpeed(float)
    UNITY2K_VIDEOPLAYBACK_START_PLAYBACK, // VideoPlayback::StartPlayback()
    UNITY2K_VIDEOPLAYBACK_PAUSE_PLAYBACK, // VideoPlayback::PausePlayback()
    UNITY2K_VIDEOPLAYBACK_STOP_PLAYBACK, // VideoPlayback::StopPlayback()
    UNITY2K_VIDEOPLAYBACK_IS_PLAYBACK_ACTIVE, // VideoPlayback::IsPlaybackActive()
    UNITY2K_VIDEOPLAYBACK_SEEK_DOUBLE, // VideoPlayback::Seek(double, void (*)(void*), void*)
    UNITY2K_VIDEOPLAYBACK_SEEK_LONG, // VideoPlayback::Seek(long, void (*)(void*), void*)
    UNITY2K_VIDEOPLAYBACK_STEP, // VideoPlayback::Step()
    UNITY2K_VIDEOPLAYBACK_SET_AUDIO_TARGET, // VideoPlayback::SetAudioTarget(unsigned short, bool, AudioSource*)
    UNITY2K_VIDEOPLAYBACK_HAS_AUDIO_SOURCE, // VideoPlayback::HasAudioSource(unsigned short) const
    UNITY2K_VIDEOPLAYBACK_IS_AUDIO_TRACK_ENABLED, // VideoPlayback::IsAudioTrackEnabled(unsigned short) const
    UNITY2K_VIDEOPLAYBACK_SETUP_AUDIO_SOURCE_OUTPUT, // VideoPlayback::SetupAudioSourceOutput(unsigned short, unsigned short, unsigned int)
    UNITY2K_VIDEOPLAYBACK_GET_AUDIO_SOURCE_QUEUE_FREE_SAMPLE_FRAME_COUNT, // VideoPlayback::GetAudioSourceQueueFreeSampleFrameCount(unsigned short) const
    UNITY2K_VIDEOPLAYBACK_RELEASE_AUDIO_SOURCE_OUTPUTS, // VideoPlayback::ReleaseAudioSourceOutputs()
    UNITY2K_VIDEOPLAYBACK_SET_REFERENCE_CLOCK, // VideoPlayback::SetReferenceClock(VideoReferenceClock*, void (*)(void*, double), void*)
    UNITY2K_VIDEOPLAYBACK_SYNC_CLOCK, // VideoPlayback::SyncClock()
    UNITY2K_VIDEOPLAYBACK_UPDATE_PLAYBACK, // VideoPlayback::UpdatePlayback()
    UNITY2K_VIDEOPLAYBACK_ON_CLOCK_RESYNC_COMPLETED, // VideoPlayback::OnClockResyncCompleted()
    UNITY2K_VIDEOPLAYBACK_CLEAR_AUDIO_SOURCE_QUEUES, // VideoPlayback::ClearAudioSourceQueues()
    UNITY2K_VIDEOPLAYBACK_ON_CLOCK_SEEK_COMPLETED, // VideoPlayback::OnClockSeekCompleted()
    UNITY2K_VIDEOPLAYBACK_QUEUE_AUDIO_SOURCE_SAMPLES, // VideoPlayback::QueueAudioSourceSamples(unsigned short, float const*, unsigned int)
    UNITY2K_VIDEOPLAYBACK_CAN_SKIP_ON_DROP, // VideoPlayback::CanSkipOnDrop() const
    UNITY2K_VIDEOPLAYBACK_CAN_RELEASE, // VideoPlayback::CanRelease() const
    UNITY2K_VIDEOPLAYBACK_MARK_FOR_RELEASE, // VideoPlayback::MarkForRelease()
    UNITY2K_VIDEOPLAYBACK_CONFIGURE_AUDIO_OUTPUT, // VideoPlayback::ConfigureAudioOutput(unsigned short)
    UNITY2K_VIDEOPLAYBACK_CALLBACKS_ON_CLOCK_RESYNC_COMPLETED, // VideoPlayback::Callbacks::OnClockResyncCompleted(void*)
    UNITY2K_VIDEOPLAYBACK_CALLBACKS_ON_CLOCK_SEEK_COMPLETED, // VideoPlayback::Callbacks::OnClockSeekCompleted(void*)
    UNITY2K_VIDEOPLAYBACKMGR_CLEANUP_PLAYBACK_JOB, // VideoPlaybackMgr::CleanupPlaybackJob(VideoClipPlayback*)
    UNITY2K_VIDEOPLAYBACKMGR_DESTROY_PLAYBACK_JOB, // VideoPlaybackMgr::DestroyPlaybackJob(VideoClipPlayback*)
    UNITY2K_VIDEOPLAYBACKMGR_IS_PLAYING, // VideoPlaybackMgr::IsPlaying() const
    UNITY2K_VIDEOPLAYBACKMGR_IS_EMPTY, // VideoPlaybackMgr::IsEmpty() const
    UNITY2K_VIDEOPLAYBACKMGR_IS_SUSPENDED, // VideoPlaybackMgr::IsSuspended() const
    UNITY2K_VIDEOPLAYBACKMGR_SUSPEND, // VideoPlaybackMgr::Suspend()
    UNITY2K_VIDEOPLAYBACKMGR_RESUME, // VideoPlaybackMgr::Resume()
    UNITY2K_VIDEOPLAYBACKMGR_UPDATE, // VideoPlaybackMgr::Update()
    UNITY2K_VIDEOPLAYBACKMGR_CREATE_DECODER_THREADS, // VideoPlaybackMgr::CreateDecoderThreads(int)
    UNITY2K_VIDEOPLAYBACKMGR_RELEASE_DECODER_THREADS, // VideoPlaybackMgr::ReleaseDecoderThreads(bool)
    UNITY2K_VIDEOPLAYBACKMGR_DISPATCH_MEDIA_DECODE, // VideoPlaybackMgr::DispatchMediaDecode(VideoPlaybackMgr::DecoderThread*)
    UNITY2K_VIDEOPLAYBACKMGR_CREATE_VIDEO_PLAYBACK_FORMAT, // VideoPlaybackMgr::CreateVideoPlayback(core::basic_string<char, core::StringStorageDefault<char> > const&, VideoMediaFormat, bool, void (*)(void*, core::basic_string<char, core::StringStorageDefault<char> > const&), void (*)(void*), void (*)(void*), void*)
    UNITY2K_VIDEOPLAYBACKMGR_CREATE_VIDEO_PLAYBACK_PATHS, // VideoPlaybackMgr::CreateVideoPlayback(core::basic_string<char, core::StringStorageDefault<char> > const&, core::basic_string<char, core::StringStorageDefault<char> > const&, unsigned long, unsigned long, VideoMediaFormat, bool, bool, void (*)(void*, core::basic_string<char, core::StringStorageDefault<char> > const&), void (*)(void*), void (*)(void*), void*)
    UNITY2K_VIDEOPLAYBACKMGR_RELEASE_VIDEO_PLAYBACK, // VideoPlaybackMgr::ReleaseVideoPlayback(VideoPlayback*)
    UNITY2K_VIDEOPLAYBACKMGR_DECODERTHREAD_START, // VideoPlaybackMgr::DecoderThread::Start()
    UNITY2K_VIDEOPLAYBACKMGR_DECODERTHREAD_WAIT, // VideoPlaybackMgr::DecoderThread::Wait()
    UNITY2K_VIDEOPLAYBACKMGR_DECODERTHREAD_QUIT, // VideoPlaybackMgr::DecoderThread::Quit()
    UNITY2K_VIDEOPLAYBACKMGR_DECODERTHREAD_QUIT_AND_WAIT, // VideoPlaybackMgr::DecoderThread::QuitAndWait()
    UNITY2K_VIDEOPLAYBACKMGR_DECODERTHREAD_SLEEP_MS, // VideoPlaybackMgr::DecoderThread::SleepMS(int)
    UNITY2K_VIDEOPLAYBACKMGR_DECODERTHREAD_IS_RUNNING, // VideoPlaybackMgr::DecoderThread::IsRunning()
    UNITY2K_VIDEOPLAYBACKMGR_DECODERTHREAD_RUN, // VideoPlaybackMgr::DecoderThread::Run()
    UNITY2K_VIDEOPLAYBACKMGR_DECODERTHREAD_START_THREAD, // VideoPlaybackMgr::DecoderThread::StartThread(void*)
    PHYSX_CM_FANOUTTASK_REMOVEREFERENCE, // physx::Cm::FanoutTask::removeReference()
    UNITY_FUNCS_LEN
};

#endif
