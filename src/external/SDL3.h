/*
    SDL3 replacement header for libTAS.
    Only minimal declarations are provided to compile SDL3 dynapi function declarations.
*/

#ifndef _SDL3_h
#define _SDL3_h

#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

#include "SDL_common.h"

#define SDL_VERSIONNUM_MAJOR(version) ((version) / 1000000)
#define SDL_VERSIONNUM_MINOR(version) (((version) / 1000) % 1000)
#define SDL_VERSIONNUM_MICRO(version) ((version) % 1000)

namespace libtas {

namespace sdl3 {

// === Core SDL3 Type Aliases ===
typedef int SDL_JoystickID;
typedef Sint64 SDL_TouchID;
typedef Sint64 SDL_FingerID;
typedef Sint64 SDL_GestureID;
typedef Uint32 SDL_AudioDeviceID;
typedef Uint32 SDL_TimerID;
typedef Sint32 SDL_Keycode;
typedef Uint16 SDL_Keymod;

typedef void *JavaVM;
typedef void *PFN_xrGetInstanceProcAddr;
typedef void *SDL_AppEvent_func;
typedef void *SDL_AppInit_func;
typedef void *SDL_AppIterate_func;
typedef void *SDL_AppQuit_func;
typedef void *SDL_AssertData;
typedef void *SDL_AssertState;
typedef void *SDL_AssertionHandler;
typedef void *SDL_AsyncIO;
typedef void *SDL_AsyncIOOutcome;
typedef void *SDL_AsyncIOQueue;
typedef void *SDL_AtomicInt;
typedef void *SDL_AtomicU32;
// SDL_AudioPostmixCallback: skip
// SDL_AudioStreamCallback: skip
// SDL_AudioStreamDataCompleteCallback: skip
// SDL_BlendFactor defined as enum
// SDL_BlendMode defined as enum
// SDL_BlendOperation defined as enum
typedef void *SDL_Camera;
typedef void *SDL_CameraID;
typedef void *SDL_CameraPermissionState;
typedef void *SDL_CameraPosition;
typedef void *SDL_CameraSpec;
typedef void *SDL_CleanupPropertyCallback;
typedef void *SDL_ClipboardCleanupCallback;
typedef void *SDL_ClipboardDataCallback;
// SDL_Color defined as struct
typedef void *SDL_Colorspace;
// SDL_CompareCallback defined as function pointer
typedef void *SDL_CompareCallback_r;
typedef void *SDL_Condition;
// SDL_Cursor defined in forward declarations
typedef void *SDL_CursorFrameInfo;
typedef void *SDL_DateFormat;
typedef void *SDL_DateTime;
typedef void *SDL_DialogFileCallback;
typedef void *SDL_DialogFileFilter;
typedef Uint32 SDL_DisplayID;
// SDL_DisplayOrientation defined as enum
typedef void *SDL_EGLAttribArrayCallback;
typedef void *SDL_EGLConfig;
typedef void *SDL_EGLDisplay;
typedef void *SDL_EGLIntArrayCallback;
typedef void *SDL_EGLSurface;
typedef void *SDL_EnumerateDirectoryCallback;
typedef void *SDL_EnumeratePropertiesCallback;
typedef void *SDL_Environment;
// SDL_Event defined as union
// SDL_EventAction: skip
// SDL_EventFilter defined as function pointer
// SDL_FPoint defined as struct
// SDL_FRect defined as struct
typedef void *SDL_FileDialogType;
// SDL_Finger defined in forward declarations
// SDL_FlashOperation defined as enum
// SDL_FlipMode: skip
typedef void *SDL_Folder;
typedef void *SDL_FunctionPointer;
typedef void *SDL_GPUBlitInfo;
typedef void *SDL_GPUBuffer;
typedef void *SDL_GPUBufferBinding;
typedef void *SDL_GPUBufferCreateInfo;
typedef void *SDL_GPUBufferLocation;
typedef void *SDL_GPUBufferRegion;
typedef void *SDL_GPUColorTargetInfo;
typedef void *SDL_GPUCommandBuffer;
typedef void *SDL_GPUComputePass;
typedef void *SDL_GPUComputePipeline;
typedef void *SDL_GPUComputePipelineCreateInfo;
typedef void *SDL_GPUCopyPass;
typedef void *SDL_GPUDepthStencilTargetInfo;
typedef void *SDL_GPUDevice;
typedef void *SDL_GPUFence;
typedef void *SDL_GPUGraphicsPipeline;
typedef void *SDL_GPUGraphicsPipelineCreateInfo;
typedef void *SDL_GPUIndexElementSize;
typedef void *SDL_GPUPresentMode;
typedef void *SDL_GPURenderPass;
typedef void *SDL_GPURenderState;
typedef void *SDL_GPURenderStateCreateInfo;
typedef void *SDL_GPUSampleCount;
typedef void *SDL_GPUSampler;
typedef void *SDL_GPUSamplerCreateInfo;
typedef void *SDL_GPUShader;
typedef void *SDL_GPUShaderCreateInfo;
typedef void *SDL_GPUShaderFormat;
typedef void *SDL_GPUStorageBufferReadWriteBinding;
typedef void *SDL_GPUStorageTextureReadWriteBinding;
typedef void *SDL_GPUSwapchainComposition;
typedef void *SDL_GPUTexture;
typedef void *SDL_GPUTextureCreateInfo;
typedef void *SDL_GPUTextureFormat;
typedef void *SDL_GPUTextureLocation;
typedef void *SDL_GPUTextureRegion;
typedef void *SDL_GPUTextureSamplerBinding;
typedef void *SDL_GPUTextureTransferInfo;
typedef void *SDL_GPUTextureType;
typedef void *SDL_GPUTextureUsageFlags;
typedef void *SDL_GPUTransferBuffer;
typedef void *SDL_GPUTransferBufferCreateInfo;
typedef void *SDL_GPUTransferBufferLocation;
typedef void *SDL_GPUViewport;
typedef void *SDL_Gamepad;
typedef void *SDL_GlobFlags;
// SDL_HintCallback defined as function pointer
// SDL_HintPriority defined as enum
// SDL_HitTest defined as int typedef
typedef void *SDL_INOUT_Z_CAP;
typedef void *SDL_IOStatus;
typedef void *SDL_IOStream;
typedef void *SDL_IOStreamInterface;
typedef void *SDL_IOWhence;
typedef void *SDL_InitState;
typedef Uint32 SDL_KeyboardID;
// SDL_Keycode defined as Sint32 typedef
// SDL_Keymod defined as Uint16 typedef
// SDL_Locale defined in forward declarations
// SDL_LogOutputFunction defined as function pointer
// SDL_LogPriority defined as enum
typedef void *SDL_MainThreadCallback;
// SDL_MessageBoxData defined in forward declarations
typedef void *SDL_MessageBoxFlags;
// SDL_MetalView defined as void* alias
typedef Uint32 SDL_MouseID;
typedef void *SDL_MouseMotionTransformCallback;
typedef void *SDL_Mutex;
typedef void *SDL_NSTimerCallback;
// SDL_Palette defined as struct
typedef void *SDL_PathInfo;
typedef void *SDL_PenDeviceType;
typedef void *SDL_PenID;
// SDL_Point defined as struct
// SDL_PowerState defined as enum
typedef void *SDL_Process;
typedef Uint32 SDL_PropertiesID;
typedef void *SDL_PropertyType;
typedef void *SDL_RWLock;
// SDL_Renderer defined in forward declarations
typedef void *SDL_RequestAndroidPermissionCallback;
typedef void *SDL_Sandbox;
// SDL_ScaleMode: skip
// SDL_Scancode defined as enum
typedef void *SDL_Semaphore;
// SDL_Sensor defined in forward declarations
// SDL_SensorID defined as int typedef
// SDL_SensorType defined as enum
typedef void *SDL_SharedObject;
// SDL_SpinLock defined as int typedef
typedef void *SDL_Storage;
typedef void *SDL_StorageInterface;
// SDL_Surface defined as struct
// SDL_SystemCursor defined as enum
typedef void *SDL_SystemTheme;
typedef void *SDL_TLSDestructorCallback;
typedef void *SDL_TLSID;
// SDL_Texture defined in forward declarations
// SDL_TextureAccess defined as enum
// SDL_Thread defined in forward declarations
// SDL_ThreadFunction defined as function pointer
typedef void *SDL_ThreadID;
// SDL_ThreadPriority defined as int typedef
typedef void *SDL_ThreadState;
typedef void *SDL_Time;
typedef void *SDL_TimeFormat;
typedef void *SDL_TimerCallback;
// SDL_TimerID defined as Uint32 typedef
// SDL_TouchDeviceType defined as enum
// SDL_TouchID defined as Sint64 typedef
typedef void *SDL_Tray;
typedef void *SDL_TrayCallback;
typedef void *SDL_TrayEntry;
typedef void *SDL_TrayEntryFlags;
typedef void *SDL_TrayMenu;
// SDL_Vertex defined as struct
// SDL_VirtualJoystickDesc defined in forward declarations
// SDL_Window defined in forward declarations
typedef Uint32 SDL_WindowID;
// SDL_WindowsMessageHook defined as function pointer
typedef void *SDL_X11EventHook;
typedef void *SDL_calloc_func;
typedef void *SDL_free_func;
// SDL_hid_device forward declared
// SDL_hid_device_info forward declared
typedef void *SDL_iOSAnimationCallback;
// SDL_iconv_t defined as void* typedef
// SDL_main_func defined as function pointer
typedef void *SDL_malloc_func;
typedef void *SDL_realloc_func;
typedef void *VkInstance;
typedef void *VkPhysicalDevice;
typedef void *VkSurfaceKHR;
typedef struct VkAllocationCallbacks VkAllocationCallbacks;
typedef void *XTaskQueueHandle;
typedef void *XUserHandle;
typedef void *XrResult;
typedef void *XrSession;
typedef void *XrSessionCreateInfo;
typedef void *XrSwapchain;
typedef void *XrSwapchainCreateInfo;

// === Core SDL3 Structures and Enums ===

typedef enum SDL_InitFlags
{
    SDL_INIT_AUDIO      = 0x00000010u,
    SDL_INIT_VIDEO      = 0x00000020u, 
    SDL_INIT_JOYSTICK   = 0x00000200u, 
    SDL_INIT_HAPTIC     = 0x00001000u,
    SDL_INIT_GAMEPAD    = 0x00002000u,
    SDL_INIT_EVENTS     = 0x00004000u,
    SDL_INIT_SENSOR     = 0x00008000u, 
    SDL_INIT_CAMERA     = 0x00010000u 
} SDL_InitFlags;

typedef struct SDL_Rect
{
    int x, y;
    int w, h;
} SDL_Rect;

typedef struct SDL_FRect
{
    float x;
    float y;
    float w;
    float h;
} SDL_FRect;

typedef struct SDL_Point
{
    int x;
    int y;
} SDL_Point;

typedef struct SDL_FPoint
{
    float x;
    float y;
} SDL_FPoint;

typedef struct SDL_Color
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} SDL_Color;

typedef struct SDL_FColor
{
    float r;
    float g;
    float b;
    float a;
} SDL_FColor;

typedef struct SDL_Palette
{
    int ncolors;        /**< number of elements in `colors`. */
    SDL_Color *colors;  /**< an array of colors, `ncolors` long. */
    Uint32 version;     /**< internal use only, do not touch. */
    int refcount;       /**< internal use only, do not touch. */
} SDL_Palette;

typedef enum SDL_PixelFormat
{
    SDL_PIXELFORMAT_UNKNOWN = 0,
    SDL_PIXELFORMAT_INDEX1LSB = 0x11100100u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX1, SDL_BITMAPORDER_4321, 0, 1, 0), */
    SDL_PIXELFORMAT_INDEX1MSB = 0x11200100u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX1, SDL_BITMAPORDER_1234, 0, 1, 0), */
    SDL_PIXELFORMAT_INDEX2LSB = 0x1c100200u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX2, SDL_BITMAPORDER_4321, 0, 2, 0), */
    SDL_PIXELFORMAT_INDEX2MSB = 0x1c200200u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX2, SDL_BITMAPORDER_1234, 0, 2, 0), */
    SDL_PIXELFORMAT_INDEX4LSB = 0x12100400u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX4, SDL_BITMAPORDER_4321, 0, 4, 0), */
    SDL_PIXELFORMAT_INDEX4MSB = 0x12200400u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX4, SDL_BITMAPORDER_1234, 0, 4, 0), */
    SDL_PIXELFORMAT_INDEX8 = 0x13000801u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX8, 0, 0, 8, 1), */
    SDL_PIXELFORMAT_RGB332 = 0x14110801u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED8, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_332, 8, 1), */
    SDL_PIXELFORMAT_XRGB4444 = 0x15120c02u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_4444, 12, 2), */
    SDL_PIXELFORMAT_XBGR4444 = 0x15520c02u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_4444, 12, 2), */
    SDL_PIXELFORMAT_XRGB1555 = 0x15130f02u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_1555, 15, 2), */
    SDL_PIXELFORMAT_XBGR1555 = 0x15530f02u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_1555, 15, 2), */
    SDL_PIXELFORMAT_ARGB4444 = 0x15321002u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ARGB, SDL_PACKEDLAYOUT_4444, 16, 2), */
    SDL_PIXELFORMAT_RGBA4444 = 0x15421002u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_RGBA, SDL_PACKEDLAYOUT_4444, 16, 2), */
    SDL_PIXELFORMAT_ABGR4444 = 0x15721002u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ABGR, SDL_PACKEDLAYOUT_4444, 16, 2), */
    SDL_PIXELFORMAT_BGRA4444 = 0x15821002u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_BGRA, SDL_PACKEDLAYOUT_4444, 16, 2), */
    SDL_PIXELFORMAT_ARGB1555 = 0x15331002u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ARGB, SDL_PACKEDLAYOUT_1555, 16, 2), */
    SDL_PIXELFORMAT_RGBA5551 = 0x15441002u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_RGBA, SDL_PACKEDLAYOUT_5551, 16, 2), */
    SDL_PIXELFORMAT_ABGR1555 = 0x15731002u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ABGR, SDL_PACKEDLAYOUT_1555, 16, 2), */
    SDL_PIXELFORMAT_BGRA5551 = 0x15841002u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_BGRA, SDL_PACKEDLAYOUT_5551, 16, 2), */
    SDL_PIXELFORMAT_RGB565 = 0x15151002u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_565, 16, 2), */
    SDL_PIXELFORMAT_BGR565 = 0x15551002u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_565, 16, 2), */
    SDL_PIXELFORMAT_RGB24 = 0x17101803u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU8, SDL_ARRAYORDER_RGB, 0, 24, 3), */
    SDL_PIXELFORMAT_BGR24 = 0x17401803u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU8, SDL_ARRAYORDER_BGR, 0, 24, 3), */
    SDL_PIXELFORMAT_XRGB8888 = 0x16161804u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_8888, 24, 4), */
    SDL_PIXELFORMAT_RGBX8888 = 0x16261804u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_RGBX, SDL_PACKEDLAYOUT_8888, 24, 4), */
    SDL_PIXELFORMAT_XBGR8888 = 0x16561804u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_8888, 24, 4), */
    SDL_PIXELFORMAT_BGRX8888 = 0x16661804u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_BGRX, SDL_PACKEDLAYOUT_8888, 24, 4), */
    SDL_PIXELFORMAT_ARGB8888 = 0x16362004u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ARGB, SDL_PACKEDLAYOUT_8888, 32, 4), */
    SDL_PIXELFORMAT_RGBA8888 = 0x16462004u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_RGBA, SDL_PACKEDLAYOUT_8888, 32, 4), */
    SDL_PIXELFORMAT_ABGR8888 = 0x16762004u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ABGR, SDL_PACKEDLAYOUT_8888, 32, 4), */
    SDL_PIXELFORMAT_BGRA8888 = 0x16862004u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_BGRA, SDL_PACKEDLAYOUT_8888, 32, 4), */
    SDL_PIXELFORMAT_XRGB2101010 = 0x16172004u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_2101010, 32, 4), */
    SDL_PIXELFORMAT_XBGR2101010 = 0x16572004u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_2101010, 32, 4), */
    SDL_PIXELFORMAT_ARGB2101010 = 0x16372004u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ARGB, SDL_PACKEDLAYOUT_2101010, 32, 4), */
    SDL_PIXELFORMAT_ABGR2101010 = 0x16772004u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ABGR, SDL_PACKEDLAYOUT_2101010, 32, 4), */
    SDL_PIXELFORMAT_RGB48 = 0x18103006u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU16, SDL_ARRAYORDER_RGB, 0, 48, 6), */
    SDL_PIXELFORMAT_BGR48 = 0x18403006u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU16, SDL_ARRAYORDER_BGR, 0, 48, 6), */
    SDL_PIXELFORMAT_RGBA64 = 0x18204008u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU16, SDL_ARRAYORDER_RGBA, 0, 64, 8), */
    SDL_PIXELFORMAT_ARGB64 = 0x18304008u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU16, SDL_ARRAYORDER_ARGB, 0, 64, 8), */
    SDL_PIXELFORMAT_BGRA64 = 0x18504008u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU16, SDL_ARRAYORDER_BGRA, 0, 64, 8), */
    SDL_PIXELFORMAT_ABGR64 = 0x18604008u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU16, SDL_ARRAYORDER_ABGR, 0, 64, 8), */
    SDL_PIXELFORMAT_RGB48_FLOAT = 0x1a103006u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF16, SDL_ARRAYORDER_RGB, 0, 48, 6), */
    SDL_PIXELFORMAT_BGR48_FLOAT = 0x1a403006u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF16, SDL_ARRAYORDER_BGR, 0, 48, 6), */
    SDL_PIXELFORMAT_RGBA64_FLOAT = 0x1a204008u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF16, SDL_ARRAYORDER_RGBA, 0, 64, 8), */
    SDL_PIXELFORMAT_ARGB64_FLOAT = 0x1a304008u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF16, SDL_ARRAYORDER_ARGB, 0, 64, 8), */
    SDL_PIXELFORMAT_BGRA64_FLOAT = 0x1a504008u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF16, SDL_ARRAYORDER_BGRA, 0, 64, 8), */
    SDL_PIXELFORMAT_ABGR64_FLOAT = 0x1a604008u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF16, SDL_ARRAYORDER_ABGR, 0, 64, 8), */
    SDL_PIXELFORMAT_RGB96_FLOAT = 0x1b10600cu,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF32, SDL_ARRAYORDER_RGB, 0, 96, 12), */
    SDL_PIXELFORMAT_BGR96_FLOAT = 0x1b40600cu,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF32, SDL_ARRAYORDER_BGR, 0, 96, 12), */
    SDL_PIXELFORMAT_RGBA128_FLOAT = 0x1b208010u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF32, SDL_ARRAYORDER_RGBA, 0, 128, 16), */
    SDL_PIXELFORMAT_ARGB128_FLOAT = 0x1b308010u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF32, SDL_ARRAYORDER_ARGB, 0, 128, 16), */
    SDL_PIXELFORMAT_BGRA128_FLOAT = 0x1b508010u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF32, SDL_ARRAYORDER_BGRA, 0, 128, 16), */
    SDL_PIXELFORMAT_ABGR128_FLOAT = 0x1b608010u,
        /* SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYF32, SDL_ARRAYORDER_ABGR, 0, 128, 16), */

    SDL_PIXELFORMAT_YV12 = 0x32315659u,      /**< Planar mode: Y + V + U  (3 planes) */
        /* SDL_DEFINE_PIXELFOURCC('Y', 'V', '1', '2'), */
    SDL_PIXELFORMAT_IYUV = 0x56555949u,      /**< Planar mode: Y + U + V  (3 planes) */
        /* SDL_DEFINE_PIXELFOURCC('I', 'Y', 'U', 'V'), */
    SDL_PIXELFORMAT_YUY2 = 0x32595559u,      /**< Packed mode: Y0+U0+Y1+V0 (1 plane) */
        /* SDL_DEFINE_PIXELFOURCC('Y', 'U', 'Y', '2'), */
    SDL_PIXELFORMAT_UYVY = 0x59565955u,      /**< Packed mode: U0+Y0+V0+Y1 (1 plane) */
        /* SDL_DEFINE_PIXELFOURCC('U', 'Y', 'V', 'Y'), */
    SDL_PIXELFORMAT_YVYU = 0x55595659u,      /**< Packed mode: Y0+V0+Y1+U0 (1 plane) */
        /* SDL_DEFINE_PIXELFOURCC('Y', 'V', 'Y', 'U'), */
    SDL_PIXELFORMAT_NV12 = 0x3231564eu,      /**< Planar mode: Y + U/V interleaved  (2 planes) */
        /* SDL_DEFINE_PIXELFOURCC('N', 'V', '1', '2'), */
    SDL_PIXELFORMAT_NV21 = 0x3132564eu,      /**< Planar mode: Y + V/U interleaved  (2 planes) */
        /* SDL_DEFINE_PIXELFOURCC('N', 'V', '2', '1'), */
    SDL_PIXELFORMAT_P010 = 0x30313050u,      /**< Planar mode: Y + U/V interleaved  (2 planes) */
        /* SDL_DEFINE_PIXELFOURCC('P', '0', '1', '0'), */
    SDL_PIXELFORMAT_EXTERNAL_OES = 0x2053454fu,     /**< Android video texture format */
        /* SDL_DEFINE_PIXELFOURCC('O', 'E', 'S', ' ') */

    SDL_PIXELFORMAT_MJPG = 0x47504a4du,     /**< Motion JPEG */
        /* SDL_DEFINE_PIXELFOURCC('M', 'J', 'P', 'G') */

    /* Aliases for RGBA byte arrays of color data, for the current platform */
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    SDL_PIXELFORMAT_RGBA32 = SDL_PIXELFORMAT_RGBA8888,
    SDL_PIXELFORMAT_ARGB32 = SDL_PIXELFORMAT_ARGB8888,
    SDL_PIXELFORMAT_BGRA32 = SDL_PIXELFORMAT_BGRA8888,
    SDL_PIXELFORMAT_ABGR32 = SDL_PIXELFORMAT_ABGR8888,
    SDL_PIXELFORMAT_RGBX32 = SDL_PIXELFORMAT_RGBX8888,
    SDL_PIXELFORMAT_XRGB32 = SDL_PIXELFORMAT_XRGB8888,
    SDL_PIXELFORMAT_BGRX32 = SDL_PIXELFORMAT_BGRX8888,
    SDL_PIXELFORMAT_XBGR32 = SDL_PIXELFORMAT_XBGR8888
    #else
    SDL_PIXELFORMAT_RGBA32 = SDL_PIXELFORMAT_ABGR8888,
    SDL_PIXELFORMAT_ARGB32 = SDL_PIXELFORMAT_BGRA8888,
    SDL_PIXELFORMAT_BGRA32 = SDL_PIXELFORMAT_ARGB8888,
    SDL_PIXELFORMAT_ABGR32 = SDL_PIXELFORMAT_RGBA8888,
    SDL_PIXELFORMAT_RGBX32 = SDL_PIXELFORMAT_XBGR8888,
    SDL_PIXELFORMAT_XRGB32 = SDL_PIXELFORMAT_BGRX8888,
    SDL_PIXELFORMAT_BGRX32 = SDL_PIXELFORMAT_XRGB8888,
    SDL_PIXELFORMAT_XBGR32 = SDL_PIXELFORMAT_RGBX8888
    #endif
} SDL_PixelFormat;

typedef struct SDL_PixelFormatDetails
{
    SDL_PixelFormat format;
    Uint8 bits_per_pixel;
    Uint8 bytes_per_pixel;
    Uint8 padding[2];
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    Uint32 Amask;
    Uint8 Rbits;
    Uint8 Gbits;
    Uint8 Bbits;
    Uint8 Abits;
    Uint8 Rshift;
    Uint8 Gshift;
    Uint8 Bshift;
    Uint8 Ashift;
} SDL_PixelFormatDetails;

typedef enum SDL_SurfaceFlags
{
    SDL_SURFACE_PREALLOCATED    = 0x00000001u, /**< Surface uses preallocated pixel memory */
    SDL_SURFACE_LOCK_NEEDED     = 0x00000002u, /**< Surface needs to be locked to access pixels */
    SDL_SURFACE_LOCKED          = 0x00000004u, /**< Surface is currently locked */
    SDL_SURFACE_SIMD_ALIGNED    = 0x00000008u  /**< Surface uses pixel memory allocated with SDL_aligned_alloc() */
} SDL_SurfaceFlags;

struct SDL_Surface
{
    SDL_SurfaceFlags flags;     /**< The flags of the surface, read-only */
    SDL_PixelFormat format;     /**< The format of the surface, read-only */
    int w;                      /**< The width of the surface, read-only. */
    int h;                      /**< The height of the surface, read-only. */
    int pitch;                  /**< The distance in bytes between rows of pixels, read-only */
    void *pixels;               /**< A pointer to the pixels of the surface, the pixels are writeable if non-NULL */

    int refcount;               /**< Application reference count, used when freeing surface */

    void *reserved;             /**< Reserved for internal use */
};

typedef enum SDL_RendererLogicalPresentation
{
    SDL_LOGICAL_PRESENTATION_DISABLED,  /**< There is no logical size in effect */
    SDL_LOGICAL_PRESENTATION_STRETCH,   /**< The rendered content is stretched to the output resolution */
    SDL_LOGICAL_PRESENTATION_LETTERBOX, /**< The rendered content is fit to the largest dimension and the other dimension is letterboxed with the clear color */
    SDL_LOGICAL_PRESENTATION_OVERSCAN,  /**< The rendered content is fit to the smallest dimension and the other dimension extends beyond the output bounds */
    SDL_LOGICAL_PRESENTATION_INTEGER_SCALE   /**< The rendered content is scaled up by integer multiples to fit the output resolution */
} SDL_RendererLogicalPresentation;

typedef Uint64 SDL_WindowFlags;

enum SDL_WindowFlagsEnum : Uint64
{
    SDL_WINDOW_FULLSCREEN                 = 0x0000000000000001,    /**< window is in fullscreen mode */
    SDL_WINDOW_OPENGL                     = 0x0000000000000002,    /**< window usable with OpenGL context */
    SDL_WINDOW_OCCLUDED                   = 0x0000000000000004,    /**< window is occluded */
    SDL_WINDOW_HIDDEN                     = 0x0000000000000008,    /**< window is neither mapped onto the desktop nor shown in the taskbar/dock/window list; SDL_ShowWindow(, is required for it to become visible */
    SDL_WINDOW_BORDERLESS                 = 0x0000000000000010,    /**< no window decoration */
    SDL_WINDOW_RESIZABLE                  = 0x0000000000000020,    /**< window can be resized */
    SDL_WINDOW_MINIMIZED                  = 0x0000000000000040,    /**< window is minimized */
    SDL_WINDOW_MAXIMIZED                  = 0x0000000000000080,    /**< window is maximized */
    SDL_WINDOW_MOUSE_GRABBED              = 0x0000000000000100,    /**< window has grabbed mouse input */
    SDL_WINDOW_INPUT_FOCUS                = 0x0000000000000200,    /**< window has input focus */
    SDL_WINDOW_MOUSE_FOCUS                = 0x0000000000000400,    /**< window has mouse focus */
    SDL_WINDOW_EXTERNAL                   = 0x0000000000000800,    /**< window not created by SDL */
    SDL_WINDOW_MODAL                      = 0x0000000000001000,    /**< window is modal */
    SDL_WINDOW_HIGH_PIXEL_DENSITY         = 0x0000000000002000,    /**< window uses high pixel density back buffer if possible */
    SDL_WINDOW_MOUSE_CAPTURE              = 0x0000000000004000,    /**< window has mouse captured (unrelated to MOUSE_GRABBED, */
    SDL_WINDOW_MOUSE_RELATIVE_MODE        = 0x0000000000008000,    /**< window has relative mode enabled */
    SDL_WINDOW_ALWAYS_ON_TOP              = 0x0000000000010000,    /**< window should always be above others */
    SDL_WINDOW_UTILITY                    = 0x0000000000020000,    /**< window should be treated as a utility window, not showing in the task bar and window list */
    SDL_WINDOW_TOOLTIP                    = 0x0000000000040000,    /**< window should be treated as a tooltip and does not get mouse or keyboard focus, requires a parent window */
    SDL_WINDOW_POPUP_MENU                 = 0x0000000000080000,    /**< window should be treated as a popup menu, requires a parent window */
    SDL_WINDOW_KEYBOARD_GRABBED           = 0x0000000000100000,    /**< window has grabbed keyboard input */
    SDL_WINDOW_VULKAN                     = 0x0000000010000000,    /**< window usable for Vulkan surface */
    SDL_WINDOW_METAL                      = 0x0000000020000000,    /**< window usable for Metal view */
    SDL_WINDOW_TRANSPARENT                = 0x0000000040000000,    /**< window with transparent buffer */
    SDL_WINDOW_NOT_FOCUSABLE              = 0x0000000080000000,    /**< window should not be focusable */
};

typedef enum SDL_Scancode
{
    SDL_SCANCODE_UNKNOWN = 0,
    SDL_SCANCODE_A = 4,
    SDL_SCANCODE_B = 5,
    SDL_SCANCODE_C = 6,
    SDL_SCANCODE_D = 7,
    SDL_SCANCODE_E = 8,
    SDL_SCANCODE_F = 9,
    SDL_SCANCODE_G = 10,
    SDL_SCANCODE_H = 11,
    SDL_SCANCODE_I = 12,
    SDL_SCANCODE_J = 13,
    SDL_SCANCODE_K = 14,
    SDL_SCANCODE_L = 15,
    SDL_SCANCODE_M = 16,
    SDL_SCANCODE_N = 17,
    SDL_SCANCODE_O = 18,
    SDL_SCANCODE_P = 19,
    SDL_SCANCODE_Q = 20,
    SDL_SCANCODE_R = 21,
    SDL_SCANCODE_S = 22,
    SDL_SCANCODE_T = 23,
    SDL_SCANCODE_U = 24,
    SDL_SCANCODE_V = 25,
    SDL_SCANCODE_W = 26,
    SDL_SCANCODE_X = 27,
    SDL_SCANCODE_Y = 28,
    SDL_SCANCODE_Z = 29,
    SDL_SCANCODE_1 = 30,
    SDL_SCANCODE_2 = 31,
    SDL_SCANCODE_3 = 32,
    SDL_SCANCODE_4 = 33,
    SDL_SCANCODE_5 = 34,
    SDL_SCANCODE_6 = 35,
    SDL_SCANCODE_7 = 36,
    SDL_SCANCODE_8 = 37,
    SDL_SCANCODE_9 = 38,
    SDL_SCANCODE_0 = 39,
    SDL_SCANCODE_RETURN = 40,
    SDL_SCANCODE_ESCAPE = 41,
    SDL_SCANCODE_BACKSPACE = 42,
    SDL_SCANCODE_TAB = 43,
    SDL_SCANCODE_SPACE = 44,
    SDL_SCANCODE_MINUS = 45,
    SDL_SCANCODE_EQUALS = 46,
    SDL_SCANCODE_LEFTBRACKET = 47,
    SDL_SCANCODE_RIGHTBRACKET = 48,
    SDL_SCANCODE_BACKSLASH = 49,
    SDL_SCANCODE_NONUSHASH = 50,
    SDL_SCANCODE_SEMICOLON = 51,
    SDL_SCANCODE_APOSTROPHE = 52,
    SDL_SCANCODE_GRAVE = 53,
    SDL_SCANCODE_COMMA = 54,
    SDL_SCANCODE_PERIOD = 55,
    SDL_SCANCODE_SLASH = 56,
    SDL_SCANCODE_CAPSLOCK = 57,
    SDL_SCANCODE_F1 = 58,
    SDL_SCANCODE_F2 = 59,
    SDL_SCANCODE_F3 = 60,
    SDL_SCANCODE_F4 = 61,
    SDL_SCANCODE_F5 = 62,
    SDL_SCANCODE_F6 = 63,
    SDL_SCANCODE_F7 = 64,
    SDL_SCANCODE_F8 = 65,
    SDL_SCANCODE_F9 = 66,
    SDL_SCANCODE_F10 = 67,
    SDL_SCANCODE_F11 = 68,
    SDL_SCANCODE_F12 = 69,
    SDL_NUM_SCANCODES = 512
} SDL_Scancode;

typedef struct SDL_Keysym
{
    SDL_Scancode scancode;
    SDL_Keycode sym;
    Uint16 mod;
    Uint32 unused;
} SDL_Keysym;

typedef Uint32 SDL_MouseButtonFlags;

/**
 * Pen input flags, as reported by various pen events' `pen_state` field.
 *
 * \since This datatype is available since SDL 3.2.0.
 */
typedef enum SDL_PenInputFlags
{
    SDL_PEN_INPUT_DOWN         = (1u << 0),  /**< pen is pressed down */
    SDL_PEN_INPUT_BUTTON_1     = (1u << 1),  /**< button 1 is pressed */
    SDL_PEN_INPUT_BUTTON_2     = (1u << 2),  /**< button 2 is pressed */
    SDL_PEN_INPUT_BUTTON_3     = (1u << 3),  /**< button 3 is pressed */
    SDL_PEN_INPUT_BUTTON_4     = (1u << 4),  /**< button 4 is pressed */
    SDL_PEN_INPUT_BUTTON_5     = (1u << 5),  /**< button 5 is pressed */
    SDL_PEN_INPUT_ERASER_TIP   = (1u << 30), /**< eraser tip is used */
    SDL_PEN_INPUT_IN_PROXIMITY = (1u << 31)  /**< pen is in proximity (since SDL 3.4.0) */
} SDL_PenInputFlags;

typedef enum SDL_PenAxis
{
    SDL_PEN_AXIS_PRESSURE,  /**< Pen pressure.  Unidirectional: 0 to 1.0 */
    SDL_PEN_AXIS_XTILT,     /**< Pen horizontal tilt angle.  Bidirectional: -90.0 to 90.0 (left-to-right). */
    SDL_PEN_AXIS_YTILT,     /**< Pen vertical tilt angle.  Bidirectional: -90.0 to 90.0 (top-to-down). */
    SDL_PEN_AXIS_DISTANCE,  /**< Pen distance to drawing surface.  Unidirectional: 0.0 to 1.0 */
    SDL_PEN_AXIS_ROTATION,  /**< Pen barrel rotation.  Bidirectional: -180 to 179.9 (clockwise, 0 is facing up, -180.0 is facing down). */
    SDL_PEN_AXIS_SLIDER,    /**< Pen finger wheel or slider (e.g., Airbrush Pen).  Unidirectional: 0 to 1.0 */
    SDL_PEN_AXIS_TANGENTIAL_PRESSURE,    /**< Pressure from squeezing the pen ("barrel pressure"). */
    SDL_PEN_AXIS_COUNT       /**< Total known pen axis types in this version of SDL. This number may grow in future releases! */
} SDL_PenAxis;

typedef enum SDL_MouseWheelDirection
{
    SDL_MOUSEWHEEL_NORMAL,    /**< The scroll direction is normal */
    SDL_MOUSEWHEEL_FLIPPED    /**< The scroll direction is flipped / natural */
} SDL_MouseWheelDirection;

typedef enum SDL_PowerState
{
    SDL_POWERSTATE_UNKNOWN = 0,
    SDL_POWERSTATE_ON_BATTERY,
    SDL_POWERSTATE_NO_BATTERY,
    SDL_POWERSTATE_CHARGING,
    SDL_POWERSTATE_CHARGED
} SDL_PowerState;

typedef enum SDL_ProgressState
{
    SDL_PROGRESS_STATE_INVALID = -1,    /**< An invalid progress state indicating an error; check SDL_GetError() */
    SDL_PROGRESS_STATE_NONE,            /**< No progress bar is shown */
    SDL_PROGRESS_STATE_INDETERMINATE,   /**< The progress bar is shown in a indeterminate state */
    SDL_PROGRESS_STATE_NORMAL,          /**< The progress bar is shown in a normal state */
    SDL_PROGRESS_STATE_PAUSED,          /**< The progress bar is shown in a paused state */
    SDL_PROGRESS_STATE_ERROR            /**< The progress bar is shown in a state indicating the application had an error */
} SDL_ProgressState;

typedef int SDL_SensorID;

typedef enum SDL_EventType
{
    SDL_EVENT_FIRST     = 0,     /**< Unused (do not remove) */

    /* Application events */
    SDL_EVENT_QUIT           = 0x100, /**< User-requested quit */

    /* These application events have special meaning on iOS and Android, see README-ios.md and README-android.md for details */
    SDL_EVENT_TERMINATING,      /**< The application is being terminated by the OS. This event must be handled in a callback set with SDL_AddEventWatch().
                                     Called on iOS in applicationWillTerminate()
                                     Called on Android in onDestroy()
                                */
    SDL_EVENT_LOW_MEMORY,       /**< The application is low on memory, free memory if possible. This event must be handled in a callback set with SDL_AddEventWatch().
                                     Called on iOS in applicationDidReceiveMemoryWarning()
                                     Called on Android in onTrimMemory()
                                */
    SDL_EVENT_WILL_ENTER_BACKGROUND, /**< The application is about to enter the background. This event must be handled in a callback set with SDL_AddEventWatch().
                                     Called on iOS in applicationWillResignActive()
                                     Called on Android in onPause()
                                */
    SDL_EVENT_DID_ENTER_BACKGROUND, /**< The application did enter the background and may not get CPU for some time. This event must be handled in a callback set with SDL_AddEventWatch().
                                     Called on iOS in applicationDidEnterBackground()
                                     Called on Android in onPause()
                                */
    SDL_EVENT_WILL_ENTER_FOREGROUND, /**< The application is about to enter the foreground. This event must be handled in a callback set with SDL_AddEventWatch().
                                     Called on iOS in applicationWillEnterForeground()
                                     Called on Android in onResume()
                                */
    SDL_EVENT_DID_ENTER_FOREGROUND, /**< The application is now interactive. This event must be handled in a callback set with SDL_AddEventWatch().
                                     Called on iOS in applicationDidBecomeActive()
                                     Called on Android in onResume()
                                */

    SDL_EVENT_LOCALE_CHANGED,  /**< The user's locale preferences have changed. */

    SDL_EVENT_SYSTEM_THEME_CHANGED, /**< The system theme changed */

    /* Display events */
    /* 0x150 was SDL_DISPLAYEVENT, reserve the number for sdl2-compat */
    SDL_EVENT_DISPLAY_ORIENTATION = 0x151,   /**< Display orientation has changed to data1 */
    SDL_EVENT_DISPLAY_ADDED,                 /**< Display has been added to the system */
    SDL_EVENT_DISPLAY_REMOVED,               /**< Display has been removed from the system */
    SDL_EVENT_DISPLAY_MOVED,                 /**< Display has changed position */
    SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED,  /**< Display has changed desktop mode */
    SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED,  /**< Display has changed current mode */
    SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED, /**< Display has changed content scale */
    SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED, /**< Display has changed usable bounds */
    SDL_EVENT_DISPLAY_FIRST = SDL_EVENT_DISPLAY_ORIENTATION,
    SDL_EVENT_DISPLAY_LAST = SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED,

    /* Window events */
    /* 0x200 was SDL_WINDOWEVENT, reserve the number for sdl2-compat */
    /* 0x201 was SDL_SYSWMEVENT, reserve the number for sdl2-compat */
    SDL_EVENT_WINDOW_SHOWN = 0x202,     /**< Window has been shown */
    SDL_EVENT_WINDOW_HIDDEN,            /**< Window has been hidden */
    SDL_EVENT_WINDOW_EXPOSED,           /**< Window has been exposed and should be redrawn, and can be redrawn directly from event watchers for this event.
                                             data1 is 1 for live-resize expose events, 0 otherwise. */
    SDL_EVENT_WINDOW_MOVED,             /**< Window has been moved to data1, data2 */
    SDL_EVENT_WINDOW_RESIZED,           /**< Window has been resized to data1xdata2 */
    SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED,/**< The pixel size of the window has changed to data1xdata2 */
    SDL_EVENT_WINDOW_METAL_VIEW_RESIZED,/**< The pixel size of a Metal view associated with the window has changed */
    SDL_EVENT_WINDOW_MINIMIZED,         /**< Window has been minimized */
    SDL_EVENT_WINDOW_MAXIMIZED,         /**< Window has been maximized */
    SDL_EVENT_WINDOW_RESTORED,          /**< Window has been restored to normal size and position */
    SDL_EVENT_WINDOW_MOUSE_ENTER,       /**< Window has gained mouse focus */
    SDL_EVENT_WINDOW_MOUSE_LEAVE,       /**< Window has lost mouse focus */
    SDL_EVENT_WINDOW_FOCUS_GAINED,      /**< Window has gained keyboard focus */
    SDL_EVENT_WINDOW_FOCUS_LOST,        /**< Window has lost keyboard focus */
    SDL_EVENT_WINDOW_CLOSE_REQUESTED,   /**< The window manager requests that the window be closed */
    SDL_EVENT_WINDOW_HIT_TEST,          /**< Window had a hit test that wasn't SDL_HITTEST_NORMAL */
    SDL_EVENT_WINDOW_ICCPROF_CHANGED,   /**< The window's ICC profile has changed */
    SDL_EVENT_WINDOW_DISPLAY_CHANGED,   /**< Window has been moved to display data1 */
    SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED, /**< Window display scale has been changed */
    SDL_EVENT_WINDOW_SAFE_AREA_CHANGED, /**< The window safe area has been changed */
    SDL_EVENT_WINDOW_OCCLUDED,          /**< The window has been occluded */
    SDL_EVENT_WINDOW_ENTER_FULLSCREEN,  /**< The window has entered fullscreen mode */
    SDL_EVENT_WINDOW_LEAVE_FULLSCREEN,  /**< The window has left fullscreen mode */
    SDL_EVENT_WINDOW_DESTROYED,         /**< The window with the associated ID is being or has been destroyed. If this message is being handled
                                             in an event watcher, the window handle is still valid and can still be used to retrieve any properties
                                             associated with the window. Otherwise, the handle has already been destroyed and all resources
                                             associated with it are invalid */
    SDL_EVENT_WINDOW_HDR_STATE_CHANGED, /**< Window HDR properties have changed */
    SDL_EVENT_WINDOW_CURVATURE_CHANGED, /**< Window curvature has changed to data1 (on visionOS) */
    SDL_EVENT_WINDOW_FIRST = SDL_EVENT_WINDOW_SHOWN,
    SDL_EVENT_WINDOW_LAST = SDL_EVENT_WINDOW_CURVATURE_CHANGED,

    /* Keyboard events */
    SDL_EVENT_KEY_DOWN        = 0x300, /**< Key pressed */
    SDL_EVENT_KEY_UP,                  /**< Key released */
    SDL_EVENT_TEXT_EDITING,            /**< Keyboard text editing (composition) */
    SDL_EVENT_TEXT_INPUT,              /**< Keyboard text input */
    SDL_EVENT_KEYMAP_CHANGED,          /**< Keymap changed due to a system event such as an
                                            input language or keyboard layout change. */
    SDL_EVENT_KEYBOARD_ADDED,          /**< A new keyboard has been inserted into the system */
    SDL_EVENT_KEYBOARD_REMOVED,        /**< A keyboard has been removed */
    SDL_EVENT_TEXT_EDITING_CANDIDATES, /**< Keyboard text editing candidates */
    SDL_EVENT_SCREEN_KEYBOARD_SHOWN,   /**< The on-screen keyboard has been shown */
    SDL_EVENT_SCREEN_KEYBOARD_HIDDEN,  /**< The on-screen keyboard has been hidden */

    /* Mouse events */
    SDL_EVENT_MOUSE_MOTION    = 0x400, /**< Mouse moved */
    SDL_EVENT_MOUSE_BUTTON_DOWN,       /**< Mouse button pressed */
    SDL_EVENT_MOUSE_BUTTON_UP,         /**< Mouse button released */
    SDL_EVENT_MOUSE_WHEEL,             /**< Mouse wheel motion */
    SDL_EVENT_MOUSE_ADDED,             /**< A new mouse has been inserted into the system */
    SDL_EVENT_MOUSE_REMOVED,           /**< A mouse has been removed */

    /* Joystick events */
    SDL_EVENT_JOYSTICK_AXIS_MOTION  = 0x600, /**< Joystick axis motion */
    SDL_EVENT_JOYSTICK_BALL_MOTION,          /**< Joystick trackball motion */
    SDL_EVENT_JOYSTICK_HAT_MOTION,           /**< Joystick hat position change */
    SDL_EVENT_JOYSTICK_BUTTON_DOWN,          /**< Joystick button pressed */
    SDL_EVENT_JOYSTICK_BUTTON_UP,            /**< Joystick button released */
    SDL_EVENT_JOYSTICK_ADDED,                /**< A new joystick has been inserted into the system */
    SDL_EVENT_JOYSTICK_REMOVED,              /**< An opened joystick has been removed */
    SDL_EVENT_JOYSTICK_BATTERY_UPDATED,      /**< Joystick battery level change */
    SDL_EVENT_JOYSTICK_UPDATE_COMPLETE,      /**< Joystick update is complete */

    /* Gamepad events */
    SDL_EVENT_GAMEPAD_AXIS_MOTION  = 0x650, /**< Gamepad axis motion */
    SDL_EVENT_GAMEPAD_BUTTON_DOWN,          /**< Gamepad button pressed */
    SDL_EVENT_GAMEPAD_BUTTON_UP,            /**< Gamepad button released */
    SDL_EVENT_GAMEPAD_ADDED,                /**< A new gamepad has been inserted into the system */
    SDL_EVENT_GAMEPAD_REMOVED,              /**< A gamepad has been removed */
    SDL_EVENT_GAMEPAD_REMAPPED,             /**< The gamepad mapping was updated */
    SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN,        /**< Gamepad touchpad was touched */
    SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION,      /**< Gamepad touchpad finger was moved */
    SDL_EVENT_GAMEPAD_TOUCHPAD_UP,          /**< Gamepad touchpad finger was lifted */
    SDL_EVENT_GAMEPAD_SENSOR_UPDATE,        /**< Gamepad sensor was updated */
    SDL_EVENT_GAMEPAD_UPDATE_COMPLETE,      /**< Gamepad update is complete */
    SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED,  /**< Gamepad Steam handle has changed */
    SDL_EVENT_GAMEPAD_CAPSENSE_TOUCH,       /**< Gamepad capsense was touched */
    SDL_EVENT_GAMEPAD_CAPSENSE_RELEASE,     /**< Gamepad capsense was released */

    /* Touch events */
    SDL_EVENT_FINGER_DOWN      = 0x700,
    SDL_EVENT_FINGER_UP,
    SDL_EVENT_FINGER_MOTION,
    SDL_EVENT_FINGER_CANCELED,

    /* Pinch events */
    SDL_EVENT_PINCH_BEGIN      = 0x710,     /**< Pinch gesture started */
    SDL_EVENT_PINCH_UPDATE,                 /**< Pinch gesture updated */
    SDL_EVENT_PINCH_END,                    /**< Pinch gesture ended */

    /* 0x800, 0x801, and 0x802 were the Gesture events from SDL2. Do not reuse these values! sdl2-compat needs them! */

    /* Clipboard events */
    SDL_EVENT_CLIPBOARD_UPDATE = 0x900, /**< The clipboard changed */

    /* Drag and drop events */
    SDL_EVENT_DROP_FILE        = 0x1000, /**< The system requests a file open */
    SDL_EVENT_DROP_TEXT,                 /**< text/plain drag-and-drop event */
    SDL_EVENT_DROP_BEGIN,                /**< A new set of drops is beginning (NULL filename) */
    SDL_EVENT_DROP_COMPLETE,             /**< Current set of drops is now complete (NULL filename) */
    SDL_EVENT_DROP_POSITION,             /**< Position while moving over the window */

    /* Audio hotplug events */
    SDL_EVENT_AUDIO_DEVICE_ADDED = 0x1100,  /**< A new audio device is available */
    SDL_EVENT_AUDIO_DEVICE_REMOVED,         /**< An audio device has been removed. */
    SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED,  /**< An audio device's format has been changed by the system. */

    /* Sensor events */
    SDL_EVENT_SENSOR_UPDATE = 0x1200,     /**< A sensor was updated */

    /* Pressure-sensitive pen events */
    SDL_EVENT_PEN_PROXIMITY_IN = 0x1300,  /**< Pressure-sensitive pen has become available */
    SDL_EVENT_PEN_PROXIMITY_OUT,          /**< Pressure-sensitive pen has become unavailable */
    SDL_EVENT_PEN_DOWN,                   /**< Pressure-sensitive pen touched drawing surface */
    SDL_EVENT_PEN_UP,                     /**< Pressure-sensitive pen stopped touching drawing surface */
    SDL_EVENT_PEN_BUTTON_DOWN,            /**< Pressure-sensitive pen button pressed */
    SDL_EVENT_PEN_BUTTON_UP,              /**< Pressure-sensitive pen button released */
    SDL_EVENT_PEN_MOTION,                 /**< Pressure-sensitive pen is moving on the tablet */
    SDL_EVENT_PEN_AXIS,                   /**< Pressure-sensitive pen angle/pressure/etc changed */

    /* Camera hotplug events */
    SDL_EVENT_CAMERA_DEVICE_ADDED = 0x1400,  /**< A new camera device is available */
    SDL_EVENT_CAMERA_DEVICE_REMOVED,         /**< A camera device has been removed. */
    SDL_EVENT_CAMERA_DEVICE_APPROVED,        /**< A camera device has been approved for use by the user. */
    SDL_EVENT_CAMERA_DEVICE_DENIED,          /**< A camera device has been denied for use by the user. */

    /* Render events */
    SDL_EVENT_RENDER_TARGETS_RESET = 0x2000, /**< The render targets have been reset and their contents need to be updated */
    SDL_EVENT_RENDER_DEVICE_RESET, /**< The device has been reset and all textures need to be recreated */
    SDL_EVENT_RENDER_DEVICE_LOST, /**< The device has been lost and can't be recovered. */

    /* Reserved events for private platforms */
    SDL_EVENT_PRIVATE0 = 0x4000,
    SDL_EVENT_PRIVATE1,
    SDL_EVENT_PRIVATE2,
    SDL_EVENT_PRIVATE3,

    /* Internal events */
    SDL_EVENT_POLL_SENTINEL = 0x7F00, /**< Signals the end of an event poll cycle */

    /** Events SDL_EVENT_USER through SDL_EVENT_LAST are for your use,
     *  and should be allocated with SDL_RegisterEvents()
     */
    SDL_EVENT_USER    = 0x8000,

    /**
     *  This last event is only for bounding internal arrays
     */
    SDL_EVENT_LAST    = 0xFFFF,

    /* This just makes sure the enum is the size of Uint32 */
    SDL_EVENT_ENUM_PADDING = 0x7FFFFFFF

} SDL_EventType;

/**
 * Fields shared by every event
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_CommonEvent
{
    Uint32 type;        /**< Event type, shared with all events, Uint32 to cover user events which are not in the SDL_EventType enumeration */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
} SDL_CommonEvent;

/**
 * Display state change event data (event.display.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_DisplayEvent
{
    SDL_EventType type; /**< SDL_EVENT_DISPLAY_* */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_DisplayID displayID;/**< The associated display */
    Sint32 data1;       /**< event dependent data */
    Sint32 data2;       /**< event dependent data */
} SDL_DisplayEvent;

/**
 * Window state change event data (event.window.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_WindowEvent
{
    SDL_EventType type; /**< SDL_EVENT_WINDOW_* */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID; /**< The associated window */
    Sint32 data1;       /**< event dependent data */
    Sint32 data2;       /**< event dependent data */
} SDL_WindowEvent;

/**
 * Keyboard device event structure (event.kdevice.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_KeyboardDeviceEvent
{
    SDL_EventType type; /**< SDL_EVENT_KEYBOARD_ADDED or SDL_EVENT_KEYBOARD_REMOVED */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_KeyboardID which;   /**< The keyboard instance id */
} SDL_KeyboardDeviceEvent;

/**
 * Keyboard button event structure (event.key.*)
 *
 * The `key` is the base SDL_Keycode generated by pressing the `scancode`
 * using the current keyboard layout, applying any options specified in
 * SDL_HINT_KEYCODE_OPTIONS. You can get the SDL_Keycode corresponding to the
 * event scancode and modifiers directly from the keyboard layout, bypassing
 * SDL_HINT_KEYCODE_OPTIONS, by calling SDL_GetKeyFromScancode().
 *
 * \since This struct is available since SDL 3.2.0.
 *
 * \sa SDL_GetKeyFromScancode
 * \sa SDL_HINT_KEYCODE_OPTIONS
 */
typedef struct SDL_KeyboardEvent
{
    SDL_EventType type;     /**< SDL_EVENT_KEY_DOWN or SDL_EVENT_KEY_UP */
    Uint32 reserved;
    Uint64 timestamp;       /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID;  /**< The window with keyboard focus, if any */
    SDL_KeyboardID which;   /**< The keyboard instance id, or 0 if unknown or virtual */
    SDL_Scancode scancode;  /**< SDL physical key code */
    SDL_Keycode key;        /**< SDL virtual key code */
    SDL_Keymod mod;         /**< current key modifiers */
    Uint16 raw;             /**< The platform dependent scancode for this event */
    bool down;              /**< true if the key is pressed */
    bool repeat;            /**< true if this is a key repeat */
} SDL_KeyboardEvent;

/**
 * Keyboard text editing event structure (event.edit.*)
 *
 * The start cursor is the position, in UTF-8 characters, where new typing
 * will be inserted into the editing text. The length is the number of UTF-8
 * characters that will be replaced by new typing.
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_TextEditingEvent
{
    SDL_EventType type;         /**< SDL_EVENT_TEXT_EDITING */
    Uint32 reserved;
    Uint64 timestamp;           /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID;      /**< The window with keyboard focus, if any */
    const char *text;           /**< The editing text */
    Sint32 start;               /**< The start cursor of selected editing text, or -1 if not set */
    Sint32 length;              /**< The length of selected editing text, or -1 if not set */
} SDL_TextEditingEvent;

/**
 * Keyboard IME candidates event structure (event.edit_candidates.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_TextEditingCandidatesEvent
{
    SDL_EventType type;         /**< SDL_EVENT_TEXT_EDITING_CANDIDATES */
    Uint32 reserved;
    Uint64 timestamp;           /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID;      /**< The window with keyboard focus, if any */
    const char * const *candidates;    /**< The list of candidates, or NULL if there are no candidates available */
    Sint32 num_candidates;      /**< The number of strings in `candidates` */
    Sint32 selected_candidate;  /**< The index of the selected candidate, or -1 if no candidate is selected */
    bool horizontal;          /**< true if the list is horizontal, false if it's vertical */
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
} SDL_TextEditingCandidatesEvent;

/**
 * Keyboard text input event structure (event.text.*)
 *
 * This event will never be delivered unless text input is enabled by calling
 * SDL_StartTextInput(). Text input is disabled by default!
 *
 * \since This struct is available since SDL 3.2.0.
 *
 * \sa SDL_StartTextInput
 * \sa SDL_StopTextInput
 */
typedef struct SDL_TextInputEvent
{
    SDL_EventType type; /**< SDL_EVENT_TEXT_INPUT */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID; /**< The window with keyboard focus, if any */
    const char *text;   /**< The input text, UTF-8 encoded */
} SDL_TextInputEvent;

/**
 * Mouse device event structure (event.mdevice.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_MouseDeviceEvent
{
    SDL_EventType type; /**< SDL_EVENT_MOUSE_ADDED or SDL_EVENT_MOUSE_REMOVED */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_MouseID which;  /**< The mouse instance id */
} SDL_MouseDeviceEvent;

/**
 * Mouse motion event structure (event.motion.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_MouseMotionEvent
{
    SDL_EventType type; /**< SDL_EVENT_MOUSE_MOTION */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID; /**< The window with mouse focus, if any */
    SDL_MouseID which;  /**< The mouse instance id in relative mode, SDL_TOUCH_MOUSEID for touch events, SDL_PEN_MOUSEID for pen events, or 0 */
    SDL_MouseButtonFlags state;       /**< The current button state */
    float x;            /**< X coordinate, relative to window */
    float y;            /**< Y coordinate, relative to window */
    float xrel;         /**< The relative motion in the X direction */
    float yrel;         /**< The relative motion in the Y direction */
} SDL_MouseMotionEvent;

/**
 * Mouse button event structure (event.button.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_MouseButtonEvent
{
    SDL_EventType type; /**< SDL_EVENT_MOUSE_BUTTON_DOWN or SDL_EVENT_MOUSE_BUTTON_UP */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID; /**< The window with mouse focus, if any */
    SDL_MouseID which;  /**< The mouse instance id in relative mode, SDL_TOUCH_MOUSEID for touch events, or 0 */
    Uint8 button;       /**< The mouse button index */
    bool down;          /**< true if the button is pressed */
    Uint8 clicks;       /**< 1 for single-click, 2 for double-click, etc. */
    Uint8 padding;
    float x;            /**< X coordinate, relative to window */
    float y;            /**< Y coordinate, relative to window */
} SDL_MouseButtonEvent;

/**
 * Mouse wheel event structure (event.wheel.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_MouseWheelEvent
{
    SDL_EventType type; /**< SDL_EVENT_MOUSE_WHEEL */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID; /**< The window with mouse focus, if any */
    SDL_MouseID which;  /**< The mouse instance id in relative mode or 0 */
    float x;            /**< The amount scrolled horizontally, positive to the right and negative to the left */
    float y;            /**< The amount scrolled vertically, positive away from the user and negative toward the user */
    SDL_MouseWheelDirection direction; /**< Set to one of the SDL_MOUSEWHEEL_* defines. When FLIPPED the values in X and Y will be opposite. Multiply by -1 to change them back */
    float mouse_x;      /**< X coordinate, relative to window */
    float mouse_y;      /**< Y coordinate, relative to window */
    Sint32 integer_x;   /**< The amount scrolled horizontally, accumulated to whole scroll "ticks" (added in 3.2.12) */
    Sint32 integer_y;   /**< The amount scrolled vertically, accumulated to whole scroll "ticks" (added in 3.2.12) */
} SDL_MouseWheelEvent;

/**
 * Joystick axis motion event structure (event.jaxis.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_JoyAxisEvent
{
    SDL_EventType type; /**< SDL_EVENT_JOYSTICK_AXIS_MOTION */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    Uint8 axis;         /**< The joystick axis index */
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 value;       /**< The axis value (range: -32768 to 32767) */
    Uint16 padding4;
} SDL_JoyAxisEvent;

/**
 * Joystick trackball motion event structure (event.jball.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_JoyBallEvent
{
    SDL_EventType type; /**< SDL_EVENT_JOYSTICK_BALL_MOTION */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    Uint8 ball;         /**< The joystick trackball index */
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 xrel;        /**< The relative motion in the X direction */
    Sint16 yrel;        /**< The relative motion in the Y direction */
} SDL_JoyBallEvent;

/**
 * Joystick hat position change event structure (event.jhat.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_JoyHatEvent
{
    SDL_EventType type; /**< SDL_EVENT_JOYSTICK_HAT_MOTION */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    Uint8 hat;          /**< The joystick hat index */
    Uint8 value;        /**< The hat position value.
                         *   \sa SDL_HAT_LEFTUP SDL_HAT_UP SDL_HAT_RIGHTUP
                         *   \sa SDL_HAT_LEFT SDL_HAT_CENTERED SDL_HAT_RIGHT
                         *   \sa SDL_HAT_LEFTDOWN SDL_HAT_DOWN SDL_HAT_RIGHTDOWN
                         *
                         *   Note that zero means the POV is centered.
                         */
    Uint8 padding1;
    Uint8 padding2;
} SDL_JoyHatEvent;

/**
 * Joystick button event structure (event.jbutton.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_JoyButtonEvent
{
    SDL_EventType type; /**< SDL_EVENT_JOYSTICK_BUTTON_DOWN or SDL_EVENT_JOYSTICK_BUTTON_UP */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    Uint8 button;       /**< The joystick button index */
    bool down;      /**< true if the button is pressed */
    Uint8 padding1;
    Uint8 padding2;
} SDL_JoyButtonEvent;

/**
 * Joystick device event structure (event.jdevice.*)
 *
 * SDL will send JOYSTICK_ADDED events for devices that are already plugged in
 * during SDL_Init.
 *
 * \since This struct is available since SDL 3.2.0.
 *
 * \sa SDL_GamepadDeviceEvent
 */
typedef struct SDL_JoyDeviceEvent
{
    SDL_EventType type; /**< SDL_EVENT_JOYSTICK_ADDED or SDL_EVENT_JOYSTICK_REMOVED or SDL_EVENT_JOYSTICK_UPDATE_COMPLETE */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which;       /**< The joystick instance id */
} SDL_JoyDeviceEvent;

/**
 * Joystick battery level change event structure (event.jbattery.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_JoyBatteryEvent
{
    SDL_EventType type; /**< SDL_EVENT_JOYSTICK_BATTERY_UPDATED */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    SDL_PowerState state; /**< The joystick battery state */
    int percent;          /**< The joystick battery percent charge remaining */
} SDL_JoyBatteryEvent;

/**
 * Gamepad axis motion event structure (event.gaxis.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_GamepadAxisEvent
{
    SDL_EventType type; /**< SDL_EVENT_GAMEPAD_AXIS_MOTION */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    Uint8 axis;         /**< The gamepad axis (SDL_GamepadAxis) */
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 value;       /**< The axis value (range: -32768 to 32767) */
    Uint16 padding4;
} SDL_GamepadAxisEvent;


/**
 * Gamepad button event structure (event.gbutton.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_GamepadButtonEvent
{
    SDL_EventType type; /**< SDL_EVENT_GAMEPAD_BUTTON_DOWN or SDL_EVENT_GAMEPAD_BUTTON_UP */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    Uint8 button;       /**< The gamepad button (SDL_GamepadButton) */
    bool down;      /**< true if the button is pressed */
    Uint8 padding1;
    Uint8 padding2;
} SDL_GamepadButtonEvent;


/**
 * Gamepad device event structure (event.gdevice.*)
 *
 * Joysticks that are supported gamepads receive both an SDL_JoyDeviceEvent
 * and an SDL_GamepadDeviceEvent.
 *
 * SDL will send GAMEPAD_ADDED events for joysticks that are already plugged
 * in during SDL_Init() and are recognized as gamepads. It will also send
 * events for joysticks that get gamepad mappings at runtime.
 *
 * \since This struct is available since SDL 3.2.0.
 *
 * \sa SDL_JoyDeviceEvent
 */
typedef struct SDL_GamepadDeviceEvent
{
    SDL_EventType type; /**< SDL_EVENT_GAMEPAD_ADDED, SDL_EVENT_GAMEPAD_REMOVED, or SDL_EVENT_GAMEPAD_REMAPPED, SDL_EVENT_GAMEPAD_UPDATE_COMPLETE or SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which;       /**< The joystick instance id */
} SDL_GamepadDeviceEvent;

/**
 * Gamepad touchpad event structure (event.gtouchpad.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_GamepadTouchpadEvent
{
    SDL_EventType type; /**< SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN or SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION or SDL_EVENT_GAMEPAD_TOUCHPAD_UP */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    Sint32 touchpad;    /**< The index of the touchpad */
    Sint32 finger;      /**< The index of the finger on the touchpad */
    float x;            /**< Normalized in the range 0...1 with 0 being on the left */
    float y;            /**< Normalized in the range 0...1 with 0 being at the top */
    float pressure;     /**< Normalized in the range 0...1 */
} SDL_GamepadTouchpadEvent;

/**
 * Gamepad sensor event structure (event.gsensor.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_GamepadSensorEvent
{
    SDL_EventType type; /**< SDL_EVENT_GAMEPAD_SENSOR_UPDATE */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    Sint32 sensor;      /**< The type of the sensor, one of the values of SDL_SensorType */
    float data[3];      /**< Up to 3 values from the sensor, as defined in SDL_sensor.h */
    Uint64 sensor_timestamp; /**< The timestamp of the sensor reading in nanoseconds, not necessarily synchronized with the system clock */
} SDL_GamepadSensorEvent;

/**
 * Gamepad capsense event structure (event.gcapsense.*)
 *
 * \since This struct is available since SDL 3.6.0.
 */
typedef struct SDL_GamepadCapSenseEvent
{
    SDL_EventType type;     /**< SDL_EVENT_GAMEPAD_CAPSENSE_TOUCH or SDL_EVENT_GAMEPAD_CAPSENSE_RELEASE */
    Uint32 reserved;
    Uint64 timestamp;       /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which;   /**< The joystick instance id */
    Uint8 capsense;         /**< The capsense type (SDL_GamepadCapSenseType) */
    bool down;              /**< true if the capsense is touched */
    Uint8 padding1;
    Uint8 padding2;
} SDL_GamepadCapSenseEvent;

/**
 * Audio device event structure (event.adevice.*)
 *
 * Note that SDL will send a SDL_EVENT_AUDIO_DEVICE_ADDED event for every
 * device it discovers during initialization. After that, this event will only
 * arrive when a device is hotplugged during the program's run.
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_AudioDeviceEvent
{
    SDL_EventType type; /**< SDL_EVENT_AUDIO_DEVICE_ADDED, or SDL_EVENT_AUDIO_DEVICE_REMOVED, or SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_AudioDeviceID which;       /**< SDL_AudioDeviceID for the device being added or removed or changing */
    bool recording; /**< false if a playback device, true if a recording device. */
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
} SDL_AudioDeviceEvent;

/**
 * Camera device event structure (event.cdevice.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_CameraDeviceEvent
{
    SDL_EventType type; /**< SDL_EVENT_CAMERA_DEVICE_ADDED, SDL_EVENT_CAMERA_DEVICE_REMOVED, SDL_EVENT_CAMERA_DEVICE_APPROVED, SDL_EVENT_CAMERA_DEVICE_DENIED */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_CameraID which;       /**< SDL_CameraID for the device being added or removed or changing */
} SDL_CameraDeviceEvent;


/**
 * Renderer event structure (event.render.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_RenderEvent
{
    SDL_EventType type; /**< SDL_EVENT_RENDER_TARGETS_RESET, SDL_EVENT_RENDER_DEVICE_RESET, SDL_EVENT_RENDER_DEVICE_LOST */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID; /**< The window containing the renderer in question. */
} SDL_RenderEvent;


/**
 * Touch finger event structure (event.tfinger.*)
 *
 * Coordinates in this event are normalized. `x` and `y` are normalized to a
 * range between 0.0f and 1.0f, relative to the window, so (0,0) is the top
 * left and (1,1) is the bottom right. Delta coordinates `dx` and `dy` are
 * normalized in the ranges of -1.0f (traversed all the way from the bottom or
 * right to all the way up or left) to 1.0f (traversed all the way from the
 * top or left to all the way down or right).
 *
 * Note that while the coordinates are _normalized_, they are not _clamped_,
 * which means in some circumstances you can get a value outside of this
 * range. For example, a renderer using logical presentation might give a
 * negative value when the touch is in the letterboxing. Some platforms might
 * report a touch outside of the window, which will also be outside of the
 * range.
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_TouchFingerEvent
{
    SDL_EventType type; /**< SDL_EVENT_FINGER_DOWN, SDL_EVENT_FINGER_UP, SDL_EVENT_FINGER_MOTION, or SDL_EVENT_FINGER_CANCELED */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_TouchID touchID; /**< The touch device id */
    SDL_FingerID fingerID;
    float x;            /**< Normalized in the range 0...1 */
    float y;            /**< Normalized in the range 0...1 */
    float dx;           /**< Normalized in the range -1...1 */
    float dy;           /**< Normalized in the range -1...1 */
    float pressure;     /**< Normalized in the range 0...1 */
    SDL_WindowID windowID; /**< The window underneath the finger, if any */
} SDL_TouchFingerEvent;

/**
 * Pinch event structure (event.pinch.*)
 */
typedef struct SDL_PinchFingerEvent
{
    SDL_EventType type; /**< ::SDL_EVENT_PINCH_BEGIN or ::SDL_EVENT_PINCH_UPDATE or ::SDL_EVENT_PINCH_END */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    float scale;        /**< The scale change since the last SDL_EVENT_PINCH_UPDATE. Scale < 1 is "zoom out". Scale > 1 is "zoom in". */
    SDL_WindowID windowID; /**< The window underneath the finger, if any */
} SDL_PinchFingerEvent;

/**
 * Pressure-sensitive pen proximity event structure (event.pproximity.*)
 *
 * When a pen becomes visible to the system (it is close enough to a tablet,
 * etc), SDL will send an SDL_EVENT_PEN_PROXIMITY_IN event with the new pen's
 * ID. This ID is valid until the pen leaves proximity again (has been removed
 * from the tablet's area, the tablet has been unplugged, etc). If the same
 * pen reenters proximity again, it will be given a new ID.
 *
 * Note that "proximity" means "close enough for the tablet to know the tool
 * is there." The pen touching and lifting off from the tablet while not
 * leaving the area are handled by SDL_EVENT_PEN_DOWN and SDL_EVENT_PEN_UP.
 *
 * Not all platforms have a window associated with the pen during proximity
 * events. Some wait until motion/button/etc events to offer this info.
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_PenProximityEvent
{
    SDL_EventType type; /**< SDL_EVENT_PEN_PROXIMITY_IN or SDL_EVENT_PEN_PROXIMITY_OUT */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID; /**< The window with pen focus, if any */
    SDL_PenID which;        /**< The pen instance id */
} SDL_PenProximityEvent;

/**
 * Pressure-sensitive pen motion event structure (event.pmotion.*)
 *
 * Depending on the hardware, you may get motion events when the pen is not
 * touching a tablet, for tracking a pen even when it isn't drawing. You
 * should listen for SDL_EVENT_PEN_DOWN and SDL_EVENT_PEN_UP events, or check
 * `pen_state & SDL_PEN_INPUT_DOWN` to decide if a pen is "drawing" when
 * dealing with pen motion.
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_PenMotionEvent
{
    SDL_EventType type; /**< SDL_EVENT_PEN_MOTION */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID; /**< The window with pen focus, if any */
    SDL_PenID which;        /**< The pen instance id */
    SDL_PenInputFlags pen_state;   /**< Complete pen input state at time of event */
    float x;                /**< X coordinate, relative to window */
    float y;                /**< Y coordinate, relative to window */
} SDL_PenMotionEvent;

/**
 * Pressure-sensitive pen touched event structure (event.ptouch.*)
 *
 * These events come when a pen touches a surface (a tablet, etc), or lifts
 * off from one.
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_PenTouchEvent
{
    SDL_EventType type;     /**< SDL_EVENT_PEN_DOWN or SDL_EVENT_PEN_UP */
    Uint32 reserved;
    Uint64 timestamp;       /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID;  /**< The window with pen focus, if any */
    SDL_PenID which;        /**< The pen instance id */
    SDL_PenInputFlags pen_state;   /**< Complete pen input state at time of event */
    float x;                /**< X coordinate, relative to window */
    float y;                /**< Y coordinate, relative to window */
    bool eraser;        /**< true if eraser end is used (not all pens support this). */
    bool down;          /**< true if the pen is touching or false if the pen is lifted off */
} SDL_PenTouchEvent;

/**
 * Pressure-sensitive pen button event structure (event.pbutton.*)
 *
 * This is for buttons on the pen itself that the user might click. The pen
 * itself pressing down to draw triggers a SDL_EVENT_PEN_DOWN event instead.
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_PenButtonEvent
{
    SDL_EventType type; /**< SDL_EVENT_PEN_BUTTON_DOWN or SDL_EVENT_PEN_BUTTON_UP */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID; /**< The window with mouse focus, if any */
    SDL_PenID which;        /**< The pen instance id */
    SDL_PenInputFlags pen_state;   /**< Complete pen input state at time of event */
    float x;                /**< X coordinate, relative to window */
    float y;                /**< Y coordinate, relative to window */
    Uint8 button;       /**< The pen button index (first button is 1). */
    bool down;      /**< true if the button is pressed */
} SDL_PenButtonEvent;

/**
 * Pressure-sensitive pen pressure / angle event structure (event.paxis.*)
 *
 * You might get some of these events even if the pen isn't touching the
 * tablet.
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_PenAxisEvent
{
    SDL_EventType type;     /**< SDL_EVENT_PEN_AXIS */
    Uint32 reserved;
    Uint64 timestamp;       /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID;  /**< The window with pen focus, if any */
    SDL_PenID which;        /**< The pen instance id */
    SDL_PenInputFlags pen_state;   /**< Complete pen input state at time of event */
    float x;                /**< X coordinate, relative to window */
    float y;                /**< Y coordinate, relative to window */
    SDL_PenAxis axis;       /**< Axis that has changed */
    float value;            /**< New value of axis */
} SDL_PenAxisEvent;

/**
 * An event used to drop text or request a file open by the system
 * (event.drop.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_DropEvent
{
    SDL_EventType type; /**< SDL_EVENT_DROP_BEGIN or SDL_EVENT_DROP_FILE or SDL_EVENT_DROP_TEXT or SDL_EVENT_DROP_COMPLETE or SDL_EVENT_DROP_POSITION */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID;    /**< The window that was dropped on, if any */
    float x;            /**< X coordinate, relative to window (not on begin) */
    float y;            /**< Y coordinate, relative to window (not on begin) */
    const char *source; /**< The source app that sent this drop event, or NULL if that isn't available */
    const char *data;   /**< The text for SDL_EVENT_DROP_TEXT and the file name for SDL_EVENT_DROP_FILE, NULL for other events */
} SDL_DropEvent;

/**
 * An event triggered when the clipboard contents have changed
 * (event.clipboard.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_ClipboardEvent
{
    SDL_EventType type; /**< SDL_EVENT_CLIPBOARD_UPDATE */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    bool owner;         /**< are we owning the clipboard (internal update) */
    Sint32 num_mime_types;   /**< number of mime types */
    const char **mime_types; /**< current mime types */
} SDL_ClipboardEvent;

/**
 * Sensor event structure (event.sensor.*)
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_SensorEvent
{
    SDL_EventType type; /**< SDL_EVENT_SENSOR_UPDATE */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_SensorID which; /**< The instance ID of the sensor */
    float data[6];      /**< Up to 6 values from the sensor - additional values can be queried using SDL_GetSensorData() */
    Uint64 sensor_timestamp; /**< The timestamp of the sensor reading in nanoseconds, not necessarily synchronized with the system clock */
} SDL_SensorEvent;

/**
 * The "quit requested" event
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_QuitEvent
{
    SDL_EventType type; /**< SDL_EVENT_QUIT */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
} SDL_QuitEvent;

/**
 * A user-defined event type (event.user.*)
 *
 * This event is unique; it is never created by SDL, but only by the
 * application. The event can be pushed onto the event queue using
 * SDL_PushEvent(). The contents of the structure members are completely up to
 * the programmer; the only requirement is that '''type''' is a value obtained
 * from SDL_RegisterEvents().
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef struct SDL_UserEvent
{
    Uint32 type;        /**< SDL_EVENT_USER through SDL_EVENT_LAST, Uint32 because these are not in the SDL_EventType enumeration */
    Uint32 reserved;
    Uint64 timestamp;   /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_WindowID windowID; /**< The associated window if any */
    Sint32 code;        /**< User defined event code */
    void *data1;        /**< User defined data pointer */
    void *data2;        /**< User defined data pointer */
} SDL_UserEvent;


/**
 * The structure for all events in SDL.
 *
 * The SDL_Event structure is the core of all event handling in SDL. SDL_Event
 * is a union of all event structures used in SDL.
 *
 * \since This struct is available since SDL 3.2.0.
 */
typedef union SDL_Event
{
    Uint32 type;                            /**< Event type, shared with all events, Uint32 to cover user events which are not in the SDL_EventType enumeration */
    SDL_CommonEvent common;                 /**< Common event data */
    SDL_DisplayEvent display;               /**< Display event data */
    SDL_WindowEvent window;                 /**< Window event data */
    SDL_KeyboardDeviceEvent kdevice;        /**< Keyboard device change event data */
    SDL_KeyboardEvent key;                  /**< Keyboard event data */
    SDL_TextEditingEvent edit;              /**< Text editing event data */
    SDL_TextEditingCandidatesEvent edit_candidates; /**< Text editing candidates event data */
    SDL_TextInputEvent text;                /**< Text input event data */
    SDL_MouseDeviceEvent mdevice;           /**< Mouse device change event data */
    SDL_MouseMotionEvent motion;            /**< Mouse motion event data */
    SDL_MouseButtonEvent button;            /**< Mouse button event data */
    SDL_MouseWheelEvent wheel;              /**< Mouse wheel event data */
    SDL_JoyDeviceEvent jdevice;             /**< Joystick device change event data */
    SDL_JoyAxisEvent jaxis;                 /**< Joystick axis event data */
    SDL_JoyBallEvent jball;                 /**< Joystick ball event data */
    SDL_JoyHatEvent jhat;                   /**< Joystick hat event data */
    SDL_JoyButtonEvent jbutton;             /**< Joystick button event data */
    SDL_JoyBatteryEvent jbattery;           /**< Joystick battery event data */
    SDL_GamepadDeviceEvent gdevice;         /**< Gamepad device event data */
    SDL_GamepadAxisEvent gaxis;             /**< Gamepad axis event data */
    SDL_GamepadButtonEvent gbutton;         /**< Gamepad button event data */
    SDL_GamepadTouchpadEvent gtouchpad;     /**< Gamepad touchpad event data */
    SDL_GamepadSensorEvent gsensor;         /**< Gamepad sensor event data */
    SDL_GamepadCapSenseEvent gcapsense;     /**< Gamepad capsense event data */
    SDL_AudioDeviceEvent adevice;           /**< Audio device event data */
    SDL_CameraDeviceEvent cdevice;          /**< Camera device event data */
    SDL_SensorEvent sensor;                 /**< Sensor event data */
    SDL_QuitEvent quit;                     /**< Quit request event data */
    SDL_UserEvent user;                     /**< Custom event data */
    SDL_TouchFingerEvent tfinger;           /**< Touch finger event data */
    SDL_PinchFingerEvent pinch;             /**< Pinch event data */
    SDL_PenProximityEvent pproximity;       /**< Pen proximity event data */
    SDL_PenTouchEvent ptouch;               /**< Pen tip touching event data */
    SDL_PenMotionEvent pmotion;             /**< Pen motion event data */
    SDL_PenButtonEvent pbutton;             /**< Pen button event data */
    SDL_PenAxisEvent paxis;                 /**< Pen axis event data */
    SDL_RenderEvent render;                 /**< Render event data */
    SDL_DropEvent drop;                     /**< Drag and drop event data */
    SDL_ClipboardEvent clipboard;           /**< Clipboard event data */

    /* This is necessary for ABI compatibility between Visual C++ and GCC.
       Visual C++ will respect the push pack pragma and use 52 bytes (size of
       SDL_TextEditingEvent, the largest structure for 32-bit and 64-bit
       architectures) for this union, and GCC will use the alignment of the
       largest datatype within the union, which is 8 bytes on 64-bit
       architectures.

       So... we'll add padding to force the size to be the same for both.

       On architectures where pointers are 16 bytes, this needs rounding up to
       the next multiple of 16, 64, and on architectures where pointers are
       even larger the size of SDL_UserEvent will dominate as being 3 pointers.
    */
    Uint8 padding[128];
} SDL_Event;

typedef enum SDL_EventAction
{
    SDL_ADDEVENT,  /**< Add events to the back of the queue. */
    SDL_PEEKEVENT, /**< Check but don't remove events from the queue front. */
    SDL_GETEVENT   /**< Retrieve/remove events from the front of the queue. */
} SDL_EventAction;

typedef enum SDL_BlendMode
{
    SDL_BLENDMODE_NONE = 0x00000000,
    SDL_BLENDMODE_BLEND = 0x00000001,
    SDL_BLENDMODE_ADD = 0x00000002,
    SDL_BLENDMODE_MOD = 0x00000004,
    SDL_BLENDMODE_MUL = 0x00000008
} SDL_BlendMode;

typedef enum SDL_BlendFactor
{
    SDL_BLENDFACTOR_ZERO,
    SDL_BLENDFACTOR_ONE,
    SDL_BLENDFACTOR_SRC_COLOR,
    SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
    SDL_BLENDFACTOR_SRC_ALPHA,
    SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
    SDL_BLENDFACTOR_DST_COLOR,
    SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR,
    SDL_BLENDFACTOR_DST_ALPHA,
    SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA,
    SDL_BLENDFACTOR_SRC_ALPHA_SATURATE
} SDL_BlendFactor;

typedef enum SDL_BlendOperation
{
    SDL_BLENDOPERATION_ADD,
    SDL_BLENDOPERATION_SUBTRACT,
    SDL_BLENDOPERATION_REV_SUBTRACT,
    SDL_BLENDOPERATION_MINIMUM,
    SDL_BLENDOPERATION_MAXIMUM
} SDL_BlendOperation;

typedef enum SDL_FlashOperation
{
    SDL_FLASH_BRIEFLY = 1,
    SDL_FLASH_UNTIL_FOCUSED = 2
} SDL_FlashOperation;

typedef enum SDL_TextureAccess
{
    SDL_TEXTUREACCESS_STATIC,
    SDL_TEXTUREACCESS_STREAMING,
    SDL_TEXTUREACCESS_TARGET
} SDL_TextureAccess;

typedef enum SDL_TextureAddressMode
{
    SDL_TEXTURE_ADDRESS_INVALID = -1,
    SDL_TEXTURE_ADDRESS_AUTO,   /**< Wrapping is enabled if texture coordinates are outside [0, 1], this is the default */
    SDL_TEXTURE_ADDRESS_CLAMP,  /**< Texture coordinates are clamped to the [0, 1] range */
    SDL_TEXTURE_ADDRESS_WRAP    /**< The texture is repeated (tiled) */
} SDL_TextureAddressMode;

typedef struct SDL_AudioSpec
{
    SDL_AudioFormat format;     /**< Audio data format */
    int channels;               /**< Number of channels: 1 mono, 2 stereo, etc */
    int freq;                   /**< sample rate: sample frames per second */
} SDL_AudioSpec;

typedef void (*SDL_AudioPostmixCallback)(void *userdata, const float *buffer, int buflen);
typedef void (*SDL_AudioStreamCallback)(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);
typedef void (*SDL_AudioStreamDataCompleteCallback)(SDL_AudioStream *stream, void *userdata);

typedef Uint32 SDL_HapticID;
typedef Uint16 SDL_HapticEffectType;
typedef Uint8 SDL_HapticDirectionType;
typedef int SDL_HapticEffectID;

typedef struct SDL_HapticDirection
{
    SDL_HapticDirectionType type;  /**< The type of encoding. */
    Sint32 dir[3];                 /**< The encoded direction. */
} SDL_HapticDirection;

typedef struct SDL_HapticConstant
{
    /* Header */
    SDL_HapticEffectType type;      /**< SDL_HAPTIC_CONSTANT */
    SDL_HapticDirection direction;  /**< Direction of the effect. */

    /* Replay */
    Uint32 length;          /**< Duration of the effect. */
    Uint16 delay;           /**< Delay before starting the effect. */

    /* Trigger */
    Uint16 button;          /**< Button that triggers the effect. */
    Uint16 interval;        /**< How soon it can be triggered again after button. */

    /* Constant */
    Sint16 level;           /**< Strength of the constant effect. */

    /* Envelope */
    Uint16 attack_length;   /**< Duration of the attack. */
    Uint16 attack_level;    /**< Level at the start of the attack. */
    Uint16 fade_length;     /**< Duration of the fade. */
    Uint16 fade_level;      /**< Level at the end of the fade. */
} SDL_HapticConstant;

typedef struct SDL_HapticPeriodic
{
    /* Header */
    SDL_HapticEffectType type;      /**< SDL_HAPTIC_SINE, SDL_HAPTIC_SQUARE
                                         SDL_HAPTIC_TRIANGLE, SDL_HAPTIC_SAWTOOTHUP or
                                         SDL_HAPTIC_SAWTOOTHDOWN */
    SDL_HapticDirection direction;  /**< Direction of the effect. */

    /* Replay */
    Uint32 length;      /**< Duration of the effect. */
    Uint16 delay;       /**< Delay before starting the effect. */

    /* Trigger */
    Uint16 button;      /**< Button that triggers the effect. */
    Uint16 interval;    /**< How soon it can be triggered again after button. */

    /* Periodic */
    Uint16 period;      /**< Period of the wave. */
    Sint16 magnitude;   /**< Peak value; if negative, equivalent to 180 degrees extra phase shift. */
    Sint16 offset;      /**< Mean value of the wave. */
    Uint16 phase;       /**< Positive phase shift given by hundredth of a degree. */

    /* Envelope */
    Uint16 attack_length;   /**< Duration of the attack. */
    Uint16 attack_level;    /**< Level at the start of the attack. */
    Uint16 fade_length; /**< Duration of the fade. */
    Uint16 fade_level;  /**< Level at the end of the fade. */
} SDL_HapticPeriodic;

typedef struct SDL_HapticCondition
{
    /* Header */
    SDL_HapticEffectType type;      /**< SDL_HAPTIC_SPRING, SDL_HAPTIC_DAMPER,
                                         SDL_HAPTIC_INERTIA or SDL_HAPTIC_FRICTION */
    SDL_HapticDirection direction;  /**< Direction of the effect. */

    /* Replay */
    Uint32 length;          /**< Duration of the effect. */
    Uint16 delay;           /**< Delay before starting the effect. */

    /* Trigger */
    Uint16 button;          /**< Button that triggers the effect. */
    Uint16 interval;        /**< How soon it can be triggered again after button. */

    /* Condition */
    Uint16 right_sat[3];    /**< Level when joystick is to the positive side; max 0xFFFF. */
    Uint16 left_sat[3];     /**< Level when joystick is to the negative side; max 0xFFFF. */
    Sint16 right_coeff[3];  /**< How fast to increase the force towards the positive side. */
    Sint16 left_coeff[3];   /**< How fast to increase the force towards the negative side. */
    Uint16 deadband[3];     /**< Size of the dead zone; max 0xFFFF: whole axis-range when 0-centered. */
    Sint16 center[3];       /**< Position of the dead zone. */
} SDL_HapticCondition;

typedef struct SDL_HapticRamp
{
    /* Header */
    SDL_HapticEffectType type;      /**< SDL_HAPTIC_RAMP */
    SDL_HapticDirection direction;  /**< Direction of the effect. */

    /* Replay */
    Uint32 length;          /**< Duration of the effect. */
    Uint16 delay;           /**< Delay before starting the effect. */

    /* Trigger */
    Uint16 button;          /**< Button that triggers the effect. */
    Uint16 interval;        /**< How soon it can be triggered again after button. */

    /* Ramp */
    Sint16 start;           /**< Beginning strength level. */
    Sint16 end;             /**< Ending strength level. */

    /* Envelope */
    Uint16 attack_length;   /**< Duration of the attack. */
    Uint16 attack_level;    /**< Level at the start of the attack. */
    Uint16 fade_length;     /**< Duration of the fade. */
    Uint16 fade_level;      /**< Level at the end of the fade. */
} SDL_HapticRamp;

typedef struct SDL_HapticLeftRight
{
    /* Header */
    SDL_HapticEffectType type;  /**< SDL_HAPTIC_LEFTRIGHT */

    /* Replay */
    Uint32 length;          /**< Duration of the effect in milliseconds. */

    /* Rumble */
    Uint16 large_magnitude; /**< Control of the large controller motor. */
    Uint16 small_magnitude; /**< Control of the small controller motor. */
} SDL_HapticLeftRight;

typedef struct SDL_HapticCustom
{
    /* Header */
    SDL_HapticEffectType type;      /**< SDL_HAPTIC_CUSTOM */
    SDL_HapticDirection direction;  /**< Direction of the effect. */

    /* Replay */
    Uint32 length;          /**< Duration of the effect. */
    Uint16 delay;           /**< Delay before starting the effect. */

    /* Trigger */
    Uint16 button;          /**< Button that triggers the effect. */
    Uint16 interval;        /**< How soon it can be triggered again after button. */

    /* Custom */
    Uint8 channels;         /**< Axes to use, minimum of one. */
    Uint16 period;          /**< Sample periods. */
    Uint16 samples;         /**< Amount of samples. */
    Uint16 *data;           /**< Should contain channels*samples items. */

    /* Envelope */
    Uint16 attack_length;   /**< Duration of the attack. */
    Uint16 attack_level;    /**< Level at the start of the attack. */
    Uint16 fade_length;     /**< Duration of the fade. */
    Uint16 fade_level;      /**< Level at the end of the fade. */
} SDL_HapticCustom;

typedef union SDL_HapticEffect
{
    /* Common for all force feedback effects */
    SDL_HapticEffectType type;      /**< Effect type. */
    SDL_HapticConstant constant;    /**< Constant effect. */
    SDL_HapticPeriodic periodic;    /**< Periodic effect. */
    SDL_HapticCondition condition;  /**< Condition effect. */
    SDL_HapticRamp ramp;            /**< Ramp effect. */
    SDL_HapticLeftRight leftright;  /**< Left/Right effect. */
    SDL_HapticCustom custom;        /**< Custom effect. */
} SDL_HapticEffect;

typedef enum SDL_FlipMode
{
    SDL_FLIP_NONE,                                                                  /**< Do not flip */
    SDL_FLIP_HORIZONTAL,                                                            /**< flip horizontally */
    SDL_FLIP_VERTICAL,                                                              /**< flip vertically */
    SDL_FLIP_HORIZONTAL_AND_VERTICAL = (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL)    /**< flip horizontally and vertically (not a diagonal flip) */
} SDL_FlipMode;

typedef enum SDL_GamepadType
{
    SDL_GAMEPAD_TYPE_UNKNOWN = 0,
    SDL_GAMEPAD_TYPE_STANDARD,
    SDL_GAMEPAD_TYPE_XBOX360,
    SDL_GAMEPAD_TYPE_XBOXONE,
    SDL_GAMEPAD_TYPE_PS3,
    SDL_GAMEPAD_TYPE_PS4,
    SDL_GAMEPAD_TYPE_PS5,
    SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO,
    SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT,
    SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT,
    SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR,
    SDL_GAMEPAD_TYPE_COUNT
} SDL_GamepadType;

// Hat positions
enum {
    SDL_HAT_CENTERED = 0x00,
    SDL_HAT_UP = 0x01,
    SDL_HAT_RIGHT = 0x02,
    SDL_HAT_DOWN = 0x04,
    SDL_HAT_LEFT = 0x08,
    SDL_HAT_RIGHTUP = SDL_HAT_RIGHT | SDL_HAT_UP,
    SDL_HAT_RIGHTDOWN = SDL_HAT_RIGHT | SDL_HAT_DOWN,
    SDL_HAT_LEFTUP = SDL_HAT_LEFT | SDL_HAT_UP,
    SDL_HAT_LEFTDOWN = SDL_HAT_LEFT | SDL_HAT_DOWN
};

typedef enum SDL_GLAttr
{
    SDL_GL_RED_SIZE,                    /**< the minimum number of bits for the red channel of the color buffer; defaults to 3. */
    SDL_GL_GREEN_SIZE,                  /**< the minimum number of bits for the green channel of the color buffer; defaults to 3. */
    SDL_GL_BLUE_SIZE,                   /**< the minimum number of bits for the blue channel of the color buffer; defaults to 2. */
    SDL_GL_ALPHA_SIZE,                  /**< the minimum number of bits for the alpha channel of the color buffer; defaults to 0. */
    SDL_GL_BUFFER_SIZE,                 /**< the minimum number of bits for frame buffer size; defaults to 0. */
    SDL_GL_DOUBLEBUFFER,                /**< whether the output is single or double buffered; defaults to double buffering on. */
    SDL_GL_DEPTH_SIZE,                  /**< the minimum number of bits in the depth buffer; defaults to 16. */
    SDL_GL_STENCIL_SIZE,                /**< the minimum number of bits in the stencil buffer; defaults to 0. */
    SDL_GL_ACCUM_RED_SIZE,              /**< the minimum number of bits for the red channel of the accumulation buffer; defaults to 0. */
    SDL_GL_ACCUM_GREEN_SIZE,            /**< the minimum number of bits for the green channel of the accumulation buffer; defaults to 0. */
    SDL_GL_ACCUM_BLUE_SIZE,             /**< the minimum number of bits for the blue channel of the accumulation buffer; defaults to 0. */
    SDL_GL_ACCUM_ALPHA_SIZE,            /**< the minimum number of bits for the alpha channel of the accumulation buffer; defaults to 0. */
    SDL_GL_STEREO,                      /**< whether the output is stereo 3D; defaults to off. */
    SDL_GL_MULTISAMPLEBUFFERS,          /**< the number of buffers used for multisample anti-aliasing; defaults to 0. */
    SDL_GL_MULTISAMPLESAMPLES,          /**< the number of samples used around the current pixel used for multisample anti-aliasing. */
    SDL_GL_ACCELERATED_VISUAL,          /**< set to 1 to require hardware acceleration, set to 0 to force software rendering; defaults to allow either. */
    SDL_GL_RETAINED_BACKING,            /**< not used (deprecated). */
    SDL_GL_CONTEXT_MAJOR_VERSION,       /**< OpenGL context major version. */
    SDL_GL_CONTEXT_MINOR_VERSION,       /**< OpenGL context minor version. */
    SDL_GL_CONTEXT_FLAGS,               /**< some combination of 0 or more of elements of the SDL_GLContextFlag enumeration; defaults to 0. */
    SDL_GL_CONTEXT_PROFILE_MASK,        /**< type of GL context (Core, Compatibility, ES). See SDL_GLProfile; default value depends on platform. */
    SDL_GL_SHARE_WITH_CURRENT_CONTEXT,  /**< OpenGL context sharing; defaults to 0. */
    SDL_GL_FRAMEBUFFER_SRGB_CAPABLE,    /**< requests sRGB capable visual; defaults to 0. */
    SDL_GL_CONTEXT_RELEASE_BEHAVIOR,    /**< sets context the release behavior. See SDL_GLContextReleaseFlag; defaults to FLUSH. */
    SDL_GL_CONTEXT_RESET_NOTIFICATION,  /**< set context reset notification. See SDL_GLContextResetNotification; defaults to NO_NOTIFICATION. */
    SDL_GL_CONTEXT_NO_ERROR,
    SDL_GL_FLOATBUFFERS,
    SDL_GL_EGL_PLATFORM
} SDL_GLAttr;

typedef enum SDL_GLProfile
{
    SDL_GL_CONTEXT_PROFILE_CORE =          0x0001,  /**< OpenGL Core Profile context */
    SDL_GL_CONTEXT_PROFILE_COMPATIBILITY = 0x0002,  /**< OpenGL Compatibility Profile context */
    SDL_GL_CONTEXT_PROFILE_ES =            0x0004,  /**< GLX_CONTEXT_ES2_PROFILE_BIT_EXT */
} SDL_GLProfile;

typedef enum SDL_GLContextFlag
{
    SDL_GL_CONTEXT_DEBUG_FLAG =              0x0001,
    SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG = 0x0002,
    SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG =      0x0004,
    SDL_GL_CONTEXT_RESET_ISOLATION_FLAG =    0x0008
} SDL_GLContextFlag;

typedef enum SDL_GLContextReleaseFlag
{
    SDL_GL_CONTEXT_RELEASE_BEHAVIOR_NONE =  0x0000,
    SDL_GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH = 0x0001,
} SDL_GLContextReleaseFlag;

typedef enum SDL_GLContextResetNotification
{
    SDL_GL_CONTEXT_RESET_NO_NOTIFICATION =  0x0000,
    SDL_GL_CONTEXT_RESET_LOSE_CONTEXT = 0x0001,
} SDL_GLContextResetNotification;

typedef enum SDL_GamepadAxis
{
    SDL_GAMEPAD_AXIS_INVALID = -1,
    SDL_GAMEPAD_AXIS_LEFTX,
    SDL_GAMEPAD_AXIS_LEFTY,
    SDL_GAMEPAD_AXIS_RIGHTX,
    SDL_GAMEPAD_AXIS_RIGHTY,
    SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
    SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
    SDL_GAMEPAD_AXIS_COUNT
} SDL_GamepadAxis;

typedef enum SDL_GamepadButton
{
    SDL_GAMEPAD_BUTTON_INVALID = -1,
    SDL_GAMEPAD_BUTTON_SOUTH,           /**< Bottom face button (e.g. Xbox A button) */
    SDL_GAMEPAD_BUTTON_EAST,            /**< Right face button (e.g. Xbox B button) */
    SDL_GAMEPAD_BUTTON_WEST,            /**< Left face button (e.g. Xbox X button) */
    SDL_GAMEPAD_BUTTON_NORTH,           /**< Top face button (e.g. Xbox Y button) */
    SDL_GAMEPAD_BUTTON_BACK,
    SDL_GAMEPAD_BUTTON_GUIDE,
    SDL_GAMEPAD_BUTTON_START,
    SDL_GAMEPAD_BUTTON_LEFT_STICK,
    SDL_GAMEPAD_BUTTON_RIGHT_STICK,
    SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
    SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
    SDL_GAMEPAD_BUTTON_DPAD_UP,
    SDL_GAMEPAD_BUTTON_DPAD_DOWN,
    SDL_GAMEPAD_BUTTON_DPAD_LEFT,
    SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
    SDL_GAMEPAD_BUTTON_MISC1,           /**< Additional button (e.g. Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Steam Controller QAM button, Amazon Luna microphone button, Google Stadia capture button) */
    SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1,   /**< Upper or primary paddle, under your right hand (e.g. Xbox Elite paddle P1, DualSense Edge RB button, Right Joy-Con SR button, Steam Controller R4 button) */
    SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,    /**< Upper or primary paddle, under your left hand (e.g. Xbox Elite paddle P3, DualSense Edge LB button, Left Joy-Con SL button, Steam Controller L4 button) */
    SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2,   /**< Lower or secondary paddle, under your right hand (e.g. Xbox Elite paddle P2, DualSense Edge right Fn button, Right Joy-Con SL button, Steam Controller R5 button) */
    SDL_GAMEPAD_BUTTON_LEFT_PADDLE2,    /**< Lower or secondary paddle, under your left hand (e.g. Xbox Elite paddle P4, DualSense Edge left Fn button, Left Joy-Con SR button, Steam Controller L5 button) */
    SDL_GAMEPAD_BUTTON_TOUCHPAD,        /**< PS4/PS5 touchpad button */
    SDL_GAMEPAD_BUTTON_MISC2,           /**< Additional button */
    SDL_GAMEPAD_BUTTON_MISC3,           /**< Additional button (e.g. Nintendo GameCube left trigger click) */
    SDL_GAMEPAD_BUTTON_MISC4,           /**< Additional button (e.g. Nintendo GameCube right trigger click) */
    SDL_GAMEPAD_BUTTON_MISC5,           /**< Additional button */
    SDL_GAMEPAD_BUTTON_MISC6,           /**< Additional button */
    SDL_GAMEPAD_BUTTON_COUNT
} SDL_GamepadButton;

typedef enum SDL_GamepadCapSenseType
{
    SDL_GAMEPAD_CAPSENSE_INVALID = -1,
    SDL_GAMEPAD_CAPSENSE_LEFT_STICK,    /**< Activated by touching the top of the left thumbstick */
    SDL_GAMEPAD_CAPSENSE_RIGHT_STICK,   /**< Activated by touching the top of the right thumbstick */
    SDL_GAMEPAD_CAPSENSE_LEFT_GRIP,     /**< Activated by gripping the left handle of the controller */
    SDL_GAMEPAD_CAPSENSE_RIGHT_GRIP,    /**< Activated by gripping the right handle of the controller */
    SDL_GAMEPAD_CAPSENSE_COUNT
} SDL_GamepadCapSenseType;

typedef enum SDL_GamepadBindingType
{
    SDL_GAMEPAD_BINDTYPE_NONE = 0,
    SDL_GAMEPAD_BINDTYPE_BUTTON,
    SDL_GAMEPAD_BINDTYPE_AXIS,
    SDL_GAMEPAD_BINDTYPE_HAT
} SDL_GamepadBindingType;

typedef struct SDL_GamepadBinding
{
    SDL_GamepadBindingType input_type;
    union
    {
        int button;

        struct
        {
            int axis;
            int axis_min;
            int axis_max;
        } axis;

        struct
        {
            int hat;
            int hat_mask;
        } hat;

    } input;

    SDL_GamepadBindingType output_type;
    union
    {
        SDL_GamepadButton button;

        struct
        {
            SDL_GamepadAxis axis;
            int axis_min;
            int axis_max;
        } axis;

    } output;
} SDL_GamepadBinding;

typedef enum SDL_GamepadButtonLabel
{
    SDL_GAMEPAD_BUTTON_LABEL_UNKNOWN,
    SDL_GAMEPAD_BUTTON_LABEL_A,
    SDL_GAMEPAD_BUTTON_LABEL_B,
    SDL_GAMEPAD_BUTTON_LABEL_X,
    SDL_GAMEPAD_BUTTON_LABEL_Y,
    SDL_GAMEPAD_BUTTON_LABEL_CROSS,
    SDL_GAMEPAD_BUTTON_LABEL_CIRCLE,
    SDL_GAMEPAD_BUTTON_LABEL_SQUARE,
    SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE
} SDL_GamepadButtonLabel;

typedef enum SDL_SensorType
{
    SDL_SENSOR_INVALID = -1,
    SDL_SENSOR_UNKNOWN,
    SDL_SENSOR_ACCEL,
    SDL_SENSOR_GYRO,
    SDL_SENSOR_ACCEL_L,
    SDL_SENSOR_GYRO_L,
    SDL_SENSOR_ACCEL_R,
    SDL_SENSOR_GYRO_R
} SDL_SensorType;

typedef enum SDL_SystemCursor
{
    SDL_SYSTEM_CURSOR_DEFAULT,      /**< Default cursor. Usually an arrow. */
    SDL_SYSTEM_CURSOR_TEXT,         /**< Text selection. Usually an I-beam. */
    SDL_SYSTEM_CURSOR_WAIT,         /**< Wait. Usually an hourglass or watch or spinning ball. */
    SDL_SYSTEM_CURSOR_CROSSHAIR,    /**< Crosshair. */
    SDL_SYSTEM_CURSOR_PROGRESS,     /**< Program is busy but still interactive. Usually it's WAIT with an arrow. */
    SDL_SYSTEM_CURSOR_NWSE_RESIZE,  /**< Double arrow pointing northwest and southeast. */
    SDL_SYSTEM_CURSOR_NESW_RESIZE,  /**< Double arrow pointing northeast and southwest. */
    SDL_SYSTEM_CURSOR_EW_RESIZE,    /**< Double arrow pointing west and east. */
    SDL_SYSTEM_CURSOR_NS_RESIZE,    /**< Double arrow pointing north and south. */
    SDL_SYSTEM_CURSOR_MOVE,         /**< Four pointed arrow pointing north, south, east, and west. */
    SDL_SYSTEM_CURSOR_NOT_ALLOWED,  /**< Not permitted. Usually a slashed circle or crossbones. */
    SDL_SYSTEM_CURSOR_POINTER,      /**< Pointer that indicates a link. Usually a pointing hand. */
    SDL_SYSTEM_CURSOR_NW_RESIZE,    /**< Window resize top-left. This may be a single arrow or a double arrow like NWSE_RESIZE. */
    SDL_SYSTEM_CURSOR_N_RESIZE,     /**< Window resize top. May be NS_RESIZE. */
    SDL_SYSTEM_CURSOR_NE_RESIZE,    /**< Window resize top-right. May be NESW_RESIZE. */
    SDL_SYSTEM_CURSOR_E_RESIZE,     /**< Window resize right. May be EW_RESIZE. */
    SDL_SYSTEM_CURSOR_SE_RESIZE,    /**< Window resize bottom-right. May be NWSE_RESIZE. */
    SDL_SYSTEM_CURSOR_S_RESIZE,     /**< Window resize bottom. May be NS_RESIZE. */
    SDL_SYSTEM_CURSOR_SW_RESIZE,    /**< Window resize bottom-left. May be NESW_RESIZE. */
    SDL_SYSTEM_CURSOR_W_RESIZE,     /**< Window resize left. May be EW_RESIZE. */
    SDL_SYSTEM_CURSOR_COUNT
} SDL_SystemCursor;

typedef enum SDL_ScaleMode
{
    SDL_SCALEMODE_INVALID = -1,
    SDL_SCALEMODE_NEAREST,  /**< nearest pixel sampling */
    SDL_SCALEMODE_LINEAR,   /**< linear filtering */
    SDL_SCALEMODE_PIXELART  /**< nearest pixel sampling with improved scaling for pixel art, available since SDL 3.4.0 */
} SDL_ScaleMode;

typedef enum SDL_JoystickType
{
    SDL_JOYSTICK_TYPE_UNKNOWN,
    SDL_JOYSTICK_TYPE_GAMEPAD,
    SDL_JOYSTICK_TYPE_WHEEL,
    SDL_JOYSTICK_TYPE_ARCADE_STICK,
    SDL_JOYSTICK_TYPE_FLIGHT_STICK,
    SDL_JOYSTICK_TYPE_DANCE_PAD,
    SDL_JOYSTICK_TYPE_GUITAR,
    SDL_JOYSTICK_TYPE_DRUM_KIT,
    SDL_JOYSTICK_TYPE_ARCADE_PAD,
    SDL_JOYSTICK_TYPE_THROTTLE,
    SDL_JOYSTICK_TYPE_COUNT
} SDL_JoystickType;

typedef enum SDL_JoystickConnectionState
{
    SDL_JOYSTICK_CONNECTION_INVALID = -1,
    SDL_JOYSTICK_CONNECTION_UNKNOWN,
    SDL_JOYSTICK_CONNECTION_WIRED,
    SDL_JOYSTICK_CONNECTION_WIRELESS
} SDL_JoystickConnectionState;

typedef enum SDL_HintPriority
{
    SDL_HINT_DEFAULT = 0,
    SDL_HINT_NORMAL,
    SDL_HINT_OVERRIDE
} SDL_HintPriority;

typedef enum SDL_LogPriority
{
    SDL_LOG_PRIORITY_VERBOSE = 1,
    SDL_LOG_PRIORITY_DEBUG,
    SDL_LOG_PRIORITY_INFO,
    SDL_LOG_PRIORITY_WARN,
    SDL_LOG_PRIORITY_ERROR,
    SDL_LOG_PRIORITY_CRITICAL
} SDL_LogPriority;

typedef enum SDL_DisplayOrientation
{
    SDL_ORIENTATION_UNKNOWN,            /**< The display orientation can't be determined */
    SDL_ORIENTATION_LANDSCAPE,          /**< The display is in landscape mode, with the right side up, relative to portrait mode */
    SDL_ORIENTATION_LANDSCAPE_FLIPPED,  /**< The display is in landscape mode, with the left side up, relative to portrait mode */
    SDL_ORIENTATION_PORTRAIT,           /**< The display is in portrait mode */
    SDL_ORIENTATION_PORTRAIT_FLIPPED    /**< The display is in portrait mode, upside down */
} SDL_DisplayOrientation;

typedef enum SDL_TouchDeviceType
{
    SDL_TOUCH_DEVICE_INVALID = 0,
    SDL_TOUCH_DEVICE_DIRECT,
    SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE,
    SDL_TOUCH_DEVICE_INDIRECT_RELATIVE
} SDL_TouchDeviceType;

typedef struct SDL_Vertex
{
    SDL_FPoint position;
    SDL_Color color;
    SDL_FPoint tex_coord;
} SDL_Vertex;

typedef struct SDL_DisplayModeData SDL_DisplayModeData;

typedef struct SDL_DisplayMode
{
    SDL_DisplayID displayID;        /**< the display this mode is associated with */
    SDL_PixelFormat format;         /**< pixel format */
    int w;                          /**< width */
    int h;                          /**< height */
    float pixel_density;            /**< scale converting size to pixels (e.g. a 1920x1080 mode with 2.0 scale would have 3840x2160 pixels) */
    float refresh_rate;             /**< refresh rate (or 0.0f for unspecified) */
    int refresh_rate_numerator;     /**< precise refresh rate numerator (or 0 for unspecified) */
    int refresh_rate_denominator;   /**< precise refresh rate denominator */

    SDL_DisplayModeData *internal;  /**< Private */

} SDL_DisplayMode;

// Type aliases
typedef int SDL_errorcode;
typedef void (*SDL_HintCallback)(void *userdata, const char *name, const char *oldValue, const char *newValue);
typedef void (*SDL_LogOutputFunction)(void *userdata, int category, SDL_LogPriority priority, const char *message);
typedef int (*SDL_ThreadFunction)(void *data);
typedef int (*SDL_EventFilter)(void *userdata, SDL_Event *event);
typedef int (*SDL_CompareCallback)(const void *a, const void *b);
typedef int SDL_SpinLock;
typedef int SDL_ThreadPriority;
typedef int SDL_HitTest;
typedef void *SDL_iconv_t;
typedef int (*SDL_main_func)(int argc, char *argv[]);
typedef void (*SDL_WindowsMessageHook)(void *userdata, void *window, void *msg, void *wParam, void *lParam);
typedef int SDL_YUV_CONVERSION_MODE;
typedef void *SDL_MetalView;

#ifndef SDL_DYNAPI_PROC
#define SDL_DYNAPI_PROC(rc, fn, params, args, ret) extern rc __attribute__((weak)) fn params;
#endif

#ifndef SDL_PRINTF_FORMAT_STRING
#define SDL_PRINTF_FORMAT_STRING
#endif

#ifndef SDL_SCANF_FORMAT_STRING
#define SDL_SCANF_FORMAT_STRING
#endif

#ifndef SDL_OUT_Z_CAP
#define SDL_OUT_Z_CAP(x)
#endif

#ifndef SDL_OUT_BYTECAP
#define SDL_OUT_BYTECAP(x)
#endif

#ifndef SDL_IN_BYTECAP
#define SDL_IN_BYTECAP(x)
#endif

#ifndef SDL_INOUT_Z_CAP
#define SDL_INOUT_Z_CAP(x)
#endif

#ifndef SDL_IN_Z_CAP
#define SDL_IN_Z_CAP(x)
#endif

#ifndef SDLCALL
#define SDLCALL
#endif

#include "SDL3_dynapi_procs.h"

#ifdef SDL_DYNAPI_PROC
#undef SDL_DYNAPI_PROC
#endif

} // namespace SDL3

}

#endif
