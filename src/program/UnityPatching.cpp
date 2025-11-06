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

#include "UnityPatching.h"
#include "Signature.h"
#include "Context.h"
#include "utils.h"

#include "ramsearch/MemAccess.h"
#include "ramsearch/BaseAddresses.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"
#include "../shared/unity_funcs.h"

#include <sys/mman.h> // mmap
#include <iostream>
#include <unistd.h> // access

struct usymbol_t {
    int id;
    const char* name;
    const char* symbol;
};

struct usig_t {
    int id;
    const char* signature;
};

const static usymbol_t UNITY_SYMBOLS[] = {
    {
        UNITY_VERSION,
        "UnityVersion::UnityVersion",
        "_ZN12UnityVersionC2EPKc",
    },
    {
        UNITY4_JOBSCHEDULER_AWAKE,
        "JobScheduler::AwakeIdleWorkerThreads",
        "_ZN12JobScheduler22AwakeIdleWorkerThreadsEi",
    },
    {
        UNITY4_JOBSCHEDULER_FETCH,
        "JobScheduler::FetchNextJob",
        "_ZN12JobScheduler12FetchNextJobERi",
    },
    {
        UNITY4_JOBSCHEDULER_PROCESS,
        "JobScheduler::ProcessJob",
        "_ZN12JobScheduler10ProcessJobER7JobInfoi",
    },
    {
        UNITY4_JOBSCHEDULER_SUBMIT,
        "JobScheduler::SubmitJob",
        "_ZN12JobScheduler9SubmitJobEiPFPvS0_ES0_PVS0_",
    },
    {
        UNITY4_JOBSCHEDULER_WAIT,
        "JobScheduler::WaitForGroup",
        "_ZN12JobScheduler12WaitForGroupEi",
    },
    {
        UNITY5_BACKGROUND_JOBQUEUE_EXECUTE,
        "BackgroundJobQueue::ExecuteMainThreadJobs",
        "_ZN18BackgroundJobQueue21ExecuteMainThreadJobsEv",
    },
    {
        UNITY5_BACKGROUND_JOBQUEUE_SCHEDULE,
        "BackgroundJobQueue::ScheduleJob",
        "_ZN18BackgroundJobQueue11ScheduleJobEPFvPvES0_",
    },
    {
        UNITY5_BACKGROUND_JOBQUEUE_SCHEDULE_MAIN,
        "BackgroundJobQueue::ScheduleMainThreadJob",
        "_ZN18BackgroundJobQueue21ScheduleMainThreadJobEPFvPvES0_",
    },
    {
        UNITY5_JOBQUEUE_CREATE_JOB_BATCH,
        "JobQueue::CreateJobBatch",
        "_ZN8JobQueue14CreateJobBatchEPFvPvES0_10JobGroupIDP8JobGroup",
    },
    {
        UNITY5_JOBQUEUE_ENQUEUEALL,
        "JobQueue::EnqueueAll",
        "_ZN8JobQueue10EnqueueAllEP8JobGroupS1_",
    },
    {
        UNITY5_JOBQUEUE_ENQUEUEALL_INTERNAL,
        "JobQueue::EnqueueAllInternal",
        "_ZN8JobQueue18EnqueueAllInternalEP8JobGroupS1_P11AtomicQueuePi",
    },
    {
        UNITY5_JOBQUEUE_EXEC,
        "JobQueue::Exec",
        "_ZN8JobQueue4ExecEP7JobInfoxi",
    },
    {
        UNITY5_JOBQUEUE_EXECUTE,
        "JobQueue::ExecuteOneJob",
        "_ZN8JobQueue13ExecuteOneJobEv",
    },
    {
        UNITY5_JOBQUEUE_EXECUTE_QUEUE,
        "JobQueue::ExecuteJobFromQueue",
        "_ZN8JobQueue19ExecuteJobFromQueueEv",
    },
    {
        UNITY5_JOBQUEUE_MAIN_ENQUEUEALL,
        "JobQueue::MainEnqueueAll",
        "_ZN8JobQueue14MainEnqueueAllEP8JobGroupS1_",
    },
    {
        UNITY5_JOBQUEUE_POP,
        "JobQueue::Pop",
        "_ZN8JobQueue3PopE10JobGroupID",
    },
    {
        UNITY5_JOBQUEUE_PROCESS,
        "JobQueue::ProcessJobs",
        "_ZN8JobQueue11ProcessJobsEPv",
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_GROUP,
        "JobQueue::ScheduleGroup",
        "_ZN8JobQueue13ScheduleGroupEP8JobGroupNS_16JobQueuePriorityE",
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_GROUPS,
        "JobQueue::ScheduleGroups",
        "_ZN8JobQueue14ScheduleGroupsEP8JobGroupS1_",
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_JOB,
        "JobQueue::ScheduleJob",
        "_ZN8JobQueue11ScheduleJobEPFvPvES0_10JobGroupIDNS_16JobQueuePriorityE",
    },
    {
        UNITY5_JOBQUEUE_WAIT_JOB_GROUP,
        "JobQueue::WaitForJobGroup",
        "_ZN8JobQueue15WaitForJobGroupE10JobGroupIDb",
    },
    {
        UNITY2K_BACKGROUND_JOBQUEUE_SCHEDULE,
        "BackgroundJobQueue::ScheduleJobInternal",
        "_ZN18BackgroundJobQueue19ScheduleJobInternalEPFvPvES0_RKNS_8JobFenceEN8JobQueue16JobQueuePriorityE",
    },
    {
        UNITY2K_BACKGROUND_JOBQUEUE_SCHEDULE_MAIN,
        "BackgroundJobQueue::ScheduleMainThreadJobInternal",
        "_ZN18BackgroundJobQueue29ScheduleMainThreadJobInternalEPFvPvES0_",
    },
    {
        UNITY2K_JOBQUEUE_COMPLETE_ALL_JOBS,
        "JobQueue::CompleteAllJobs",
        "_ZN8JobQueue15CompleteAllJobsEv",
    },
    {
        UNITY2K_JOBQUEUE_EXEC,
        "JobQueue::Exec",
        "_ZN8JobQueue4ExecEP7JobInfoxib",
    },
    {
        UNITY2K_JOBQUEUE_EXECUTE_QUEUE,
        "JobQueue::ExecuteJobFromQueue",
        "_ZN8JobQueue19ExecuteJobFromQueueEb",
    },
    {
        UNITY2K_JOBQUEUE_PROCESS,
        "JobQueue::ProcessJobs",
        "_ZN8JobQueue11ProcessJobsEPNS_10ThreadInfoEPv",
    },
    {
        UNITY2K_JOBQUEUE_SCHEDULE_DEPENDENCIES,
        "JobQueue::ScheduleDependencies",
        "_ZN8JobQueue20ScheduleDependenciesER10JobGroupIDP7JobInfoS3_b",
    },
    {
        UNITY2K_JOBQUEUE_SCHEDULE_JOB_MULTIPLE,
        "JobQueue::ScheduleJobMultipleDependencies",
        "_ZN8JobQueue31ScheduleJobMultipleDependenciesEPFvPvES0_P10JobGroupIDi10MemLabelId",
    },
    {
        UNITY2K_JOBQUEUE_SCHEDULE_GROUP_INTERNAL,
        "JobQueue::ScheduleGroupInternal",
        "_ZN8JobQueue21ScheduleGroupInternalEP8JobGroupNS_16JobQueuePriorityEb",
    },
    {
        UNITY2K_JOBQUEUE_WAIT_JOB_GROUP,
        "JobQueue::WaitForJobGroupID",
        "_ZN8JobQueue17WaitForJobGroupIDE10JobGroupIDNS_21JobQueueWorkStealModeE",
    },
    {
        UNITY6_BATCH_JOB,
        "ScheduleBatchJob",
        "_ZL16ScheduleBatchJobPv13ujob_handle_t",
    },
    {
        UNITY6_JOB_REFLECTION,
        "JobsUtility_CUSTOM_CreateJobReflectionData",
        "_Z42JobsUtility_CUSTOM_CreateJobReflectionDataP37ScriptingBackendNativeObjectPtrOpaqueS0_S0_S0_S0_",
    },
    {
        UNITY6_JOB_SCHEDULE,
        "JobsUtility_CUSTOM_Schedule",
        "_Z27JobsUtility_CUSTOM_ScheduleR21JobScheduleParametersR8JobFence",
    },
    {
        UNITY6_JOBQUEUE_SCHEDULE_GROUPS,
        "JobQueue::ScheduleGroups",
        "_ZN8JobQueue14ScheduleGroupsEP15JobBatchHandlesi",
    },
    {
        UNITY6_LANE_GUTS,
        "lane_guts",
        "_ZL9lane_gutsP14ujob_control_tP11ujob_lane_tiiPK21ujob_dependency_chain",
    },
    {
        UNITY6_JOB_COMPLETED,
        "job_completed",
        "_ZL13job_completedP14ujob_control_tP11ujob_lane_tP10ujob_job_t13ujob_handle_t",
    },
    {
        UNITY6_UJOB_ADD,
        "ujobs_add_to_lane_and_wake_one_thread",
        "_ZL37ujobs_add_to_lane_and_wake_one_threadP14ujob_control_tP10ujob_job_tP11ujob_lane_t",
    },
    {
        UNITY6_UJOB_EXECUTE,
        "ujob_execute_job",
        "_ZL16ujob_execute_jobP14ujob_control_tP11ujob_lane_tP10ujob_job_t13ujob_handle_tj",
    },
    {
        UNITY6_UJOB_KICK,
        "ujob_kick_jobs",
        "_Z14ujob_kick_jobsP14ujob_control_t",
    },
    {
        UNITY6_UJOB_PARTICIPATE,
        "ujob_participate",
        "_Z16ujob_participateP14ujob_control_t13ujob_handle_tRP10ujob_job_tRjPK21ujob_dependency_chain",
    },
    {
        UNITY6_UJOB_SCHEDULE,
        "ujob_schedule_job_internal",
        "_ZL26ujob_schedule_job_internalP14ujob_control_t13ujob_handle_tj",
    },
    {
        UNITY6_UJOB_SCHEDULE_PARALLEL,
        "ujob_schedule_parallel_for_internal",
        "_Z35ujob_schedule_parallel_for_internalP14ujob_control_tR21JobsCallbackFunctionsPvP17WorkStealingRangejjPK13ujob_handle_tih", 
    },
    {
        UNITY6_UJOB_WAIT,
        "ujob_wait_for",
        "_Z13ujob_wait_forP14ujob_control_t13ujob_handle_ti",
    },
    {
        UNITY6_UJOB_WAIT_ALL,
        "ujob_wait_all",
        "_Z13ujob_wait_allP14ujob_control_ti",
    },
    {
        UNITY6_WORKER_THREAD_ROUTINE,
        "worker_thread_routine",
        "_ZL21worker_thread_routinePv",
    },
    {
        UNITY6_PRELOADMANAGER_ADD,
        "PreloadManager::AddToQueue",
        "_ZN14PreloadManager10AddToQueueEP23PreloadManagerOperation",
    },
    {
        UNITY6_PRELOADMANAGER_PREPARE,
        "PreloadManager::PrepareProcessingPreloadOperation",
        "_ZN14PreloadManager33PrepareProcessingPreloadOperationEv",
    },
    {
        UNITY6_PRELOADMANAGER_PROCESS,
        "PreloadManager::ProcessSingleOperation",
        "_ZN14PreloadManager22ProcessSingleOperationEv",
    },
    {
        UNITY6_PRELOADMANAGER_UPDATE,
        "PreloadManager::UpdatePreloading",
        "_ZN14PreloadManager16UpdatePreloadingEv",
    },
    {
        UNITY4_PRELOADMANAGER_UPDATE_STEP,
        "PreloadManager::UpdatePreloadingSingleStep",
        "_ZN14PreloadManager26UpdatePreloadingSingleStepEb.constprop.207",
    },
    {
        UNITY6_PRELOADMANAGER_UPDATE_STEP,
        "PreloadManager::UpdatePreloadingSingleStep",
        "_ZN14PreloadManager26UpdatePreloadingSingleStepENS_21UpdatePreloadingFlagsEi",
    },
    {
        UNITY6_PRELOADMANAGER_WAIT,
        "PreloadManager::WaitForAllAsyncOperationsToComplete",
        "_ZN14PreloadManager35WaitForAllAsyncOperationsToCompleteEv",
    },
    {
        UNITY6_PRELOADMANAGER_RUN,
        "PreloadManager::Run",
        "_ZN14PreloadManager3RunEPv",
    },
    {
        UNITY2K_PRELOADMANAGER_PEEK,
        "PreloadManager::PeekIntegrateQueue",
        "_ZN14PreloadManager18PeekIntegrateQueueEv",
    },
    {
        UNITY2K_PRELOADMANAGER_IS_LOADING,
        "PreloadManager::IsLoadingOrQueued",
        "_ZN14PreloadManager17IsLoadingOrQueuedEv",
    },
    {
        UNITY5_ASYNCREADMANAGER_WAIT_DONE,
        "AsyncReadManagerThreaded::WaitDone",
        "_ZN24AsyncReadManagerThreaded8WaitDoneEP16AsyncReadCommand",
    },
    {
        UNITY6_ASYNCREADMANAGER_REQUEST,
        "AsyncReadManagerThreaded::Request",
        "_ZN24AsyncReadManagerThreaded7RequestEP16AsyncReadCommand",
    },
    {
        UNITY2K_ASYNCREADMANAGER_SYNC_REQUEST,
        "AsyncReadManagerThreaded::SyncRequest",
        "_ZN24AsyncReadManagerThreaded11SyncRequestEP16AsyncReadCommand",
    },
    {
        UNITY6_ASYNCREADMANAGER_OPENCOMPLETE_CALLBACK,
        "AsyncReadManagerManaged::OpenCompleteCallback",
        "_ZN23AsyncReadManagerManagedL20OpenCompleteCallbackER16AsyncReadCommandNS0_6StatusE",
    },
    {
        UNITY6_ASYNCREADMANAGER_READCOMPLETE_CALLBACK,
        "AsyncReadManagerManaged::ReadCompleteCallback",
        "_ZN23AsyncReadManagerManagedL20ReadCompleteCallbackER16AsyncReadCommandNS0_6StatusE",
    },
    {
        UNITY6_ASYNCREADMANAGER_CLOSECOMPLETE_CALLBACK,
        "AsyncReadManagerManaged::CloseCompleteCallback",
        "_ZN23AsyncReadManagerManagedL21CloseCompleteCallbackER16AsyncReadCommandNS0_6StatusE",
    },
    {
        UNITY6_ASYNCREADMANAGER_CLOSECACHEDCOMPLETE_CALLBACK,
        "AsyncReadManagerManaged::CloseCachedFileCompleteCallback",
        "_ZN23AsyncReadManagerManagedL31CloseCachedFileCompleteCallbackER16AsyncReadCommandNS0_6StatusE",
    },
    {
        UNITY6_ASYNCUPLOADMANAGER_ASYNC_READ_SUCCESS,
        "AsyncUploadManager::AsyncReadSuccess",
        "_ZN18AsyncUploadManager16AsyncReadSuccessER12AsyncCommand",
    },
    {
        UNITY6_ASYNCUPLOADMANAGER_QUEUE_UPLOAD,
        "AsyncUploadManager::QueueUploadAsset",
        "_ZN18AsyncUploadManager16QueueUploadAssetEPKcN3VFS8FileSizeEjjRK18AsyncUploadHandlerRK12AssetContextPh13FileReadFlags",
    },
    {
        UNITY6_ASYNCUPLOADMANAGER_ASYNC_RESOURCE_UPLOAD,
        "AsyncUploadManager::AsyncResourceUpload",
        "_ZN18AsyncUploadManager19AsyncResourceUploadER9GfxDeviceiRK26AsyncUploadManagerSettings",
    },
    {
        UNITY6_ASYNCUPLOADMANAGER_ASYNC_READ_CALLBACK,
        "AsyncUploadManager::AsyncReadCallbackStatic",
        "_ZN18AsyncUploadManager23AsyncReadCallbackStaticER16AsyncReadCommandNS0_6StatusE",
    },
    {
        UNITY6_ASYNCUPLOADMANAGER_SCHEDULE,
        "AsyncUploadManager::ScheduleAsyncCommandsInternal",
        "_ZN18AsyncUploadManager29ScheduleAsyncCommandsInternalEv",
    },
    {
        UNITY6_ASYNCUPLOADMANAGER_CLOSE,
        "AsyncUploadManager::CloseFile",
        "_ZN18AsyncUploadManager9CloseFileERKN4core12basic_stringIcNS0_20StringStorageDefaultIcEEEE",
    },
    {
        UNITY6_SIGNAL_CALLBACK,
        "SignalCallback",
        "_ZL14SignalCallbackR16AsyncReadCommandNS_6StatusE",
    },
    {
        UNITY6_SYNC_READ,
        "SyncReadRequest",
        "_Z15SyncReadRequestP16AsyncReadCommand",
    },
    {
        UNITY6_ARCHIVESTORAGECONVERTER_CONSTRUCTOR,
        "ArchiveStorageConverter::ArchiveStorageConverter",
        "_ZN23ArchiveStorageConverterC2EP32IArchiveStorageConverterListenerb",
    },
    {
        UNITY6_ARCHIVESTORAGECONVERTER_PROCESS_ACCUMULATED,
        "ArchiveStorageConverter::ProcessAccumulatedData",
        "_ZN23ArchiveStorageConverter22ProcessAccumulatedDataEv",
    },
    {
        UNITY6_ARCHIVESTORAGECONVERTER_PROCESS,
        "ArchiveStorageConverter::ProcessData",
        "_ZN23ArchiveStorageConverter11ProcessDataEPKvm",
    },
    {
        UNITY6_ASSETBUNDLELOAD_FEEDSTREAM,
        "AssetBundleLoadFromStreamAsyncOperation::FeedStream",
        "_ZN39AssetBundleLoadFromStreamAsyncOperation10FeedStreamEPKvm",
    },
    {
        UNITY6_LOAD_FMOD_SOUND,
        "LoadFMODSound",
        "_Z13LoadFMODSoundPPN11SoundHandle8InstanceEPKcjP10SampleClipjN3VFS8FileSizeEP22FMOD_CREATESOUNDEXINFO",
    },
    {
        UNITY5_ANALYTICS_UPDATE,
        "BaseUnityAnalytics::UpdateConfigFromServer",
        "_ZN18BaseUnityAnalytics22UpdateConfigFromServerEv",
    },
    {
        UNITY_FUNCS_LEN,
        "",
        "",
    },
};

const static usig_t UNITY_SIGNATURES_32[] = {
    {
        UNITY_FUNCS_LEN,
        ""
    },
};

const static usig_t UNITY_SIGNATURES_64[] = {
    {
        UNITY_VERSION,
        "53 48 8d 57 14 48 83 ec 50 64 48 8b 04 25 28 00 00 00 48 89 44 24 48",
    },
    {
        UNITY_VERSION,
        "41 56 53 48 81 ec 98 00 00 00 0f 57 c0 0f 11 07",
    },
    {
        UNITY4_JOBSCHEDULER_FETCH,
        "41 54 55 48 89 F5 53 48 89 FB 48 83 EC 30 48 63 06 8B 57 7C 39 D0"
    },
    {
        UNITY4_JOBSCHEDULER_PROCESS,
        "48 89 5C 24 E8 48 89 F3 48 89 6C 24 F0 4C 89 64 24 F8 89 D5 48 83 EC 48"
    },
    {
        UNITY4_JOBSCHEDULER_SUBMIT,
        "48 83 EC 28 44 8b 4f 0c 45 85 c9 0f 8e ?? ?? ?? ?? 39 77 08"
    },
    {
        UNITY4_JOBSCHEDULER_WAIT,
        "41 55 49 89 FD 41 54 55 53 48 83 EC 38 39 77 08 0F 8E"
    },
    {
        UNITY5_JOBQUEUE_CREATE_JOB_BATCH,
        "41 57 41 56 41 55 41 54 53 4D 89 CE 49 89 D7 49 89 F4 49 89 FD BE 01 00 00 00"
    },
    {
        UNITY5_JOBQUEUE_EXEC,
        // Signature for Unity 2018
        "41 57 41 56 49 89 D6 41 55 41 54 41 89 CC 55 48 89 FD 53 48 89 F3 48 83 ec 18 f0 83 6f 68 01"
    },
    {
        UNITY5_JOBQUEUE_EXEC,
        // Signature for Unity 5
        "41 57 48 8D 47 70 41 56 41 89 CE 41 55 49 89 D5 41 54 55 48 89 F5 53"
    },
    {
        UNITY5_JOBQUEUE_EXECUTE,
        // Signature for Unity 5
        "48 89 5C 24 F0 48 89 6C 24 F8 48 89 FB 48 83 EC 18 48 8B 3F E8 ?? ?? ?? ?? 48 85 C0"
    },
    {
        UNITY5_JOBQUEUE_EXECUTE,
        // Signature for Unity 2020
        "55 41 56 53 48 89 FB 40 8A AF 5A 01 00 00 48 8B 3F"
    },
    {
        UNITY5_JOBQUEUE_EXECUTE,
        // Signature for Unity 2018
        "53 48 89 FB E8 ?? ?? ?? ?? 84 C0 74 ?? B8 01 00 00 00 5B C3 0F 1F 40 00 48 89 df 5B E9"
    },
    {
        UNITY5_JOBQUEUE_EXECUTE_QUEUE,
        "41 54 55 48 89 FD 53 48 83 EC 10 48 8B 7F 08 E8"
    },
    {
        UNITY5_JOBQUEUE_PROCESS,
        // Signature for Unity 2018
        "41 57 41 56 41 55 41 54 55 53 48 89 FB 48 83 EC 48 48 8B 35 ?? ?? ?? ?? 48 8b 3d"
    },
    {
        UNITY5_JOBQUEUE_PROCESS,
        // Signature for Unity 5
        "41 55 41 54 55 48 8d 6f 74 53 48 89 fb 48 83 ec 28 f0 83 45 00 01 4c 8d 67 50"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_JOB,
        // Signature for Unity 2020
        "55 41 57 41 56 41 54 53 45 89 CE 49 89 D7 49 89 F4 48 89 FB BE 01 00 00 00"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_JOB,
        // Signature for Unity 2018
        "55 44 89 CD 53 48 89 fb 48 83 ec 08 e8 ?? ?? ?? ?? 89 ea 48 89 df 48 89 c6 e8"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_JOB,
        // Signature for Unity 5
        "48 89 5C 24 D8 48 89 6C 24 E0 48 89 F5 4C 89 64 24 E8 4C 89 6C 24 F0 49 89 D4 4c 89 74 24 f8 48 83 EC 68 48 89 CA 44 89 44 24 18 48 89 4C 24 10 BE 01 00 00 00"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_GROUP,
        // Signature for Unity 2018
        "41 57 41 56 41 55 49 89 FD 48 89 f7 41 54 49 89 F4 55 89 D5 53 48 83 ec 58"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_GROUP,
        // Signature for Unity 5
        "41 56 41 55 41 89 D5 41 54 49 89 fc 48 89 f7 55 53 48 89 f3 48 83 c4 80"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_GROUPS,
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 18 49 89 FF 8A 87 5A 01 00 00 45 31 E4 88 44 24 0F"
    },
    {
        UNITY5_JOBQUEUE_SCHEDULE_GROUPS,
        // Signature for Unity 5
        "41 55 41 54 49 89 FC 55 53 31 DB 48 83 EC 28 48 85 F6 48 8B 7F 08 74 ?? 48 89 F1 EB ?? ?? ?? ?? 48 8b 41 30"
    },
    {
        UNITY5_JOBQUEUE_WAIT_JOB_GROUP,
        "41 57 41 89 cf 41 56 41 55 41 89 d5 41 54 55 48 89 f5 53 48 89 fb 48 83 ec 78 89 54 24 38"
    },
    {
        UNITY2K_JOBQUEUE_COMPLETE_ALL_JOBS,
        // Signature for Unity 2020
        "55 41 56 53 49 89 FE 66 0F 1F 84 00 00 00 00 00 49 8B 86 38 01 00 00"
    },
    {
        UNITY2K_JOBQUEUE_EXEC,
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 18 89 CB 49 89 F6 49 89 FD"
    },
    {
        UNITY2K_JOBQUEUE_EXECUTE_QUEUE,
        // Signature for Unity 2020
        "55 41 57 41 56 53 48 83 EC 18 89 F5 49 89 FF 48 8B 7F 08"
    },
    {
        UNITY2K_JOBQUEUE_EXECUTE_QUEUE,
        // Signature for Unity 2018
        "41 55 49 89 FD 41 54 55 53 48 83 EC 18 48 8B 7F 08 E8 ?? ?? ?? ?? 49 89 C4"
    },
    {
        UNITY2K_JOBQUEUE_PROCESS,
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 50 48 89 FB 48 8B 3D"
    },
    {
        UNITY2K_JOBQUEUE_SCHEDULE_DEPENDENCIES,
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 28 48 89 D3 49 89 FE 48 8B 2E 48 85 ED"
    },
    {
        UNITY2K_JOBQUEUE_SCHEDULE_JOB_MULTIPLE,
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 38 45 89 CD 44 89 C3 48 89 4C 24 18 48 89 D5"
    },
    {
        UNITY2K_JOBQUEUE_SCHEDULE_GROUP_INTERNAL,
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 18 41 89 CC 41 89 D5 49 89 F6 48 89 FB 48 89 F7"
    },
    {
        UNITY2K_JOBQUEUE_WAIT_JOB_GROUP,
        // Signature for Unity 6
        "55 41 57 41 56 41 55 41 54 53 48 81 EC C8 00 00 00 49 89 F7 C7 44 24 2C 00 00 00 00"
    },
    {
        UNITY2K_JOBQUEUE_WAIT_JOB_GROUP,
        // Signature for Unity 2020
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 48 48 89 54 24 30 48 85 F6 0F 84"
    },
    {
        UNITY6_JOB_COMPLETED,
        "55 41 57 41 56 41 55 41 54 53 48 81 EC 38 20 00 00 49 89 CC"
    },
    {
        UNITY6_LANE_GUTS,
        "55 41 57 41 56 41 55 41 54 53 48 81 EC 88 00 00 00 4C 89 44 24 58 41 89 CD"
    },
    {
        UNITY6_UJOB_ADD,
        "41 57 41 56 53 48 89 D3 49 89 FE 48 8B 46 20"
    },
    {
        UNITY6_UJOB_EXECUTE,
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 48 45 89 C3 48 89 CB"
    },
    {
        UNITY6_UJOB_PARTICIPATE,
        "55 41 57 41 56 41 55 41 54 53 4C 89 44 24 F8 48 89 4C 24 E8 49 89 F2"
    },
    {
        UNITY6_UJOB_SCHEDULE,
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 28 48 89 F3 49 89 FC 48 89 F1"
    },
    {
        UNITY6_UJOB_SCHEDULE_PARALLEL,
        "55 41 57 41 56 41 55 41 54 53 48 83 EC 68 4D 89 CE 45 89 C4"
    },
    {
        UNITY6_WORKER_THREAD_ROUTINE,
        "41 57 41 56 53 49 89 FE 48 8B 5F 08 48 63 07"
    },
    {
        UNITY6_PRELOADMANAGER_ADD,
        "41 57 41 56 41 55 41 54 53 48 83 EC 10 49 89 F7 48 89 FB 4C 8D B7 70 01 00 00",
    },
    {
        UNITY6_PRELOADMANAGER_UPDATE,
        "55 41 57 41 56 41 54 53 48 83 ec 20 49 89 ff 4c 8d b7 70 01 00 00 4c 89 f7",
    },
    {
        UNITY6_PRELOADMANAGER_UPDATE_STEP,
        "55 41 57 41 56 41 55 41 54 53 48 83 ec 48 89 d3 41 89 f7 49 89 fd 80 7f 38 00",
    },
    {
        UNITY6_PRELOADMANAGER_WAIT,
        "55 41 57 41 56 41 55 41 54 53 48 83 ec 38 49 89 fd 8b 47 3c 89 44 24 2c 83 f8 04",
    },
    {
        UNITY6_PRELOADMANAGER_RUN,
        "55 41 57 41 56 41 55 41 54 53 48 81 ec 98 00 00 00 49 89 ff 48 8b af 58 03 00 00",
    },
    {
        UNITY6_ASYNCREADMANAGER_REQUEST,
        "41 57 41 56 41 55 41 54 53 48 83 ec 10 49 89 f7 49 89 fd c7 46 28 01 00 00 00",
    },
    {
        UNITY6_ASYNCREADMANAGER_OPENCOMPLETE_CALLBACK,
        "48 83 ec 18 48 8b 47 40 48 8b 88 b0 01 00 00 8b 90 b8 01 00 00",
    },
    {
        UNITY6_ASYNCREADMANAGER_READCOMPLETE_CALLBACK,
        "48 83 ec 18 48 8b 47 40 48 8b 88 18 01 00 00 8b 90 20 01 00 00",
    },
    {
        UNITY6_ASYNCREADMANAGER_CLOSECOMPLETE_CALLBACK,
        "41 56 53 48 83 ec 18 4c 8b 77 40 49 8b 86 c0 01 00 00",
    },
    {
        UNITY6_ASYNCREADMANAGER_CLOSECACHEDCOMPLETE_CALLBACK,
        "41 56 53 48 83 ec 18 4c 8b 77 40 49 8b 86 18 01 00 00",
    },
    {
        UNITY6_ASYNCUPLOADMANAGER_ASYNC_READ_SUCCESS,
        "41 57 41 56 41 55 41 54 53 48 83 ec 50 49 89 f5 49 89 fe 48 8b 46 60 48 85 c0",
    },
    {
        UNITY6_ASYNCUPLOADMANAGER_QUEUE_UPLOAD,
        "55 41 57 41 56 41 55 41 54 53 50 41 89 ca 49 89 d3 49 89 fe 48 8b bf 38 25 00 00",
    },
    {
        UNITY6_ASYNCUPLOADMANAGER_ASYNC_RESOURCE_UPLOAD,
        "55 41 57 41 56 41 55 41 54 53 48 83 ec 68 48 89 74 24 38 49 89 fe 48 8b 01 48 89 87 5c 25 00 00",
    },
    {
        UNITY6_ASYNCUPLOADMANAGER_ASYNC_READ_CALLBACK,
        "55 41 57 41 56 41 55 41 54 53 48 83 ec 38 41 89 f5 48 89 fb 4c 8b 35 ?? ?? ?? ?? 4c 8b 7f 40",
    },
    {
        UNITY6_ASYNCUPLOADMANAGER_SCHEDULE,
        "55 41 57 41 56 41 55 41 54 53 48 83 ec 58 49 89 fe 83 bf 5c 25 00 00 ff 0f 84 ?? ?? ?? ?? 4d 8b 7e 28 4d 85 ff",
    },
    {
        UNITY6_ASYNCUPLOADMANAGER_CLOSE,
        "55 41 57 41 56 41 55 41 54 53 48 81 ec a8 00 00 00 49 89 f7 8b 07 85 c0 0f 8e",
    },
    {
        UNITY6_SIGNAL_CALLBACK,
        "50 89 77 28 48 8b 77 40 ba 01 00 00 00 b8 01 00 00 00",
    },
    {
        UNITY6_SYNC_READ,
        "55 41 57 41 56 41 55 41 54 53 48 83 ec 28 49 89 fc 48 8b 2d ?? ?? ?? ?? 48 8d 9d e8 0a 00 00",
    },
    {
        UNITY6_ARCHIVESTORAGECONVERTER_CONSTRUCTOR,
        "55 41 57 41 56 41 54 53 48 83 ec 10 49 89 fe 48 89 37 0f 57 c0 0f 11 47 08",
    },
    {
        UNITY6_ARCHIVESTORAGECONVERTER_CONSTRUCTOR,
        "55 41 57 41 56 41 54 53 48 83 ec 10 49 89 d5 48 89 fb 48 89 37 0f 57 c0 0f 11 47 08",
    },
    {
        UNITY6_ARCHIVESTORAGECONVERTER_PROCESS_ACCUMULATED,
        "55 41 57 41 56 41 55 41 54 53 48 81 ec 38 0d 00 00 48 89 fb 80 7f 48 00 0f 84",
    },
    {
        UNITY6_ARCHIVESTORAGECONVERTER_PROCESS,
        "55 41 56 53 48 83 ec 10 48 89 d5 48 89 f1 48 89 fb 48 8b 7f 08 48 8b 43 10",
    },
    {
        UNITY6_ASSETBUNDLELOAD_FEEDSTREAM,
        "41 57 41 56 41 55 41 54 53 48 83 ec 10 83 bf c0 00 00 00 00 75 ?? 49 89 fd 48 83 bf 98 00 00 00 00",
    },
    {
        UNITY6_LOAD_FMOD_SOUND,
        "55 41 57 41 56 41 55 41 54 53 48 81 ec 78 02 00 00 89 54 24 18 48 85 ff 0f 84 ?? ?? ?? ?? 4c 89 cb",
    },
    {
        UNITY_FUNCS_LEN,
        ""
    },
};

bool UnityPatching::sendAddressesFromSymbols(std::string debugfile, uintptr_t base_address)
{
    bool found_symbols = false;

    /* Sometime games have trouble finding the address of the orginal function
     * `SDL_DYNAPI_entry()` that we hook, so we send right away the symbol
     * address if there is one */
    uint64_t sdl_addr = getSymbolAddress("SDL_DYNAPI_entry", debugfile.c_str());
    if (sdl_addr != 0) {
        sdl_addr += base_address;
        
        sendMessage(MSGN_SDL_DYNAPI_ADDR);
        sendData(&sdl_addr, sizeof(uint64_t));
    }

    for (int i=0; UNITY_SYMBOLS[i].id != UNITY_FUNCS_LEN; i++) {
        if (strlen(UNITY_SYMBOLS[i].symbol) == 0)
            continue;

        uint64_t func_addr = getSymbolAddress(UNITY_SYMBOLS[i].symbol, debugfile.c_str());
        if (func_addr != 0) {
            found_symbols = true;
            func_addr += base_address;

            sendMessage(MSGN_UNITY_ADDR);
            sendData(&UNITY_SYMBOLS[i].id, sizeof(int));
            sendData(&func_addr, sizeof(uint64_t));
            std::cout << "Found symbol for function " << UNITY_SYMBOLS[i].name << " in address " << std::hex << func_addr << std::endl;
        }
    }
    
    return found_symbols;
}

void UnityPatching::sendAddressesFromSignatures(std::pair<uintptr_t,uintptr_t> executablefile_segment, bool is_64bit)
{
    /* We need to query the executable memory to make the search */
    ptrdiff_t executable_size = executablefile_segment.second - executablefile_segment.first;
    void* executable_local_addr = mmap(nullptr, executable_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (executable_local_addr == MAP_FAILED) {
        std::cerr << "Could not map a segment of size " << executable_size << " to host the executable memory" << std::endl;
    }
    else {
        int ret = MemAccess::read(executable_local_addr, reinterpret_cast<void*>(executablefile_segment.first), executable_size);
        
        if (ret == -1)
            std::cerr << "Could not read the executable segment memory" << std::endl;
        
        const usig_t* signatures = is_64bit ? UNITY_SIGNATURES_64 : UNITY_SIGNATURES_32;
        
        for (int i=0; signatures[i].id != UNITY_FUNCS_LEN; i++) {
            const char* signature = signatures[i].signature;
            if (strlen(signature) == 0)
                continue;

            /* Get the function name. Not ideal */
            const char* name = "";
            for (int j=0; UNITY_SYMBOLS[j].id != UNITY_FUNCS_LEN; j++) {
                if (UNITY_SYMBOLS[j].id == signatures[i].id) {
                    name = UNITY_SYMBOLS[j].name;
                    break;
                }
            }
            
            Signature sig;
            sig.fromIdaString(signature);
            
            ptrdiff_t func_offset;
            uintptr_t func_addr;
            int match_count = SigSearch::Search(static_cast<uint8_t*>(executable_local_addr), executable_size, sig, &func_offset);
            
            switch (match_count) {
                case 0:
                    // std::cout << "Found no occurrence of signature " << signature << " associated with function symbol " << signature[i].symbol << std::endl;
                    break;
                case 1:
                    func_addr = executablefile_segment.first + func_offset;
                    std::cout << "Found unique matching signature for function " << name << " in address " << std::hex << (uintptr_t)func_addr << std::endl;
                    sendMessage(MSGN_UNITY_ADDR);
                    sendData(&signatures[i].id, sizeof(int));
                    sendData(&func_addr, sizeof(uintptr_t));
                    break;
                default:
                    std::cout << "Found " << match_count << " occurrences of signature " << signature << " associated with function " << name << std::endl;
                    break;
            }
        }
        
        munmap(executable_local_addr, executable_size);
    }
}

void UnityPatching::sendAddresses(Context* context)
{
    /* Find and send symbol addresses for main executable or `UnityPlayer_s.debug` */
    std::string unityplayer = dirFromPath(context->gameexecutable) + "/UnityPlayer.so";
    bool has_unityplayer = access(unityplayer.c_str(), F_OK) == 0;
    
    std::string debugfile = dirFromPath(context->gameexecutable) + "/UnityPlayer_s.debug";
    bool has_unityplayer_debug = access(debugfile.c_str(), F_OK) == 0;
    
    std::pair<uintptr_t,uintptr_t> executablefile_segment;
    int gameArch = extractBinaryType(context->gameexecutable);

    /* If the executable is position-independent (pie), it will be mapped
     * somewhere, and the symbol is only an offset, so we need the base 
     * address of the executable and adds to it. We use `file` to determine
     * if the executable is pie */
    bool is_pie = gameArch & BT_PIEAPP;
    bool is_64bit = gameArch & BT_ELF64;

    if (has_unityplayer) {
        executablefile_segment = BaseAddresses::getAddress("UnityPlayer.so");
        is_pie = true;
    }
    else {
        debugfile = context->gameexecutable;
        executablefile_segment = BaseAddresses::getExecutableSection();
    }
    
    /* Send Unity function pointers from symbol locations */
    bool found_symbols = false;
    if (has_unityplayer_debug || !has_unityplayer) {

        uintptr_t base_address = is_pie ? executablefile_segment.first : 0;
        found_symbols = sendAddressesFromSymbols(debugfile, base_address);
    }
    
    /* If no symbol present, try to find functions by signature */
    if (!found_symbols) {
        sendAddressesFromSignatures(executablefile_segment, is_64bit);
    }
}
