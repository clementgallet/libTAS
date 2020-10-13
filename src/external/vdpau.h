/*
 * This source file is documented using Doxygen markup.
 * See http://www.stack.nl/~dimitri/doxygen/
 */

/*
 * This copyright notice applies to this header file:
 *
 * Copyright (c) 2008-2015 NVIDIA Corporation
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file vdpau.h
 * \brief The Core API
 *
 * This file contains the \ref api_core "Core API".
 */

#ifndef _VDPAU_H
#define _VDPAU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup api_core Core API
 *
 * The core API encompasses all VDPAU functionality that operates
 * in the same fashion across all Window Systems.
 *
 * @{
 */

/**
 * \defgroup base_types Basic Types
 *
 * VDPAU primarily uses ISO C99 types from \c stdint.h.
 *
 * @{
 */

/** \brief A true \ref VdpBool value */
#define VDP_TRUE 1
/** \brief A false \ref VdpBool value */
#define VDP_FALSE 0
/**
 * \brief A boolean value, holding \ref VDP_TRUE or \ref
 * VDP_FALSE.
 */
typedef int VdpBool;

/** @} */

/**
 * \defgroup misc_types Miscellaneous Types
 *
 * @{
 */

/**
 * \brief An invalid object handle value.
 *
 * This value may be used to represent an invalid, or
 * non-existent, object (\ref VdpDevice "VdpDevice",
 * \ref VdpVideoSurface "VdpVideoSurface", etc.)
 *
 * Note that most APIs require valid object handles in all
 * cases, and will fail when presented with this value.
 */
#define VDP_INVALID_HANDLE 0xffffffffU

/**
 * \brief The set of all chroma formats for \ref VdpVideoSurface
 * "VdpVideoSurface"s.
 */
typedef uint32_t VdpChromaType;

/** \hideinitializer \brief 4:2:0 chroma format. Undefined field/frame based
 *  Video surfaces allocated with this chroma type have undefined
 *  field/frame structure. The implementation is free to internally morph
 *  the surface between frame/field(NV12/NV24) as required by
 *  VdpVideoDecoder operation. Interop with OpenGL allows registration
 *  of these surfaces for either field- or frame-based interop. But, an implicit
 *  field/frame structure conversion may be performed.
 */
#define VDP_CHROMA_TYPE_420 ((VdpChromaType)0)
/** \hideinitializer \brief 4:2:2 chroma format. Undefined field/frame based
 *  Video surfaces allocated with this chroma type have undefined
 *  field/frame structure. The implementation is free to internally morph
 *  the surface between frame/field(NV12/NV24) as required by
 *  VdpVideoDecoder operation. Interop with OpenGL allows registration
 *  of these surfaces for either field- or frame-based interop. But, an implicit
 *  field/frame structure conversion may be performed.
 */
#define VDP_CHROMA_TYPE_422 ((VdpChromaType)1)
/** \hideinitializer \brief 4:4:4 chroma format. Undefined field/frame based
 *  Video surfaces allocated with this chroma type have undefined
 *  field/frame structure. The implementation is free to internally morph
 *  the surface between frame/field(NV12/NV24) as required by
 *  VdpVideoDecoder operation. Interop with OpenGL allows registration
 *  of these surfaces for either field- or frame-based interop. But, an implicit
 *  field/frame structure conversion may be performed.
 */
#define VDP_CHROMA_TYPE_444 ((VdpChromaType)2)

/** \hideinitializer \brief 4:2:0 chroma format. Field based.
 *  Video surfaces allocated with this chroma type can only be
 *  interoped with OpenGL if the matching field/frame structure is
 *  specified in the OpenGL API */
#define VDP_CHROMA_TYPE_420_FIELD ((VdpChromaType)3)
/** \hideinitializer \brief 4:2:2 chroma format. Field based.
 *  Video surfaces allocated with this chroma type can only be
 *  interoped with OpenGL if the matching field/frame structure is
 *  specified in the OpenGL API */
#define VDP_CHROMA_TYPE_422_FIELD ((VdpChromaType)4)
/** \hideinitializer \brief 4:4:4 chroma format. Field based.
 *  Video surfaces allocated with this chroma type can only be
 *  interoped with OpenGL if the matching field/frame structure is
 *  specified in the OpenGL API */
#define VDP_CHROMA_TYPE_444_FIELD ((VdpChromaType)5)

/** \hideinitializer \brief 4:2:0 chroma format. Frame based.
 *  Video surfaces allocated with this chroma type can only be
 *  interoped with OpenGL if the matching field/frame structure is
 *  specified in the OpenGL API */
#define VDP_CHROMA_TYPE_420_FRAME ((VdpChromaType)6)
/** \hideinitializer \brief 4:2:2 chroma format. Frame based.
 *  Video surfaces allocated with this chroma type can only be
 *  interoped with OpenGL if the matching field/frame structure is
 *  specified in the OpenGL API */
#define VDP_CHROMA_TYPE_422_FRAME ((VdpChromaType)7)
/** \hideinitializer \brief 4:4:4 chroma format. Frame based.
 *  Video surfaces allocated with this chroma type can only be
 *  interoped with OpenGL if the matching field/frame structure is
 *  specified in the OpenGL API */
#define VDP_CHROMA_TYPE_444_FRAME ((VdpChromaType)8)
/** \hideinitializer \brief 4:2:0 chroma format. Undefined field/frame based
 *  Video surfaces allocated with this chroma type have undefined
 *  field/frame structure. The implementation is free to internally morph
 *  the surface between frame/field as required by VdpVideoDecoder operation.
 *  Interop with OpenGL allows registration of these surfaces for either
 *  field- or frame-based interop. But, an implicit field/frame structure
 *  conversion may be performed.
 */
#define VDP_CHROMA_TYPE_420_16 ((VdpChromaType)9)
/** \hideinitializer \brief 4:2:2 chroma format. Undefined field/frame based
 *  Video surfaces allocated with this chroma type have undefined
 *  field/frame structure. The implementation is free to internally morph
 *  the surface between frame/field as required by VdpVideoDecoder operation.
 *  Interop with OpenGL allows registration of these surfaces for either
 *  field- or frame-based interop. But, an implicit field/frame structure
 *  conversion may be performed.
 */
#define VDP_CHROMA_TYPE_422_16 ((VdpChromaType)10)
/** \hideinitializer \brief 4:4:4 chroma format. Undefined field/frame based
 *  Video surfaces allocated with this chroma type have undefined
 *  field/frame structure. The implementation is free to internally morph
 *  the surface between frame/field as required by VdpVideoDecoder operation.
 *  Interop with OpenGL allows registration of these surfaces for either
 *  field- or frame-based interop. But, an implicit field/frame structure
 *  conversion may be performed.
 */
#define VDP_CHROMA_TYPE_444_16 ((VdpChromaType)11)

/** \hideinitializer \brief 4:2:0 chroma format. Field based.
 *  Video surfaces allocated with this chroma type can only be
 *  interoped with OpenGL if the matching field/frame structure is
 *  specified in the OpenGL API */
#define VDP_CHROMA_TYPE_420_FIELD_16 ((VdpChromaType)12)
/** \hideinitializer \brief 4:2:2 chroma format. Field based.
 *  Video surfaces allocated with this chroma type can only be
 *  interoped with OpenGL if the matching field/frame structure is
 *  specified in the OpenGL API */
#define VDP_CHROMA_TYPE_422_FIELD_16 ((VdpChromaType)13)
/** \hideinitializer \brief 4:4:4 chroma format. Field based.
 *  Video surfaces allocated with this chroma type can only be
 *  interoped with OpenGL if the matching field/frame structure is
 *  specified in the OpenGL API */
#define VDP_CHROMA_TYPE_444_FIELD_16 ((VdpChromaType)14)

/** \hideinitializer \brief 4:2:0 chroma format. Frame based.
 *  Video surfaces allocated with this chroma type can only be
 *  interoped with OpenGL if the matching field/frame structure is
 *  specified in the OpenGL API */
#define VDP_CHROMA_TYPE_420_FRAME_16 ((VdpChromaType)15)
/** \hideinitializer \brief 4:2:2 chroma format. Frame based.
 *  Video surfaces allocated with this chroma type can only be
 *  interoped with OpenGL if the matching field/frame structure is
 *  specified in the OpenGL API */
#define VDP_CHROMA_TYPE_422_FRAME_16 ((VdpChromaType)16)
/** \hideinitializer \brief 4:4:4 chroma format. Frame based.
 *  Video surfaces allocated with this chroma type can only be
 *  interoped with OpenGL if the matching field/frame structure is
 *  specified in the OpenGL API */
#define VDP_CHROMA_TYPE_444_FRAME_16 ((VdpChromaType)17)

/**
 * \brief The set of all known YCbCr surface formats.
 */
typedef uint32_t VdpYCbCrFormat;

/**
 * \hideinitializer
 * \brief The "NV12" YCbCr surface format.
 *
 * This format has a two planes, a Y plane and a UV plane.
 *
 * The Y plane is an array of byte-sized Y components.
 * Applications should access this data via a uint8_t pointer.
 *
 * The UV plane is an array of interleaved byte-sized U and V
 * components, in the order U, V, U, V. Applications should
 * access this data via a uint8_t pointer.
 */
#define VDP_YCBCR_FORMAT_NV12     ((VdpYCbCrFormat)0)
/**
 * \hideinitializer
 * \brief The "YV12" YCbCr surface format.
 *
 * This format has a three planes, a Y plane, a V plane, and a U
 * plane.
 *
 * Each of the planes is an array of byte-sized components.
 *
 * Applications should access this data via a uint8_t pointer.
 */
#define VDP_YCBCR_FORMAT_YV12     ((VdpYCbCrFormat)1)
/**
 * \hideinitializer
 * \brief The "UYVY" YCbCr surface format.
 *
 * This format may also be known as Y422, UYNV, HDYC.
 *
 * This format has a single plane.
 *
 * This plane is an array of interleaved byte-sized Y, U, and V
 * components, in the order U, Y, V, Y, U, Y, V, Y.
 *
 * Applications should access this data via a uint8_t pointer.
 */
#define VDP_YCBCR_FORMAT_UYVY     ((VdpYCbCrFormat)2)
/**
 * \hideinitializer
 * \brief The "YUYV" YCbCr surface format.
 *
 * This format may also be known as YUY2, YUNV, V422.
 *
 * This format has a single plane.
 *
 * This plane is an array of interleaved byte-sized Y, U, and V
 * components, in the order Y, U, Y, V, Y, U, Y, V.
 *
 * Applications should access this data via a uint8_t pointer.
 */
#define VDP_YCBCR_FORMAT_YUYV     ((VdpYCbCrFormat)3)
/**
 * \hideinitializer
 * \brief A packed YCbCr format.
 *
 * This format has a single plane.
 *
 * This plane is an array packed 32-bit pixel data. Within each
 * 32-bit pixel, bits [31:24] contain A, bits [23:16] contain V,
 * bits [15:8] contain U, and bits [7:0] contain Y.
 *
 * Applications should access this data via a uint32_t pointer.
 */
#define VDP_YCBCR_FORMAT_Y8U8V8A8 ((VdpYCbCrFormat)4)
/**
 * \hideinitializer
 * \brief A packed YCbCr format.
 *
 * This format has a single plane.
 *
 * This plane is an array packed 32-bit pixel data. Within each
 * 32-bit pixel, bits [31:24] contain A, bits [23:16] contain Y,
 * bits [15:8] contain U, and bits [7:0] contain V.
 *
 * Applications should access this data via a uint32_t pointer.
 */
#define VDP_YCBCR_FORMAT_V8U8Y8A8 ((VdpYCbCrFormat)5)
/**
 * \hideinitializer
 * \brief The "Y_UV_444" YCbCr surface format.
 *
 * This format has two planes, a Y plane and a UV plane.
 *
 * The Y plane is an array of byte-sized Y components.
 * Applications should access this data via a uint8_t pointer.
 *
 * The UV plane is an array of interleaved byte-sized U and V
 * components, in the order U, V, U, V. Applications should
 * access this data via a uint8_t pointer.
 */
#define VDP_YCBCR_FORMAT_Y_UV_444     ((VdpYCbCrFormat)6)
/**
 * \hideinitializer
 * \brief The "Y_U_V_444" YCbCr surface format.
 *
 * This format has three planes, a Y plane, a V plane, and a U
 * plane.
 *
 * Each of the planes is an array of byte-sized components.
 *
 * Applications should access this data via a uint8_t pointer.
 */
#define VDP_YCBCR_FORMAT_Y_U_V_444     ((VdpYCbCrFormat)7)
/**
 * \hideinitializer
 * \brief The P010 surface format.
 *
 * This format has two planes, a Y plane and a UV plane.
 *
 * The Y plane is an array of two byte sized Y components.
 * Applications should access this data via a uint16_t pointer.
 *
 * The UV plane is an array of interleaved two byte sized U and V
 * components, in the order U, V, U, V. Applications should
 * access this data via a uint8_t pointer.
 *
 * Note that the P010 surface format has an identical memory
 * layout as the P016 surface format, with bits 0 through 5
 * set to zero.
 */
#define VDP_YCBCR_FORMAT_P010           ((VdpYCbCrFormat)8)
/**
 * \hideinitializer
 * \brief The P016 surface format.
 *
 * This format has two planes, a Y plane and a UV plane.
 *
 * The Y plane is an array of two byte sized Y components.
 * Applications should access this data via a uint16_t pointer.
 *
 * The UV plane is an array of interleaved two byte sized U and V
 * components, in the order U, V, U, V. Applications should
 * access this data via a uint8_t pointer.
 */
#define VDP_YCBCR_FORMAT_P016           ((VdpYCbCrFormat)9)
 /**
  * \hideinitializer
  * \brief The "Y_U_V_444_16" YCbCr surface format.
  *
  * This format has three planes, a Y plane, a V plane, and a U
  * plane.
  *
  * Each of the planes is an array of two byte-sized components.
  *
  * Applications should access this data via a uint16_t pointer.
  */
 #define VDP_YCBCR_FORMAT_Y_U_V_444_16     ((VdpYCbCrFormat)11)

/**
 * \brief  The set of all known RGB surface formats.
 */
typedef uint32_t VdpRGBAFormat;

/**
 * \hideinitializer
 * \brief A packed RGB format.
 *
 * This format has a single plane.
 *
 * This plane is an array packed 32-bit pixel data. Within each
 * 32-bit pixel, bits [31:24] contain A, bits [23:16] contain R,
 * bits [15:8] contain G, and bits [7:0] contain B.
 *
 * Applications should access this data via a uint32_t pointer.
 */
#define VDP_RGBA_FORMAT_B8G8R8A8    ((VdpRGBAFormat)0)
/**
 * \hideinitializer
 * \brief A packed RGB format.
 *
 * This format has a single plane.
 *
 * This plane is an array packed 32-bit pixel data. Within each
 * 32-bit pixel, bits [31:24] contain A, bits [23:16] contain B,
 * bits [15:8] contain G, and bits [7:0] contain R.
 *
 * Applications should access this data via a uint32_t pointer.
 */
#define VDP_RGBA_FORMAT_R8G8B8A8    ((VdpRGBAFormat)1)
/**
 * \hideinitializer
 * \brief A packed RGB format.
 *
 * This format has a single plane.
 *
 * This plane is an array packed 32-bit pixel data. Within each
 * 32-bit pixel, bits [31:30] contain A, bits [29:20] contain B,
 * bits [19:10] contain G, and bits [9:0] contain R.
 *
 * Applications should access this data via a uint32_t pointer.
 */
#define VDP_RGBA_FORMAT_R10G10B10A2 ((VdpRGBAFormat)2)
/**
 * \hideinitializer
 * \brief A packed RGB format.
 *
 * This format has a single plane.
 *
 * This plane is an array packed 32-bit pixel data. Within each
 * 32-bit pixel, bits [31:30] contain A, bits [29:20] contain R,
 * bits [19:10] contain G, and bits [9:0] contain B.
 *
 * Applications should access this data via a uint32_t pointer.
 */
#define VDP_RGBA_FORMAT_B10G10R10A2 ((VdpRGBAFormat)3)
/**
 * \hideinitializer
 * \brief An alpha-only surface format.
 *
 * This format has a single plane.
 *
 * This plane is an array of byte-sized components.
 *
 * Applications should access this data via a uint8_t pointer.
 */
#define VDP_RGBA_FORMAT_A8          ((VdpRGBAFormat)4)

/**
 * \brief  The set of all known indexed surface formats.
 */
typedef uint32_t VdpIndexedFormat;

/**
 * \hideinitializer
 * \brief A 4-bit indexed format, with alpha.
 *
 * This format has a single plane.
 *
 * This plane is an array of byte-sized components. Within each
 * byte, bits [7:4] contain I (index), and bits [3:0] contain A.
 *
 * Applications should access this data via a uint8_t pointer.
 */
#define VDP_INDEXED_FORMAT_A4I4 ((VdpIndexedFormat)0)
/**
 * \hideinitializer
 * \brief A 4-bit indexed format, with alpha.
 *
 * This format has a single plane.
 *
 * This plane is an array of byte-sized components. Within each
 * byte, bits [7:4] contain A, and bits [3:0] contain I (index).
 *
 * Applications should access this data via a uint8_t pointer.
 */
#define VDP_INDEXED_FORMAT_I4A4 ((VdpIndexedFormat)1)
/**
 * \hideinitializer
 * \brief A 8-bit indexed format, with alpha.
 *
 * This format has a single plane.
 *
 * This plane is an array of interleaved byte-sized A and I
 * (index) components, in the order A, I, A, I.
 *
 * Applications should access this data via a uint8_t pointer.
 */
#define VDP_INDEXED_FORMAT_A8I8 ((VdpIndexedFormat)2)
/**
 * \hideinitializer
 * \brief A 8-bit indexed format, with alpha.
 *
 * This format has a single plane.
 *
 * This plane is an array of interleaved byte-sized A and I
 * (index) components, in the order I, A, I, A.
 *
 * Applications should access this data via a uint8_t pointer.
 */
#define VDP_INDEXED_FORMAT_I8A8 ((VdpIndexedFormat)3)

/**
 * \brief A location within a surface.
 *
 * The VDPAU co-ordinate system has its origin at the top-left
 * of a surface, with x and y components increasing right and
 * down.
 */
typedef struct {
    /** X co-ordinate. */
    uint32_t x;
    /** Y co-ordinate. */
    uint32_t y;
} VdpPoint;

/**
 * \brief A rectangular region of a surface.
 *
 * The co-ordinates are top-left inclusive, bottom-right
 * exclusive.
 *
 * The VDPAU co-ordinate system has its origin at the top-left
 * of a surface, with x and y components increasing right and
 * down.
 */
typedef struct {
    /** Left X co-ordinate. Inclusive. */
    uint32_t x0;
    /** Top Y co-ordinate. Inclusive. */
    uint32_t y0;
    /** Right X co-ordinate. Exclusive. */
    uint32_t x1;
    /** Bottom Y co-ordinate. Exclusive. */
    uint32_t y1;
} VdpRect;

/**
 * A constant RGBA color.
 *
 * Note that the components are stored as float values in the
 * range 0.0...1.0 rather than format-specific integer values.
 * This allows VdpColor values to be independent from the exact
 * surface format(s) in use.
 */
typedef struct {
    float red;
    float green;
    float blue;
    float alpha;
} VdpColor;

/** @} */

/**
 * \defgroup error_handling Error Handling
 *
 * @{
 */

/**
 * \hideinitializer
 * \brief The set of all possible error codes.
 */
typedef enum {
    /** The operation completed successfully; no error. */
    VDP_STATUS_OK = 0,
    /**
     * No backend implementation could be loaded.
     */
    VDP_STATUS_NO_IMPLEMENTATION,
    /**
     * The display was preempted, or a fatal error occurred.
     *
     * The application must re-initialize VDPAU.
     */
    VDP_STATUS_DISPLAY_PREEMPTED,
    /**
     * An invalid handle value was provided.
     *
     * Either the handle does not exist at all, or refers to an object of an
     * incorrect type.
     */
    VDP_STATUS_INVALID_HANDLE,
    /**
     * An invalid pointer was provided.
     *
     * Typically, this means that a NULL pointer was provided for an "output"
     * parameter.
     */
    VDP_STATUS_INVALID_POINTER,
    /**
     * An invalid/unsupported \ref VdpChromaType value was supplied.
     */
    VDP_STATUS_INVALID_CHROMA_TYPE,
    /**
     * An invalid/unsupported \ref VdpYCbCrFormat value was supplied.
     */
    VDP_STATUS_INVALID_Y_CB_CR_FORMAT,
    /**
     * An invalid/unsupported \ref VdpRGBAFormat value was supplied.
     */
    VDP_STATUS_INVALID_RGBA_FORMAT,
    /**
     * An invalid/unsupported \ref VdpIndexedFormat value was supplied.
     */
    VDP_STATUS_INVALID_INDEXED_FORMAT,
    /**
     * An invalid/unsupported \ref VdpColorStandard value was supplied.
     */
    VDP_STATUS_INVALID_COLOR_STANDARD,
    /**
     * An invalid/unsupported \ref VdpColorTableFormat value was supplied.
     */
    VDP_STATUS_INVALID_COLOR_TABLE_FORMAT,
    /**
     * An invalid/unsupported \ref VdpOutputSurfaceRenderBlendFactor value was
     * supplied.
     */
    VDP_STATUS_INVALID_BLEND_FACTOR,
    /**
     * An invalid/unsupported \ref VdpOutputSurfaceRenderBlendEquation value
     * was supplied.
     */
    VDP_STATUS_INVALID_BLEND_EQUATION,
    /**
     * An invalid/unsupported flag value/combination was supplied.
     */
    VDP_STATUS_INVALID_FLAG,
    /**
     * An invalid/unsupported \ref VdpDecoderProfile value was supplied.
     */
    VDP_STATUS_INVALID_DECODER_PROFILE,
    /**
     * An invalid/unsupported \ref VdpVideoMixerFeature value was supplied.
     */
    VDP_STATUS_INVALID_VIDEO_MIXER_FEATURE,
    /**
     * An invalid/unsupported \ref VdpVideoMixerParameter value was supplied.
     */
    VDP_STATUS_INVALID_VIDEO_MIXER_PARAMETER,
    /**
     * An invalid/unsupported \ref VdpVideoMixerAttribute value was supplied.
     */
    VDP_STATUS_INVALID_VIDEO_MIXER_ATTRIBUTE,
    /**
     * An invalid/unsupported \ref VdpVideoMixerPictureStructure value was
     * supplied.
     */
    VDP_STATUS_INVALID_VIDEO_MIXER_PICTURE_STRUCTURE,
    /**
     * An invalid/unsupported \ref VdpFuncId value was supplied.
     */
    VDP_STATUS_INVALID_FUNC_ID,
    /**
     * The size of a supplied object does not match the object it is being
     * used with.
     *
     * For example, a \ref VdpVideoMixer "VdpVideoMixer" is configured to
     * process \ref VdpVideoSurface "VdpVideoSurface" objects of a specific
     * size. If presented with a \ref VdpVideoSurface "VdpVideoSurface" of a
     * different size, this error will be raised.
     */
    VDP_STATUS_INVALID_SIZE,
    /**
     * An invalid/unsupported value was supplied.
     *
     * This is a catch-all error code for values of type other than those
     * with a specific error code.
     */
    VDP_STATUS_INVALID_VALUE,
    /**
     * An invalid/unsupported structure version was specified in a versioned
     * structure. This implies that the implementation is older than the
     * header file the application was built against.
     */
    VDP_STATUS_INVALID_STRUCT_VERSION,
    /**
     * The system does not have enough resources to complete the requested
     * operation at this time.
     */
    VDP_STATUS_RESOURCES,
    /**
     * The set of handles supplied are not all related to the same VdpDevice.
     *
     * When performing operations that operate on multiple surfaces, such as
     * \ref  VdpOutputSurfaceRenderOutputSurface or \ref VdpVideoMixerRender,
     * all supplied surfaces must have been created within the context of the
     * same \ref VdpDevice "VdpDevice" object. This error is raised if they were
     * not.
     */
    VDP_STATUS_HANDLE_DEVICE_MISMATCH,
    /**
     * A catch-all error, used when no other error code applies.
     */
    VDP_STATUS_ERROR,
} VdpStatus;


/** @} */

/**
 * \defgroup versioning Versioning
 *
 *
 * @{
 */

/**
 * \brief The VDPAU interface version described by this header file.
 *
 * This version will only increase if a major incompatible change is made.
 * For example, if the parameters passed to an existing function are modified,
 * rather than simply adding new functions/enumerations), or if the mechanism
 * used to load the backend driver is modified incompatibly. Such changes are
 * unlikely.
 *
 * This value also represents the DSO version of VDPAU-related
 * shared-libraries.
 *
 * VDPAU version numbers are simple integers that increase monotonically
 * (typically by value 1).
 */
#define VDPAU_INTERFACE_VERSION 1

/**
 * \brief The VDPAU version described by this header file.
 *
 * This version will increase whenever any non-documentation change is made to
 * vdpau.h, or related header files such as vdpau_x11.h. Such changes
 * typically involve the addition of new functions, constants, or features.
 * Such changes are expected to be completely backwards-compatible.
 *
 * VDPAU version numbers are simple integers that increase monotonically
 * (typically by value 1).
 */
#define VDPAU_VERSION 1


/** @} */

/**
 * \defgroup VdpDevice VdpDevice; Primary API object
 *
 * The VdpDevice is the root of the VDPAU object system. Using a
 * VdpDevice object, all other object types may be created. See
 * the sections describing those other object types for details
 * on object creation.
 *
 * Note that VdpDevice objects are created using the \ref
 * api_winsys.
 *
 * @{
 */

/**
 * \brief  An opaque handle representing a VdpDevice object.
 */
typedef uint32_t VdpDevice;


/** @} */

/**
 * \defgroup VdpCSCMatrix VdpCSCMatrix; CSC Matrix Manipulation
 *
 * When converting from YCbCr to RGB data formats, a color space
 * conversion operation must be performed. This operation is
 * parameterized using a "color space conversion matrix". The
 * VdpCSCMatrix is a data structure representing this
 * information.
 *
 * @{
 */

/**
 * \brief Storage for a color space conversion matrix.
 *
 * Note that the application may choose to construct the matrix
 * content by either:
 * - Directly filling in the fields of the CSC matrix
 * - Using the \ref VdpGenerateCSCMatrix helper function.
 *
 * The color space conversion equation is as follows:
 *
 * \f[
 * \left( \begin{array}{c} R \\ G \\ B \end{array} \right)
 * =
 * \left( \begin{array}{cccc}
 * m_{0,0} & m_{0,1} & m_{0,2} & m_{0,3} \\
 * m_{1,0} & m_{1,1} & m_{1,2} & m_{1,3} \\
 * m_{2,0} & m_{2,1} & m_{2,2} & m_{2,3}
 * \end{array}
 * \right)
 * *
 * \left( \begin{array}{c} Y \\ Cb \\ Cr \\ 1.0 \end{array}
 *      \right)
 * \f]
 */
typedef float VdpCSCMatrix[3][4];

#define VDP_PROCAMP_VERSION 0

/**
 * \brief Procamp operation parameterization data.
 *
 * When performing a color space conversion operation, various
 * adjustments can be performed at the same time, such as
 * brightness and contrast. This structure defines the level of
 * adjustments to make.
 */
typedef struct {
    /**
     * This field must be filled with VDP_PROCAMP_VERSION
     */
    uint32_t struct_version;
    /**
     * Brightness adjustment amount. A value clamped between
     * -1.0 and 1.0. 0.0 represents no modification.
     */
    float brightness;
    /**
     * Contrast adjustment amount. A value clamped between
     * 0.0 and 10.0. 1.0 represents no modification.
     */
    float contrast;
    /**
     * Saturation adjustment amount. A value clamped between 0.0 and
     * 10.0. 1.0 represents no modification.
     */
    float saturation;
    /**
     * Hue adjustment amount. A value clamped between
     * -PI and PI. 0.0 represents no modification.
     */
    float hue;
} VdpProcamp;

/**
 * \brief YCbCr color space specification.
 *
 * A number of YCbCr color spaces exist. This enumeration
 * defines the specifications known to VDPAU.
 */
typedef uint32_t VdpColorStandard;

/** \hideinitializer \brief ITU-R BT.601 */
#define VDP_COLOR_STANDARD_ITUR_BT_601 ((VdpColorStandard)0)
/** \hideinitializer \brief ITU-R BT.709 */
#define VDP_COLOR_STANDARD_ITUR_BT_709 ((VdpColorStandard)1)
/** \hideinitializer \brief SMPTE-240M */
#define VDP_COLOR_STANDARD_SMPTE_240M  ((VdpColorStandard)2)


/** @} */

/**
 * \defgroup VdpVideoSurface VdpVideoSurface; Video Surface object
 *
 * A VdpVideoSurface stores YCbCr data in an internal format,
 * with a variety of possible chroma sub-sampling options.
 *
 * A VdpVideoSurface may be filled with:
 * - Data provided by the CPU via \ref
 *   VdpVideoSurfacePutBitsYCbCr (i.e. software decode.)
 * - The result of applying a \ref VdpDecoder "VdpDecoder" to
 *   compressed video data.
 *
 * VdpVideoSurface content may be accessed by:
 * - The application via \ref VdpVideoSurfaceGetBitsYCbCr
 * - The Hardware that implements \ref VdpOutputSurface
 *   "VdpOutputSurface" \ref VdpOutputSurfaceRender
 *   "rendering functionality".
 * - The Hardware the implements \ref VdpVideoMixer
 *   "VdpVideoMixer" functionality.
 *
 * VdpVideoSurfaces are not directly displayable. They must be
 * converted into a displayable format using \ref VdpVideoMixer
 * "VdpVideoMixer" objects.
 *
 * See \ref video_mixer_usage for additional information.
 *
 * @{
 */


/**
 * \brief An opaque handle representing a VdpVideoSurface
 *        object.
 */
typedef uint32_t VdpVideoSurface;

/**
 * \defgroup VdpOutputSurface VdpOutputSurface; Output Surface object
 *
 * A VdpOutputSurface stores RGBA data in a defined format.
 *
 * A VdpOutputSurface may be filled with:
 * - Data provided by the CPU via the various
 *   VdpOutputSurfacePutBits functions.
 * - Using the VdpOutputSurface \ref VdpOutputSurfaceRender
 *   "rendering functionality".
 * - Using a \ref VdpVideoMixer "VdpVideoMixer" object.
 *
 * VdpOutputSurface content may be accessed by:
 * - The application via the various VdpOutputSurfaceGetBits
 *   functions.
 * - The Hardware that implements VdpOutputSurface
 *   \ref VdpOutputSurfaceRender "rendering functionality".
 * - The Hardware the implements \ref VdpVideoMixer
 *   "VdpVideoMixer" functionality.
 * - The Hardware that implements \ref VdpPresentationQueue
 *   "VdpPresentationQueue" functionality,
 *
 * VdpOutputSurfaces are directly displayable using a \ref
 * VdpPresentationQueue "VdpPresentationQueue" object.
 *
 * @{
 */

/**
 * \brief The set of all known color table formats, for use with
 * \ref VdpOutputSurfacePutBitsIndexed.
 */
typedef uint32_t VdpColorTableFormat;

/**
 * \hideinitializer
 * \brief 8-bit per component packed into 32-bits
 *
 * This format is an array of packed 32-bit RGB color values.
 * Bits [31:24] are unused, bits [23:16] contain R, bits [15:8]
 * contain G, and bits [7:0] contain B. Note: The format is
 * physically an array of uint32_t values, and should be accessed
 * as such by the application in order to avoid endianness
 * issues.
 */
#define VDP_COLOR_TABLE_FORMAT_B8G8R8X8 ((VdpColorTableFormat)0)

/**
 * \brief An opaque handle representing a VdpOutputSurface
 *        object.
 */
typedef uint32_t VdpOutputSurface;

/** @} */

/**
 * \defgroup VdpBitmapSurface VdpBitmapSurface; Bitmap Surface object
 *
 * A VdpBitmapSurface stores RGBA data in a defined format.
 *
 * A VdpBitmapSurface may be filled with:
 * - Data provided by the CPU via the \ref
 *   VdpBitmapSurfacePutBitsNative function.
 *
 * VdpBitmapSurface content may be accessed by:
 * - The Hardware that implements \ref VdpOutputSurface
 *   "VdpOutputSurface" \ref VdpOutputSurfaceRender
 *   "rendering functionality"
 *
 * VdpBitmapSurface objects are intended to store static read-only data, such
 * as font glyphs, and the bitmaps used to compose an applications'
 * user-interface.
 *
 * The primary differences between VdpBitmapSurfaces and
 * \ref VdpOutputSurface "VdpOutputSurface"s are:
 *
 * - You cannot render to a VdpBitmapSurface, just upload native data via
 *   the PutBits API.
 *
 * - The read-only nature of a VdpBitmapSurface gives the implementation more
 *   flexibility in its choice of data storage location for the bitmap data.
 *   For example, some implementations may choose to store some/all
 *   VdpBitmapSurface objects in system memory to relieve GPU memory pressure.
 *
 * - VdpBitmapSurface and VdpOutputSurface may support different subsets of all
 *   known RGBA formats.
 *
 * @{
 */

/**
 * \brief An opaque handle representing a VdpBitmapSurface
 *        object.
 */
typedef uint32_t VdpBitmapSurface;

/** @} */

/**
 * \defgroup VdpOutputSurfaceRender VdpOutputSurface Rendering Functionality
 *
 * \ref VdpOutputSurface "VdpOutputSurface" objects
 * directly provide some rendering/compositing operations. These
 * are described below.
 *
 * @{
 */

/**
 * \hideinitializer
 * \brief The blending equation factors.
 */
typedef enum {
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ZERO                     = 0,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE                      = 1,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_SRC_COLOR                = 2,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_SRC_COLOR      = 3,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_SRC_ALPHA                = 4,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA      = 5,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_DST_ALPHA                = 6,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_DST_ALPHA      = 7,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_DST_COLOR                = 8,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_DST_COLOR      = 9,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_SRC_ALPHA_SATURATE       = 10,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_CONSTANT_COLOR           = 11,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR = 12,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_CONSTANT_ALPHA           = 13,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = 14,
} VdpOutputSurfaceRenderBlendFactor;

/**
 * \hideinitializer
 * \brief The blending equations.
 */
typedef enum {
    VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_SUBTRACT         = 0,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_REVERSE_SUBTRACT = 1,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_ADD              = 2,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_MIN              = 3,
    VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_MAX              = 4,
} VdpOutputSurfaceRenderBlendEquation;

#define VDP_OUTPUT_SURFACE_RENDER_BLEND_STATE_VERSION 0

/**
 * \brief Complete blending operation definition.
 *
 * A "blend state" operation controls the math behind certain rendering
 * operations.
 *
 * The blend math is the familiar OpenGL blend math:
 *     \f[
 *     dst.a = equation(blendFactorDstAlpha*dst.a,
 *     blendFactorSrcAlpha*src.a);
 *     \f]
 *     \f[
 *     dst.rgb = equation(blendFactorDstColor*dst.rgb,
 *     blendFactorSrcColor*src.rgb);
 *     \f]
 *
 * Note that when equation is MIN or MAX, the blend factors and constants
 * are ignored, and are treated as if they were 1.0.
 */
typedef struct {
    /**
     * This field must be filled with VDP_OUTPUT_SURFACE_RENDER_BLEND_STATE_VERSIION
     */
    uint32_t struct_version;
    VdpOutputSurfaceRenderBlendFactor   blend_factor_source_color;
    VdpOutputSurfaceRenderBlendFactor   blend_factor_destination_color;
    VdpOutputSurfaceRenderBlendFactor   blend_factor_source_alpha;
    VdpOutputSurfaceRenderBlendFactor   blend_factor_destination_alpha;
    VdpOutputSurfaceRenderBlendEquation blend_equation_color;
    VdpOutputSurfaceRenderBlendEquation blend_equation_alpha;
    VdpColor                            blend_constant;
} VdpOutputSurfaceRenderBlendState;

/**
 * \hideinitializer
 * \brief Do not rotate source_surface prior to compositing.
 */
#define VDP_OUTPUT_SURFACE_RENDER_ROTATE_0   0

/**
 * \hideinitializer
 * \brief Rotate source_surface 90 degrees clockwise prior to
 *        compositing.
 */
#define VDP_OUTPUT_SURFACE_RENDER_ROTATE_90  1

/**
 * \hideinitializer
 * \brief Rotate source_surface 180 degrees prior to
 *        compositing.
 */
#define VDP_OUTPUT_SURFACE_RENDER_ROTATE_180 2

/**
 * \hideinitializer
 * \brief Rotate source_surface 270 degrees clockwise prior to
 *        compositing.
 */
#define VDP_OUTPUT_SURFACE_RENDER_ROTATE_270 3

/**
 * \hideinitializer
 * \brief A separate color is used for each vertex of the
 *        smooth-shaded quad. Hence, colors array contains 4
 *        elements rather than 1. See description of colors
 *        array.
 */
#define VDP_OUTPUT_SURFACE_RENDER_COLOR_PER_VERTEX (1 << 2)

/** @} */

/**
 * \defgroup VdpDecoder VdpDecoder; Video Decoding object
 *
 * The VdpDecoder object decodes compressed video data, writing
 * the results to a \ref VdpVideoSurface "VdpVideoSurface".
 *
 * A specific VDPAU implementation may support decoding multiple
 * types of compressed video data. However, VdpDecoder objects
 * are able to decode a specific type of compressed video data.
 * This type must be specified during creation.
 *
 * @{
 */

/**
 * \brief The set of all known compressed video formats, and
 *        associated profiles, that may be decoded.
 */
typedef uint32_t VdpDecoderProfile;

/** \hideinitializer */
#define VDP_DECODER_PROFILE_MPEG1                       ((VdpDecoderProfile)0)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_MPEG2_SIMPLE                ((VdpDecoderProfile)1)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_MPEG2_MAIN                  ((VdpDecoderProfile)2)
/** \hideinitializer */
/** \brief MPEG 4 part 10 == H.264 == AVC */
#define VDP_DECODER_PROFILE_H264_BASELINE               ((VdpDecoderProfile)6)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_H264_MAIN                   ((VdpDecoderProfile)7)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_H264_HIGH                   ((VdpDecoderProfile)8)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_VC1_SIMPLE                  ((VdpDecoderProfile)9)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_VC1_MAIN                    ((VdpDecoderProfile)10)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_VC1_ADVANCED                ((VdpDecoderProfile)11)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_MPEG4_PART2_SP              ((VdpDecoderProfile)12)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_MPEG4_PART2_ASP             ((VdpDecoderProfile)13)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_DIVX4_QMOBILE               ((VdpDecoderProfile)14)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_DIVX4_MOBILE                ((VdpDecoderProfile)15)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_DIVX4_HOME_THEATER          ((VdpDecoderProfile)16)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_DIVX4_HD_1080P              ((VdpDecoderProfile)17)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_DIVX5_QMOBILE               ((VdpDecoderProfile)18)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_DIVX5_MOBILE                ((VdpDecoderProfile)19)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_DIVX5_HOME_THEATER          ((VdpDecoderProfile)20)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_DIVX5_HD_1080P              ((VdpDecoderProfile)21)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_H264_CONSTRAINED_BASELINE   ((VdpDecoderProfile)22)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_H264_EXTENDED               ((VdpDecoderProfile)23)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_H264_PROGRESSIVE_HIGH       ((VdpDecoderProfile)24)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_H264_CONSTRAINED_HIGH       ((VdpDecoderProfile)25)
/** \hideinitializer */
/** \brief Support for 8 bit depth only */
#define VDP_DECODER_PROFILE_H264_HIGH_444_PREDICTIVE    ((VdpDecoderProfile)26)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_VP9_PROFILE_0               ((VdpDecoderProfile)27)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_VP9_PROFILE_1               ((VdpDecoderProfile)28)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_VP9_PROFILE_2               ((VdpDecoderProfile)29)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_VP9_PROFILE_3               ((VdpDecoderProfile)30)
/** \hideinitializer */
/** \brief MPEG-H Part 2 == H.265 == HEVC */
#define VDP_DECODER_PROFILE_HEVC_MAIN                   ((VdpDecoderProfile)100)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_HEVC_MAIN_10                ((VdpDecoderProfile)101)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_HEVC_MAIN_STILL             ((VdpDecoderProfile)102)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_HEVC_MAIN_12                ((VdpDecoderProfile)103)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_HEVC_MAIN_444               ((VdpDecoderProfile)104)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_HEVC_MAIN_444_10            ((VdpDecoderProfile)105)
/** \hideinitializer */
#define VDP_DECODER_PROFILE_HEVC_MAIN_444_12            ((VdpDecoderProfile)106)

/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG1_NA 0

/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG2_LL   0
/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG2_ML   1
/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG2_HL14 2
/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG2_HL   3

/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_1     10
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_1b    9
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_1_1   11
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_1_2   12
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_1_3   13
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_2     20
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_2_1   21
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_2_2   22
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_3     30
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_3_1   31
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_3_2   32
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_4     40
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_4_1   41
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_4_2   42
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_5     50
/** \hideinitializer */
#define VDP_DECODER_LEVEL_H264_5_1   51

/** \hideinitializer */
#define VDP_DECODER_LEVEL_VC1_SIMPLE_LOW    0
/** \hideinitializer */
#define VDP_DECODER_LEVEL_VC1_SIMPLE_MEDIUM 1

/** \hideinitializer */
#define VDP_DECODER_LEVEL_VC1_MAIN_LOW    0
/** \hideinitializer */
#define VDP_DECODER_LEVEL_VC1_MAIN_MEDIUM 1
/** \hideinitializer */
#define VDP_DECODER_LEVEL_VC1_MAIN_HIGH   2

/** \hideinitializer */
#define VDP_DECODER_LEVEL_VC1_ADVANCED_L0 0
/** \hideinitializer */
#define VDP_DECODER_LEVEL_VC1_ADVANCED_L1 1
/** \hideinitializer */
#define VDP_DECODER_LEVEL_VC1_ADVANCED_L2 2
/** \hideinitializer */
#define VDP_DECODER_LEVEL_VC1_ADVANCED_L3 3
/** \hideinitializer */
#define VDP_DECODER_LEVEL_VC1_ADVANCED_L4 4

/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG4_PART2_SP_L0 0
/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG4_PART2_SP_L1 1
/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG4_PART2_SP_L2 2
/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG4_PART2_SP_L3 3

/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG4_PART2_ASP_L0 0
/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG4_PART2_ASP_L1 1
/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG4_PART2_ASP_L2 2
/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG4_PART2_ASP_L3 3
/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG4_PART2_ASP_L4 4
/** \hideinitializer */
#define VDP_DECODER_LEVEL_MPEG4_PART2_ASP_L5 5

/** \hideinitializer */
#define VDP_DECODER_LEVEL_DIVX_NA 0

/** \hideinitializer */
#define VDP_DECODER_LEVEL_VP9_L1 1

/**
 * The VDPAU H.265/HEVC decoder levels correspond to the values of
 * general_level_idc as described in the H.265 Specification, Annex A,
 * Table A.1. The enumeration values are equal to thirty times the level
 * number.
 */
#define VDP_DECODER_LEVEL_HEVC_1         30
/** \hideinitializer */
#define VDP_DECODER_LEVEL_HEVC_2         60
/** \hideinitializer */
#define VDP_DECODER_LEVEL_HEVC_2_1       63
/** \hideinitializer */
#define VDP_DECODER_LEVEL_HEVC_3         90
/** \hideinitializer */
#define VDP_DECODER_LEVEL_HEVC_3_1       93
/** \hideinitializer */
#define VDP_DECODER_LEVEL_HEVC_4        120
/** \hideinitializer */
#define VDP_DECODER_LEVEL_HEVC_4_1      123
/** \hideinitializer */
#define VDP_DECODER_LEVEL_HEVC_5        150
/** \hideinitializer */
#define VDP_DECODER_LEVEL_HEVC_5_1      153
/** \hideinitializer */
#define VDP_DECODER_LEVEL_HEVC_5_2      156
/** \hideinitializer */
#define VDP_DECODER_LEVEL_HEVC_6        180
/** \hideinitializer */
#define VDP_DECODER_LEVEL_HEVC_6_1      183
/** \hideinitializer */
#define VDP_DECODER_LEVEL_HEVC_6_2      186

typedef enum {
    VDP_VIDEO_SURFACE_FIELD_STRUCTURE         = (1 << 0),
    VDP_VIDEO_SURFACE_FRAME_STRUCTURE         = (1 << 1)
} VdpVideoSurfaceSupportedPictureStructure;

typedef enum {
    VDP_DECODER_PROFILE_MAX_LEVEL                      = 0,
    VDP_DECODER_PROFILE_MAX_MACROBLOCKS                = 1,
    VDP_DECODER_PROFILE_MAX_WIDTH                      = 2,
    VDP_DECODER_PROFILE_MAX_HEIGHT                     = 3,
    VDP_DECODER_PROFILE_SUPPORTED_PICTURE_STRUCTURE    = 4,
    /**
     * A list of supported chroma types, stored as a bitmask of 1 shifted
     * by each supported VdpChromaType value.  E.g.,
     *   (1 << VDP_CHROMA_TYPE_420) |
     *   (1 << VDP_CHROMA_TYPE_422) |
     *   (1 << VDP_CHROMA_TYPE_444)
     */
    VDP_DECODER_PROFILE_SUPPORTED_CHROMA_TYPES         = 5
} VdpDecoderCapability;

/**
 * \brief An opaque handle representing a VdpDecoder object.
 */
typedef uint32_t VdpDecoder;

#define VDP_BITSTREAM_BUFFER_VERSION 0

/**
 * \brief Application data buffer containing compressed video
 *        data.
 */
typedef struct {
    /**
     * This field must be filled with VDP_BITSTREAM_BUFFER_VERSION
     */
    uint32_t     struct_version;
    /** A pointer to the bitstream data bytes */
    void const * bitstream;
    /** The number of data bytes */
    uint32_t     bitstream_bytes;
} VdpBitstreamBuffer;

/**
 * \brief A generic "picture information" type.
 *
 * This type serves solely to document the expected usage of a
 * generic (void *) function parameter. In actual usage, the
 * application is expected to physically provide a pointer to an
 * instance of one of the "real" VdpPictureInfo* structures,
 * picking the type appropriate for the decoder object in
 * question.
 */
typedef void VdpPictureInfo;

/**
 * \brief Picture parameter information for an MPEG 1 or MPEG 2
 *        picture.
 *
 * Note: References to bitstream fields below may refer to data literally parsed
 * from the bitstream, or derived from the bitstream using a mechanism described
 * in the specification.
 */
typedef struct {
    /**
     * Reference used by B and P frames.
     * Set to VDP_INVALID_HANDLE when not used.
     */
    VdpVideoSurface forward_reference;
    /**
     * Reference used by B frames.
     * Set to VDP_INVALID_HANDLE when not used.
     */
    VdpVideoSurface backward_reference;
    /** Number of slices in the bitstream provided. */
    uint32_t        slice_count;

    /** \name MPEG bitstream
     *
     * Copies of the MPEG bitstream fields.
     * @{ */
    uint8_t picture_structure;
    uint8_t picture_coding_type;
    uint8_t intra_dc_precision;
    uint8_t frame_pred_frame_dct;
    uint8_t concealment_motion_vectors;
    uint8_t intra_vlc_format;
    uint8_t alternate_scan;
    uint8_t q_scale_type;
    uint8_t top_field_first;
    /** MPEG-1 only. For MPEG-2, set to 0. */
    uint8_t full_pel_forward_vector;
    /** MPEG-1 only. For MPEG-2, set to 0. */
    uint8_t full_pel_backward_vector;
    /** For MPEG-1, fill both horizontal and vertical entries. */
    uint8_t f_code[2][2];
    /** Convert to raster order. */
    uint8_t intra_quantizer_matrix[64];
    /** Convert to raster order. */
    uint8_t non_intra_quantizer_matrix[64];
    /** @} */
} VdpPictureInfoMPEG1Or2;

/**
 * \brief Information about an H.264 reference frame
 *
 * Note: References to bitstream fields below may refer to data literally parsed
 * from the bitstream, or derived from the bitstream using a mechanism described
 * in the specification.
 */
typedef struct {
    /**
     * The surface that contains the reference image.
     * Set to VDP_INVALID_HANDLE for unused entries.
     */
    VdpVideoSurface surface;
    /** Is this a long term reference (else short term). */
    VdpBool         is_long_term;
    /**
     * Is the top field used as a reference.
     * Set to VDP_FALSE for unused entries.
     */
    VdpBool         top_is_reference;
    /**
     * Is the bottom field used as a reference.
     * Set to VDP_FALSE for unused entries.
     */
    VdpBool         bottom_is_reference;
    /** [0]: top, [1]: bottom */
    int32_t         field_order_cnt[2];
    /**
     * Copy of the H.264 bitstream field:
     * frame_num from slice_header for short-term references,
     * LongTermPicNum from decoding algorithm for long-term references.
     */
    uint16_t        frame_idx;
} VdpReferenceFrameH264;

/**
 * \brief Picture parameter information for an H.264 picture.
 *
 * Note: The \ref referenceFrames array must contain the "DPB" as
 * defined by the H.264 specification. In particular, once a
 * reference frame has been decoded to a surface, that surface must
 * continue to appear in the DPB until no longer required to predict
 * any future frame. Once a surface is removed from the DPB, it can
 * no longer be used as a reference, unless decoded again.
 *
 * Also note that only surfaces previously generated using \ref
 * VdpDecoderRender may be used as reference frames. In particular,
 * surfaces filled using any "put bits" API will not work.
 *
 * Note: References to bitstream fields below may refer to data literally parsed
 * from the bitstream, or derived from the bitstream using a mechanism described
 * in the specification.
 *
 * Note: VDPAU clients must use VdpPictureInfoH264Predictive to describe the
 * attributes of a frame being decoded with
 * VDP_DECODER_PROFILE_H264_HIGH_444_PREDICTIVE.
 */
typedef struct {
    /** Number of slices in the bitstream provided. */
    uint32_t slice_count;
    /** [0]: top, [1]: bottom */
    int32_t  field_order_cnt[2];
    /** Will the decoded frame be used as a reference later. */
    VdpBool  is_reference;

    /** \name H.264 bitstream
     *
     * Copies of the H.264 bitstream fields.
     * @{ */
    uint16_t frame_num;
    uint8_t  field_pic_flag;
    uint8_t  bottom_field_flag;
    uint8_t  num_ref_frames;
    uint8_t  mb_adaptive_frame_field_flag;
    uint8_t  constrained_intra_pred_flag;
    uint8_t  weighted_pred_flag;
    uint8_t  weighted_bipred_idc;
    uint8_t  frame_mbs_only_flag;
    uint8_t  transform_8x8_mode_flag;
    int8_t   chroma_qp_index_offset;
    int8_t   second_chroma_qp_index_offset;
    int8_t   pic_init_qp_minus26;
    uint8_t  num_ref_idx_l0_active_minus1;
    uint8_t  num_ref_idx_l1_active_minus1;
    uint8_t  log2_max_frame_num_minus4;
    uint8_t  pic_order_cnt_type;
    uint8_t  log2_max_pic_order_cnt_lsb_minus4;
    uint8_t  delta_pic_order_always_zero_flag;
    uint8_t  direct_8x8_inference_flag;
    uint8_t  entropy_coding_mode_flag;
    uint8_t  pic_order_present_flag;
    uint8_t  deblocking_filter_control_present_flag;
    uint8_t  redundant_pic_cnt_present_flag;
    /** Convert to raster order. */
    uint8_t scaling_lists_4x4[6][16];
    /** Convert to raster order. */
    uint8_t scaling_lists_8x8[2][64];
    /** @} */

    /** See \ref VdpPictureInfoH264 for instructions regarding this field. */
    VdpReferenceFrameH264 referenceFrames[16];
} VdpPictureInfoH264;

/**
 * \brief Picture parameter information for an H.264 Hi444PP picture.
 *
 * Note: VDPAU clients must use VdpPictureInfoH264Predictive to describe the
 * attributes of a frame being decoded with
 * VDP_DECODER_PROFILE_H264_HIGH_444_PREDICTIVE.
 *
 * Note: software drivers may choose to honor values of
 * qpprime_y_zero_transform_bypass_flag greater than 1 for internal use.
 */
typedef struct {
    /** \ref VdpPictureInfoH264 struct. */
    VdpPictureInfoH264 pictureInfo;

    /** \name H.264 bitstream
     *
     * Copies of the H.264 bitstream fields.
     * @{ */
    /**
     *  0 - lossless disabled
     *  1 - lossless enabled
     */
    uint8_t qpprime_y_zero_transform_bypass_flag;
    /**
     *  0 - disabled
     *  1 - enabled
     */
    uint8_t separate_colour_plane_flag;
    /** @} */
} VdpPictureInfoH264Predictive;

/**
 * \brief Picture parameter information for a VC1 picture.
 *
 * Note: References to bitstream fields below may refer to data literally parsed
 * from the bitstream, or derived from the bitstream using a mechanism described
 * in the specification.
 */
typedef struct {
    /**
     * Reference used by B and P frames.
     * Set to VDP_INVALID_HANDLE when not used.
     */
    VdpVideoSurface forward_reference;
    /**
     * Reference used by B frames.
     * Set to VDP_INVALID_HANDLE when not used.
     */
    VdpVideoSurface backward_reference;

    /** Number of slices in the bitstream provided. */
    uint32_t slice_count;
    /** I=0, P=1, B=3, BI=4  from 7.1.1.4. */
    uint8_t  picture_type;
    /** Progressive=0, Frame-interlace=2, Field-interlace=3; see VC-1 7.1.1.15. */
    uint8_t  frame_coding_mode;

    /** \name VC-1 bitstream
     *
     * Copies of the VC-1 bitstream fields.
     * @{ */
    /** See VC-1 6.1.5. */
    uint8_t postprocflag;
    /** See VC-1 6.1.8. */
    uint8_t pulldown;
    /** See VC-1 6.1.9. */
    uint8_t interlace;
    /** See VC-1 6.1.10. */
    uint8_t tfcntrflag;
    /** See VC-1 6.1.11. */
    uint8_t finterpflag;
    /** See VC-1 6.1.3. */
    uint8_t psf;
    /** See VC-1 6.2.8. */
    uint8_t dquant;
    /** See VC-1 6.2.3. */
    uint8_t panscan_flag;
    /** See VC-1 6.2.4. */
    uint8_t refdist_flag;
    /** See VC-1 6.2.11. */
    uint8_t quantizer;
    /** See VC-1 6.2.7. */
    uint8_t extended_mv;
    /** See VC-1 6.2.14. */
    uint8_t extended_dmv;
    /** See VC-1 6.2.10. */
    uint8_t overlap;
    /** See VC-1 6.2.9. */
    uint8_t vstransform;
    /** See VC-1 6.2.5. */
    uint8_t loopfilter;
    /** See VC-1 6.2.6. */
    uint8_t fastuvmc;
    /** See VC-1 6.12.15. */
    uint8_t range_mapy_flag;
    uint8_t range_mapy;
    /** See VC-1 6.2.16. */
    uint8_t range_mapuv_flag;
    uint8_t range_mapuv;

    /**
     * See VC-1 J.1.10.
     * Only used by simple and main profiles.
     */
    uint8_t multires;
    /**
     * See VC-1 J.1.16.
     * Only used by simple and main profiles.
     */
    uint8_t syncmarker;
    /**
     * VC-1 SP/MP range reduction control.
     * Only used by simple and main profiles.
     * Bit 0: Copy of rangered VC-1 bitstream field; See VC-1 J.1.17.
     * Bit 1: Copy of rangeredfrm VC-1 bitstream fiels; See VC-1 7.1.13.
     */
    uint8_t rangered;
    /**
     * See VC-1 J.1.17.
     * Only used by simple and main profiles.
     */
    uint8_t maxbframes;
    /** @} */

    /**
     * Out-of-loop deblocking enable.
     * Bit 0 of POSTPROC from VC-1 7.1.1.27
     * Note that bit 1 of POSTPROC (dering enable) should not be included.
     */
    uint8_t deblockEnable;
    /**
     * Parameter used by VC-1 Annex H deblocking algorithm. Note that VDPAU
     * implementations may choose which deblocking algorithm to use.
     * See VC-1 7.1.1.6
     */
    uint8_t pquant;
} VdpPictureInfoVC1;

/**
 * \brief Picture parameter information for an MPEG-4 Part 2 picture.
 *
 * Note: References to bitstream fields below may refer to data literally parsed
 * from the bitstream, or derived from the bitstream using a mechanism described
 * in the specification.
 */
typedef struct {
    /**
     * Reference used by B and P frames.
     * Set to VDP_INVALID_HANDLE when not used.
     */
    VdpVideoSurface forward_reference;
    /**
     * Reference used by B frames.
     * Set to VDP_INVALID_HANDLE when not used.
     */
    VdpVideoSurface backward_reference;

    /** \name MPEG 4 part 2 bitstream
     *
     * Copies of the MPEG 4 part 2 bitstream fields.
     * @{ */
    int32_t trd[2];
    int32_t trb[2];
    uint16_t vop_time_increment_resolution;
    uint8_t vop_coding_type;
    uint8_t vop_fcode_forward;
    uint8_t vop_fcode_backward;
    uint8_t resync_marker_disable;
    uint8_t interlaced;
    uint8_t quant_type;
    uint8_t quarter_sample;
    uint8_t short_video_header;
    /** Derived from vop_rounding_type bitstream field. */
    uint8_t rounding_control;
    uint8_t alternate_vertical_scan_flag;
    uint8_t top_field_first;
    uint8_t intra_quantizer_matrix[64];
    uint8_t non_intra_quantizer_matrix[64];
    /** @} */
} VdpPictureInfoMPEG4Part2;

/**
 * \brief Picture parameter information for a DivX 4 picture.
 *
 * Due to similarites between MPEG-4 Part 2 and DivX 4, the picture
 * parameter structure is re-used.
 */
typedef VdpPictureInfoMPEG4Part2 VdpPictureInfoDivX4;

/**
 * \brief Picture parameter information for a DivX 5 picture.
 *
 * Due to similarites between MPEG-4 Part 2 and DivX 5, the picture
 * parameter structure is re-used.
 */
typedef VdpPictureInfoMPEG4Part2 VdpPictureInfoDivX5;

typedef struct
{
    unsigned int width;
    unsigned int height;

    //Frame Indices
    VdpVideoSurface lastReference;
    VdpVideoSurface goldenReference;
    VdpVideoSurface altReference;

    unsigned char colorSpace;

    unsigned short profile;
    unsigned short frameContextIdx;
    unsigned short keyFrame;
    unsigned short showFrame;
    unsigned short errorResilient;
    unsigned short frameParallelDecoding;
    unsigned short subSamplingX;
    unsigned short subSamplingY;
    unsigned short intraOnly;
    unsigned short allowHighPrecisionMv;
    unsigned short refreshEntropyProbs;

    unsigned char  refFrameSignBias[4];

    unsigned char bitDepthMinus8Luma;
    unsigned char bitDepthMinus8Chroma;
    unsigned char loopFilterLevel;
    unsigned char loopFilterSharpness;

    unsigned char modeRefLfEnabled;
    unsigned char log2TileColumns;
    unsigned char log2TileRows;

    unsigned char segmentEnabled;
    unsigned char segmentMapUpdate;
    unsigned char segmentMapTemporalUpdate;
    unsigned char segmentFeatureMode;

    unsigned char segmentFeatureEnable[8][4];
    short         segmentFeatureData[8][4];
    unsigned char mbSegmentTreeProbs[7];
    unsigned char segmentPredProbs[3];
    unsigned char reservedSegment16Bits[2];

    int qpYAc;
    int qpYDc;
    int qpChDc;
    int qpChAc;

    unsigned int activeRefIdx[3];
    unsigned int resetFrameContext;
    unsigned int mcompFilterType;
    unsigned int mbRefLfDelta[4];
    unsigned int mbModeLfDelta[2];
    unsigned int uncompressedHeaderSize;
    unsigned int compressedHeaderSize;
} VdpPictureInfoVP9;

/**
 * \brief Picture parameter information for an H.265/HEVC picture.
 *
 * References to bitsream fields below may refer to data literally parsed from
 * the bitstream, or derived from the bitstream using a mechanism described in
 * Rec. ITU-T H.265 (04/2013), hereafter referred to as "the H.265/HEVC
 * Specification".
 *
 * VDPAU H.265/HEVC implementations implement the portion of the decoding
 * process described by clauses 8.4, 8.5, 8.6 and 8.7 of the the
 * H.265/HEVC Specification. VdpPictureInfoHEVC provides enough data
 * to complete this portion of the decoding process, plus additional
 * information not defined in the H.265/HEVC Specification that may be
 * useful to particular implementations.
 *
 * Client applications must supply every field in this struct.
 */
typedef struct {
    /** \name HEVC Sequence Parameter Set
     *
     * Copies of the HEVC Sequence Parameter Set bitstream fields.
     * @{ */
    uint8_t chroma_format_idc;
    /** Only valid if chroma_format_idc == 3. Ignored otherwise.*/
    uint8_t separate_colour_plane_flag;
    uint32_t pic_width_in_luma_samples;
    uint32_t pic_height_in_luma_samples;
    uint8_t bit_depth_luma_minus8;
    uint8_t bit_depth_chroma_minus8;
    uint8_t log2_max_pic_order_cnt_lsb_minus4;
    /** Provides the value corresponding to the nuh_temporal_id of the frame
        to be decoded. */
    uint8_t sps_max_dec_pic_buffering_minus1;
    uint8_t log2_min_luma_coding_block_size_minus3;
    uint8_t log2_diff_max_min_luma_coding_block_size;
    uint8_t log2_min_transform_block_size_minus2;
    uint8_t log2_diff_max_min_transform_block_size;
    uint8_t max_transform_hierarchy_depth_inter;
    uint8_t max_transform_hierarchy_depth_intra;
    uint8_t scaling_list_enabled_flag;
    /** Scaling lists, in diagonal order, to be used for this frame. */
    /** Scaling List for 4x4 quantization matrix,
       indexed as ScalingList4x4[matrixId][i]. */
    uint8_t ScalingList4x4[6][16];
    /** Scaling List for 8x8 quantization matrix,
       indexed as ScalingList8x8[matrixId][i]. */
    uint8_t ScalingList8x8[6][64];
    /** Scaling List for 16x16 quantization matrix,
       indexed as ScalingList16x16[matrixId][i]. */
    uint8_t ScalingList16x16[6][64];
    /** Scaling List for 32x32 quantization matrix,
       indexed as ScalingList32x32[matrixId][i]. */
    uint8_t ScalingList32x32[2][64];
    /** Scaling List DC Coefficients for 16x16,
       indexed as ScalingListDCCoeff16x16[matrixId]. */
    uint8_t ScalingListDCCoeff16x16[6];
    /** Scaling List DC Coefficients for 32x32,
       indexed as ScalingListDCCoeff32x32[matrixId]. */
    uint8_t ScalingListDCCoeff32x32[2];
    uint8_t amp_enabled_flag;
    uint8_t sample_adaptive_offset_enabled_flag;
    uint8_t pcm_enabled_flag;
    /** Only needs to be set if pcm_enabled_flag is set. Ignored otherwise. */
    uint8_t pcm_sample_bit_depth_luma_minus1;
    /** Only needs to be set if pcm_enabled_flag is set. Ignored otherwise. */
    uint8_t pcm_sample_bit_depth_chroma_minus1;
    /** Only needs to be set if pcm_enabled_flag is set. Ignored otherwise. */
    uint8_t log2_min_pcm_luma_coding_block_size_minus3;
    /** Only needs to be set if pcm_enabled_flag is set. Ignored otherwise. */
    uint8_t log2_diff_max_min_pcm_luma_coding_block_size;
    /** Only needs to be set if pcm_enabled_flag is set. Ignored otherwise. */
    uint8_t pcm_loop_filter_disabled_flag;
    /** Per spec, when zero, assume short_term_ref_pic_set_sps_flag
        is also zero. */
    uint8_t num_short_term_ref_pic_sets;
    uint8_t long_term_ref_pics_present_flag;
    /** Only needed if long_term_ref_pics_present_flag is set. Ignored
        otherwise. */
    uint8_t num_long_term_ref_pics_sps;
    uint8_t sps_temporal_mvp_enabled_flag;
    uint8_t strong_intra_smoothing_enabled_flag;
    /** @} */

    /** \name HEVC Picture Parameter Set
     *
     * Copies of the HEVC Picture Parameter Set bitstream fields.
     * @{ */
    uint8_t dependent_slice_segments_enabled_flag;
    uint8_t output_flag_present_flag;
    uint8_t num_extra_slice_header_bits;
    uint8_t sign_data_hiding_enabled_flag;
    uint8_t cabac_init_present_flag;
    uint8_t num_ref_idx_l0_default_active_minus1;
    uint8_t num_ref_idx_l1_default_active_minus1;
    int8_t init_qp_minus26;
    uint8_t constrained_intra_pred_flag;
    uint8_t transform_skip_enabled_flag;
    uint8_t cu_qp_delta_enabled_flag;
    /** Only needed if cu_qp_delta_enabled_flag is set. Ignored otherwise. */
    uint8_t diff_cu_qp_delta_depth;
    int8_t pps_cb_qp_offset;
    int8_t pps_cr_qp_offset;
    uint8_t pps_slice_chroma_qp_offsets_present_flag;
    uint8_t weighted_pred_flag;
    uint8_t weighted_bipred_flag;
    uint8_t transquant_bypass_enabled_flag;
    uint8_t tiles_enabled_flag;
    uint8_t entropy_coding_sync_enabled_flag;
    /** Only valid if tiles_enabled_flag is set. Ignored otherwise. */
    uint8_t num_tile_columns_minus1;
    /** Only valid if tiles_enabled_flag is set. Ignored otherwise. */
    uint8_t num_tile_rows_minus1;
    /** Only valid if tiles_enabled_flag is set. Ignored otherwise. */
    uint8_t uniform_spacing_flag;
    /** Only need to set 0..num_tile_columns_minus1. The struct
        definition reserves up to the maximum of 20. Invalid values are
        ignored. */
    uint16_t column_width_minus1[20];
    /** Only need to set 0..num_tile_rows_minus1. The struct
        definition reserves up to the maximum of 22. Invalid values are
        ignored.*/
    uint16_t row_height_minus1[22];
    /** Only needed if tiles_enabled_flag is set. Invalid values are
        ignored. */
    uint8_t loop_filter_across_tiles_enabled_flag;
    uint8_t pps_loop_filter_across_slices_enabled_flag;
    uint8_t deblocking_filter_control_present_flag;
    /** Only valid if deblocking_filter_control_present_flag is set. Ignored
        otherwise. */
    uint8_t deblocking_filter_override_enabled_flag;
    /** Only valid if deblocking_filter_control_present_flag is set. Ignored
        otherwise. */
    uint8_t pps_deblocking_filter_disabled_flag;
    /** Only valid if deblocking_filter_control_present_flag is set and
        pps_deblocking_filter_disabled_flag is not set. Ignored otherwise.*/
    int8_t pps_beta_offset_div2;
    /** Only valid if deblocking_filter_control_present_flag is set and
        pps_deblocking_filter_disabled_flag is not set. Ignored otherwise. */
    int8_t pps_tc_offset_div2;
    uint8_t lists_modification_present_flag;
    uint8_t log2_parallel_merge_level_minus2;
    uint8_t slice_segment_header_extension_present_flag;
    /** @} */

    /** \name HEVC Slice Segment Header
     *
     * Copies of the HEVC Slice Segment Header bitstream fields and calculated
     * values detailed in the specification.
     * @{ */
    /** Set to 1 if nal_unit_type is equal to IDR_W_RADL or IDR_N_LP.
        Set to zero otherwise. */
    uint8_t IDRPicFlag;
    /** Set to 1 if nal_unit_type in the range of BLA_W_LP to
        RSV_IRAP_VCL23, inclusive. Set to zero otherwise.*/
    uint8_t RAPPicFlag;
    /** See section 7.4.7.1 of the specification. */
    uint8_t CurrRpsIdx;
    /** See section 7.4.7.2 of the specification. */
    uint32_t NumPocTotalCurr;
    /** Corresponds to specification field, NumDeltaPocs[RefRpsIdx].
        Only applicable when short_term_ref_pic_set_sps_flag == 0.
        Implementations will ignore this value in other cases. See 7.4.8. */
    uint32_t NumDeltaPocsOfRefRpsIdx;
    /** Section 7.6.3.1 of the H.265/HEVC Specification defines the syntax of
        the slice_segment_header. This header contains information that
        some VDPAU implementations may choose to skip. The VDPAU API
        requires client applications to track the number of bits used in the
        slice header for structures associated with short term and long term
        reference pictures. First, VDPAU requires the number of bits used by
        the short_term_ref_pic_set array in the slice_segment_header. */
    uint32_t NumShortTermPictureSliceHeaderBits;
    /** Second, VDPAU requires the number of bits used for long term reference
        pictures in the slice_segment_header. This is equal to the number
        of bits used for the contents of the block beginning with
        "if(long_term_ref_pics_present_flag)". */
    uint32_t NumLongTermPictureSliceHeaderBits;
    /** @} */

    /** Slice Decoding Process - Picture Order Count */
    /** The value of PicOrderCntVal of the picture in the access unit
        containing the SEI message. The picture being decoded. */
    int32_t CurrPicOrderCntVal;

    /** Slice Decoding Process - Reference Picture Sets */
    /** Array of video reference surfaces.
        Set any unused positions to VDP_INVALID_HANDLE. */
    VdpVideoSurface RefPics[16];
    /** Array of picture order counts. These correspond to positions
        in the RefPics array. */
    int32_t PicOrderCntVal[16];
    /** Array used to specify whether a particular RefPic is
        a long term reference. A value of "1" indicates a long-term
        reference. */
    uint8_t IsLongTerm[16];
    /** Copy of specification field, see Section 8.3.2 of the
        H.265/HEVC Specification. */
    uint8_t NumPocStCurrBefore;
    /** Copy of specification field, see Section 8.3.2 of the
        H.265/HEVC Specification. */
    uint8_t NumPocStCurrAfter;
    /** Copy of specification field, see Section 8.3.2 of the
        H.265/HEVC Specification. */
    uint8_t NumPocLtCurr;
    /** Reference Picture Set list, one of the short-term RPS. These
        correspond to positions in the RefPics array. */
    uint8_t RefPicSetStCurrBefore[8];
    /** Reference Picture Set list, one of the short-term RPS. These
        correspond to positions in the RefPics array. */
    uint8_t RefPicSetStCurrAfter[8];
    /** Reference Picture Set list, one of the long-term RPS. These
        correspond to positions in the RefPics array. */
    uint8_t RefPicSetLtCurr[8];
} VdpPictureInfoHEVC;

/**
 * \brief Picture parameter information for an HEVC 444 picture.
 *
 * Note: VDPAU clients must use VdpPictureInfoHEVC444 to describe the
 * attributes of a frame being decoded with
 * VDP_DECODER_PROFILE_HEVC_MAIN_444.
 */
typedef struct {
    /** \ref VdpPictureInfoHEVC struct. */
    VdpPictureInfoHEVC pictureInfo;

    /* SPS Range Extensions for Main 444, Main 10, etc. */
    uint8_t sps_range_extension_flag;
    /* sps extension for transform_skip_rotation_enabled_flag */
    uint8_t transformSkipRotationEnableFlag;
    /* sps extension for transform_skip_context_enabled_flag */
    uint8_t transformSkipContextEnableFlag;
    /* sps implicit_rdpcm_enabled_flag */
    uint8_t implicitRdpcmEnableFlag;
    /* sps explicit_rdpcm_enabled_flag */
    uint8_t explicitRdpcmEnableFlag;
    /* sps extended_precision_processing_flag,always 0 in current profile */
    uint8_t extendedPrecisionProcessingFlag;
    /* sps intra_smoothing_disabled_flag */
    uint8_t intraSmoothingDisabledFlag;
    /* sps high_precision_offsets_enabled_flag */
    uint8_t highPrecisionOffsetsEnableFlag;
    /* sps persistent_rice_adaptation_enabled_flag */
    uint8_t persistentRiceAdaptationEnableFlag;
    /* sps cabac_bypass_alignment_enabled_flag, always 0 in current profile */
    uint8_t cabacBypassAlignmentEnableFlag;
    /* sps intraBlockCopyEnableFlag, always 0 not used by the spec as of now */
    uint8_t intraBlockCopyEnableFlag;

    /* PPS Range Extensions for Main 444, Main 10, etc. */
    uint8_t pps_range_extension_flag;
    /* pps extension log2_max_transform_skip_block_size_minus2, 0...5 */
    uint8_t log2MaxTransformSkipSize;
    /* pps cross_component_prediction_enabled_flag */
    uint8_t crossComponentPredictionEnableFlag;
    /* pps chroma_qp_adjustment_enabled_flag */
    uint8_t chromaQpAdjustmentEnableFlag;
    /* pps diff_cu_chroma_qp_adjustment_depth, 0...3 */
    uint8_t diffCuChromaQpAdjustmentDepth;
    /* pps chroma_qp_adjustment_table_size_minus1+1, 1...6 */
    uint8_t chromaQpAdjustmentTableSize;
    /* pps log2_sao_offset_scale_luma, max(0,bitdepth-10), */
    /* maxBitdepth 16 for future. */
    uint8_t log2SaoOffsetScaleLuma;
    /* pps log2_sao_offset_scale_chroma */
    uint8_t log2SaoOffsetScaleChroma;
    /* -[12,+12] */
    int8_t cb_qp_adjustment[6];
    /* -[12,+12] */
    int8_t cr_qp_adjustment[6];

} VdpPictureInfoHEVC444;

/**
 * \brief Picture parameter information for HEVC FormatRangeExtensions picture.
 *
 * HEVC Main 444 Profile is part of Format Range Extensions profiles,
 * Due to similarities between Format Range Extensions profiles, the picture
 * parameter structure is re-used for Format Range Extensions profiles
 * supported.
 */
typedef VdpPictureInfoHEVC444 VdpPictureInfoHEVCRangeExt;

/** @} */

/**
 * \defgroup VdpVideoMixer VdpVideoMixer; Video Post-processing and Compositing object
 *
 * VdpVideoMixer can perform some subset of the following
 * post-processing steps on video:
 * - De-interlacing
 *   - Various types, with or without inverse telecine
 * - Noise-reduction
 * - Sharpness adjustment
 * - Color space conversion to RGB
 * - Chroma format upscaling to 4:4:4
 *
 * A VdpVideoMixer takes a source \ref VdpVideoSurface
 * "VdpVideoSurface" VdpVideoSurface and performs various video
 * processing steps on it (potentially using information from
 * past or future video surfaces). It scales the video and
 * converts it to RGB, then optionally composites it with
 * multiple auxiliary \ref VdpOutputSurface "VdpOutputSurface"s
 * before writing the result to the destination \ref
 * VdpOutputSurface "VdpOutputSurface".
 *
 * The video mixer compositing model is as follows:
 *
 * - A rectangle will be rendered on an output surface. No
 *   pixels will be rendered outside of this output rectangle.
 *   The contents of this rectangle will be a composite of many
 *   layers.
 *
 * - The first layer is the background color. The background
 *   color will fill the entire rectangle.
 *
 * - The second layer is the processed video which has been
 *   converted to RGB. These pixels will overwrite the
 *   background color of the first layer except where the second
 *   layer's rectangle does not completely cover the output
 *   rectangle. In those regions the background color will
 *   continue to show. If any portion of the second layer's
 *   output rectangle is outside of the output rectangle, those
 *   portions will be clipped.
 *
 * - The third layer contains some number of auxiliary layers
 *   (in the form of \ref VdpOutputSurface "VdpOutputSurface"s)
 *   which will be composited using the alpha value from the
 *   those surfaces. The compositing operations are equivalent
 *   to rendering with \ref VdpOutputSurfaceRenderOutputSurface
 *   using a source blend factor of SOURCE_ALPHA, a destination
 *   blend factor of ONE_MINUS_SOURCE_ALPHA and an equation of
 *   ADD.
 *
 * @{
 */

/**
 * \brief A VdpVideoMixer feature that must be requested at
 *        creation time to be used.
 *
 * Certain advanced VdpVideoMixer features are optional, and the
 * ability to use those features at all must be requested when
 * the VdpVideoMixer object is created. Each feature is named via
 * a specific VdpVideoMixerFeature value.
 *
 * Once requested, these features are permanently available
 * within that specific VdpVideoMixer object. All features that
 * are not explicitly requested at creation time default to
 * being permanently unavailable.
 *
 * Even when requested, all features default to being initially
 * disabled. However, applications can subsequently enable and
 * disable features at any time. See \ref
 * VdpVideoMixerSetFeatureEnables.
 *
 * Some features allow configuration of their operation. Each
 * configurable item is an \ref VdpVideoMixerAttribute. These
 * attributes may be manipulated at any time using \ref
 * VdpVideoMixerSetAttributeValues.
 */
typedef uint32_t VdpVideoMixerFeature;

/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * When requested and enabled, motion adaptive temporal
 * deinterlacing will be used on interlaced content.
 *
 * When multiple de-interlacing options are requested and
 * enabled, the back-end implementation chooses the best
 * algorithm to apply.
 */
#define VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL         ((VdpVideoMixerFeature)0)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * When requested and enabled, this enables a more advanced
 * version of temporal de-interlacing, that additionally uses
 * edge-guided spatial interpolation.
 *
 * When multiple de-interlacing options are requested and
 * enabled, the back-end implementation chooses the best
 * algorithm to apply.
 */
#define VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL_SPATIAL ((VdpVideoMixerFeature)1)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * When requested and enabled, cadence detection will be enabled
 * on interlaced content and the video mixer will try to extract
 * progressive frames from pull-down material.
 */
#define VDP_VIDEO_MIXER_FEATURE_INVERSE_TELECINE             ((VdpVideoMixerFeature)2)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * When requested and enabled, a noise reduction algorithm will
 * be applied to the video.
 */
#define VDP_VIDEO_MIXER_FEATURE_NOISE_REDUCTION              ((VdpVideoMixerFeature)3)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * When requested and enabled, a sharpening algorithm will be
 * applied to the video.
 */
#define VDP_VIDEO_MIXER_FEATURE_SHARPNESS                    ((VdpVideoMixerFeature)4)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * When requested and enabled, the alpha of the rendered
 * surface, which is normally set to the alpha of the background
 * color, will be forced to 0.0 on pixels corresponding to
 * source video surface luminance values in the range specified
 * by attributes \ref VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MIN_LUMA
 * to \ref VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MAX_LUMA. This
 * keying is performed after scaling and de-interlacing.
 */
#define VDP_VIDEO_MIXER_FEATURE_LUMA_KEY                     ((VdpVideoMixerFeature)5)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * A VDPAU implementation may support multiple scaling algorithms of
 * differing quality, and may potentially support a different subset
 * of algorithms on different hardware.
 *
 * In some cases, higher quality algorithms may require more resources
 * (memory size, memory bandwidth, etc.) to operate. Hence, these high
 * quality algorithms must be explicitly requested and enabled by the client
 * application. This allows applications operating in a resource-constrained
 * environment to have some level of control over resource usage.
 *
 * Basic scaling is always built into any video mixer, and is known as
 * level 0. Scaling quality increases beginning with optional level 1,
 * through optional level 9.
 *
 * If an application requests and enables multiple high quality scaling
 * algorithms, the highest level enabled scaling algorithm will be used.
 */
#define VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1      ((VdpVideoMixerFeature)11)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * See \ref VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1 for details.
 */
#define VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L2      ((VdpVideoMixerFeature)12)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * See \ref VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1 for details.
 */
#define VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L3      ((VdpVideoMixerFeature)13)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * See \ref VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1 for details.
 */
#define VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L4      ((VdpVideoMixerFeature)14)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * See \ref VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1 for details.
 */
#define VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L5      ((VdpVideoMixerFeature)15)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * See \ref VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1 for details.
 */
#define VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L6      ((VdpVideoMixerFeature)16)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * See \ref VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1 for details.
 */
#define VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L7      ((VdpVideoMixerFeature)17)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * See \ref VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1 for details.
 */
#define VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L8      ((VdpVideoMixerFeature)18)
/**
 * \hideinitializer
 * \brief A VdpVideoMixerFeature.
 *
 * See \ref VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1 for details.
 */
#define VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L9      ((VdpVideoMixerFeature)19)

/**
 * \brief A VdpVideoMixer creation parameter.
 *
 * When a VdpVideoMixer is created, certain parameters may be
 * supplied. Each parameter is named via a specific
 * VdpVideoMixerParameter value.
 *
 * Each parameter has a specific type, and specific default
 * value if not specified at VdpVideoMixer creation time. The
 * application may query the legal supported range for some
 * parameters.
 */
typedef uint32_t VdpVideoMixerParameter;

/**
 * \hideinitializer
 * \brief The exact width of input video surfaces.
 *
 * This parameter's type is uint32_t.
 *
 * This parameter defaults to 0 if not specified, which entails
 * that it must be specified.
 *
 * The application may query this parameter's supported
 * range.
 */
#define VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH  ((VdpVideoMixerParameter)0)
/**
 * \hideinitializer
 * \brief The exact height of input video surfaces.
 *
 * This parameter's type is uint32_t.
 *
 * This parameter defaults to 0 if not specified, which entails
 * that it must be specified.
 *
 * The application may query this parameter's supported
 * range.
 */
#define VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT ((VdpVideoMixerParameter)1)
/**
 * \hideinitializer
 * \brief The chroma type of the input video surfaces the will
 *        process.
 *
 * This parameter's type is VdpChromaType.
 *
 * If not specified, this parameter defaults to
 * VDP_CHROMA_TYPE_420.
 *
 * The application may not query this application's supported
 * range, since it is a potentially disjoint enumeration.
 */
#define VDP_VIDEO_MIXER_PARAMETER_CHROMA_TYPE          ((VdpVideoMixerParameter)2)
/**
 * \hideinitializer
 * \brief The number of auxiliary layers in the mixer's
 *        compositing model.
 *
 * Note that this indicates the maximum number of layers that
 * may be processed by a given \ref VdpVideoMixer "VdpVideoMixer"
 * object. Each individual \ref VdpVideoMixerRender invocation
 * may choose to use a different number of actual layers, from 0
 * up to this limit.
 *
 * This attribute's type is uint32_t.
 *
 * If not specified, this parameter defaults to 0.
 *
 * The application may query this parameter's supported
 * range.
 */
#define VDP_VIDEO_MIXER_PARAMETER_LAYERS               ((VdpVideoMixerParameter)3)

/**
 * \brief An adjustable attribute of VdpVideoMixer operation.
 *
 * Various attributes of VdpVideoMixer operation may be adjusted
 * at any time. Each attribute is named via a specific
 * VdpVideoMixerAttribute value.
 *
 * Each attribute has a specific type, and specific default
 * value if not specified at VdpVideoMixer creation time. The
 * application may query the legal supported range for some
 * attributes.
 */
typedef uint32_t VdpVideoMixerAttribute;

/**
 * \hideinitializer
 * \brief The background color in the VdpVideoMixer's compositing
 *        model.
 *
 * This attribute's type is VdpColor.
 *
 * This parameter defaults to black (all color components 0.0
 * and alpha 1.0).
 *
 * The application may not query this parameter's supported
 * range, since the type is not scalar.
 */
#define VDP_VIDEO_MIXER_ATTRIBUTE_BACKGROUND_COLOR      ((VdpVideoMixerAttribute)0)
/**
 * \hideinitializer
 * \brief The color-space conversion matrix used by the
 *        VdpVideoMixer.
 *
 * This attribute's type is \ref VdpCSCMatrix.
 *
 * Note: When using \ref VdpVideoMixerGetAttributeValues to retrieve the
 * current CSC matrix, the attribute_values array must contain a pointer to
 * a pointer a VdpCSCMatrix (VdpCSCMatrix** as a void *). The get function will
 * either initialize the referenced CSC matrix to the current value, *or*
 * clear the supplied pointer to NULL, if the previous set call supplied a
 * value of NULL in parameter_values, to request the default matrix.
 *
 * \code
 * VdpCSCMatrix   matrix;
 * VdpCSCMatrix * matrix_ptr;
 * void * attribute_values[] = {&matrix_ptr};
 * VdpStatus st = vdp_video_mixer_get_attribute_values(..., attribute_values, ...);
 * \endcode
 *
 * This parameter defaults to a matrix suitable for ITU-R BT.601
 * input surfaces, with no procamp adjustments.
 *
 * The application may not query this parameter's supported
 * range, since the type is not scalar.
 */
#define VDP_VIDEO_MIXER_ATTRIBUTE_CSC_MATRIX            ((VdpVideoMixerAttribute)1)
/**
 * \hideinitializer
 * \brief The amount of noise reduction algorithm to apply.
 *
 * This attribute's type is float.
 *
 * This parameter defaults to 0.0, which equates to no noise
 * reduction.
 *
 * The application may query this parameter's supported range.
 * However, the range is fixed as 0.0...1.0.
 */
#define VDP_VIDEO_MIXER_ATTRIBUTE_NOISE_REDUCTION_LEVEL ((VdpVideoMixerAttribute)2)
/**
 * \hideinitializer
 * \brief The amount of sharpening, or blurring, to apply.
 *
 * This attribute's type is float.
 *
 * This parameter defaults to 0.0, which equates to no
 * sharpening.
 *
 * Positive values request sharpening. Negative values request
 * blurring.
 *
 * The application may query this parameter's supported range.
 * However, the range is fixed as -1.0...1.0.
 */
#define VDP_VIDEO_MIXER_ATTRIBUTE_SHARPNESS_LEVEL       ((VdpVideoMixerAttribute)3)
/**
 * \hideinitializer
 * \brief The minimum luma value for the luma key algorithm.
 *
 * This attribute's type is float.
 *
 * This parameter defaults to 0.0.
 *
 * The application may query this parameter's supported range.
 * However, the range is fixed as 0.0...1.0.
 */
#define VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MIN_LUMA     ((VdpVideoMixerAttribute)4)
/**
 * \hideinitializer
 * \brief The maximum luma value for the luma key algorithm.
 *
 * This attribute's type is float.
 *
 * This parameter defaults to 1.0.
 *
 * The application may query this parameter's supported range.
 * However, the range is fixed as 0.0...1.0.
 */
#define VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MAX_LUMA     ((VdpVideoMixerAttribute)5)
/**
 * \hideinitializer
 * \brief Whether de-interlacers should operate solely on luma, and bob chroma.
 *
 * Note: This attribute only affects advanced de-interlacing algorithms, not
 * bob or weave.
 *
 * This attribute's type is uint8_t.
 *
 * This parameter defaults to 0.
 *
 * The application may query this parameter's supported range.
 * However, the range is fixed as 0 (no/off) ... 1 (yes/on).
 */
#define VDP_VIDEO_MIXER_ATTRIBUTE_SKIP_CHROMA_DEINTERLACE ((VdpVideoMixerAttribute)6)

/**
 * \brief An opaque handle representing a VdpVideoMixer object.
 */
typedef uint32_t VdpVideoMixer;

/**
 * \hideinitializer
 * \brief The structure of the picture present in a \ref
 *        VdpVideoSurface "VdpVideoSurface".
 */
typedef enum {
    /**
     * The picture is a field, and is the top field of the surface.
     */
    VDP_VIDEO_MIXER_PICTURE_STRUCTURE_TOP_FIELD,
    /**
     * The picture is a field, and is the bottom field of the
     * surface.
     */
    VDP_VIDEO_MIXER_PICTURE_STRUCTURE_BOTTOM_FIELD,
    /**
     * The picture is a frame, and hence is the entire surface.
     */
    VDP_VIDEO_MIXER_PICTURE_STRUCTURE_FRAME,
} VdpVideoMixerPictureStructure;

#define VDP_LAYER_VERSION 0

/**
 * \brief Definition of an additional \ref VdpOutputSurface
 *        "VdpOutputSurface" layer in the composting model.
 */
typedef struct {
    /**
     * This field must be filled with VDP_LAYER_VERSION
     */
    uint32_t struct_version;
    /**
     * The surface to composite from.
     */
    VdpOutputSurface source_surface;
    /**
     * The sub-rectangle of the source surface to use. If NULL, the
     * entire source surface will be used.
     */
    VdpRect const *  source_rect;
    /**
     * The sub-rectangle of the destination surface to map
     * this layer into. This rectangle is relative to the entire
     * destination surface. This rectangle will be clipped by \ref
     * VdpVideoMixerRender's \b destination_rect. If NULL, the
     * destination rectangle will be sized to match the source
     * rectangle, and will be located at the origin.
     */
     VdpRect const * destination_rect;
} VdpLayer;

/** @} */

/**
 * \defgroup VdpPresentationQueue VdpPresentationQueue; Video presentation (display) object
 *
 * The VdpPresentationQueue manages a queue of surfaces and
 * associated timestamps. For each surface in the queue, once
 * the associated timestamp is reached, the surface is displayed
 * to the user. This timestamp-based approach yields high
 * quality video delivery.
 *
 * The exact location of the displayed content is Window System
 * specific. For this reason, the \ref api_winsys provides an
 * API to create a \ref VdpPresentationQueueTarget object (e.g.
 * via \ref VdpPresentationQueueTargetCreateX11) which
 * encapsulates this information.
 *
 * Note that the presentation queue performs no scaling of
 * surfaces to match the display target's size, aspect ratio,
 * etc.
 *
 * Surfaces that are too large to fit into the display target
 * will be clipped. Surfaces that are too small to fill the
 * display target will be aligned to the top-left corner of the
 * display target, with the balance of the display target being
 * filled with a constant configurable "background" color.
 *
 * Note that the presentation queue operates in a manner that is
 * semantically equivalent to an overlay surface, with any
 * required color key painting hidden internally. However,
 * implementations are free to use whatever semantically
 * equivalent technique they wish. Note that implementations
 * that actually use color-keyed overlays will typically use
 * the "background" color as the overlay color key value, so
 * this color should be chosen with care.
 *
 * @{
 */

/**
 * \brief The representation of a point in time.
 *
 * VdpTime timestamps are intended to be a high-precision timing
 * system, potentially independent from any other time domain in
 * the system.
 *
 * Time is represented in units of nanoseconds. The origin
 * (i.e. the time represented by a value of 0) is implementation
 * dependent.
 */
typedef uint64_t VdpTime;

/**
 * \brief An opaque handle representing the location where
 *        video will be presented.
 *
 * VdpPresentationQueueTarget are created using a \ref api_winsys
 * specific API, such as \ref
 * VdpPresentationQueueTargetCreateX11.
 */
typedef uint32_t VdpPresentationQueueTarget;

/**
 * \brief An opaque handle representing a presentation queue
 *        object.
 */
typedef uint32_t VdpPresentationQueue;

/**
 * \hideinitializer
 * \brief The status of a surface within a presentation queue.
 */
typedef enum {
    /** The surface is not queued or currently visible. */
    VDP_PRESENTATION_QUEUE_STATUS_IDLE,
    /** The surface is in the queue, and not currently visible. */
    VDP_PRESENTATION_QUEUE_STATUS_QUEUED,
    /** The surface is the currently visible surface. */
    VDP_PRESENTATION_QUEUE_STATUS_VISIBLE,
} VdpPresentationQueueStatus;

/** @} */

/**
 * \defgroup display_preemption Display Preemption
 *
 * The Window System may operate within a frame-work (such as
 * Linux's VT switching) where the display is shared between the
 * Window System (e.g. X) and some other output mechanism (e.g.
 * the VT.) Given this scenario, the Window System's control of
 * the display could be preempted, and restored, at any time.
 *
 * VDPAU does not mandate that implementations hide such
 * preemptions from VDPAU client applications; doing so may
 * impose extreme burdens upon VDPAU implementations. Equally,
 * however, implementations are free to hide such preemptions
 * from client applications.
 *
 * VDPAU allows implementations to inform the client application
 * when such a preemption has occurred, and then refuse to
 * continue further operation.
 *
 * Similarly, some form of fatal hardware error could prevent further
 * operation of the VDPAU implementation, without a complete
 * re-initialization.
 *
 * The following discusses the behavior of implementations that
 * choose not to hide preemption from client applications.
 *
 * When preemption occurs, VDPAU internally destroys all
 * objects; the client application need not do this. However, if
 * the client application wishes to continue operation, it must
 * recreate all objects that it uses. It is probable that this
 * recreation will not succeed until the display ownership is
 * restored to the Window System.
 *
 * Once preemption has occurred, all VDPAU entry points will
 * return the specific error code \ref
 * VDP_STATUS_DISPLAY_PREEMPTED.
 *
 * VDPAU client applications may also be notified of such
 * preemptions and fatal errors via a callback. See \ref
 * VdpPreemptionCallbackRegister for more details.
 *
 * @{
 */

/**
 * \brief A callback to notify the client application that a
 *        device's display has been preempted.
 * \param[in] device The device that had its display preempted.
 * \param[in] context The client-supplied callback context
 *       information.
 * \return void No return value
 */
typedef void VdpPreemptionCallback(
    VdpDevice device,
    void *   context
);

/** @} */

/**
 * \defgroup get_proc_address Entry Point Retrieval
 *
 * In order to facilitate multiple implementations of VDPAU
 * co-existing within a single process, all functionality is
 * available via function pointers. The mechanism to retrieve
 * those function pointers is described below.
 *
 * @{
 */

/**
 * \brief A type suitable for \ref VdpGetProcAddress
 *        "VdpGetProcAddress"'s \b function_id parameter.
 */
typedef uint32_t VdpFuncId;

/** \hideinitializer */
#define VDP_FUNC_ID_GET_ERROR_STRING                                            ((VdpFuncId)0)
/** \hideinitializer */
#define VDP_FUNC_ID_GET_PROC_ADDRESS                                            ((VdpFuncId)1)
/** \hideinitializer */
#define VDP_FUNC_ID_GET_API_VERSION                                             ((VdpFuncId)2)
/** \hideinitializer */
#define VDP_FUNC_ID_GET_INFORMATION_STRING                                      ((VdpFuncId)4)
/** \hideinitializer */
#define VDP_FUNC_ID_DEVICE_DESTROY                                              ((VdpFuncId)5)
/** \hideinitializer */
#define VDP_FUNC_ID_GENERATE_CSC_MATRIX                                         ((VdpFuncId)6)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_SURFACE_QUERY_CAPABILITIES                            ((VdpFuncId)7)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_SURFACE_QUERY_GET_PUT_BITS_Y_CB_CR_CAPABILITIES       ((VdpFuncId)8)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_SURFACE_CREATE                                        ((VdpFuncId)9)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_SURFACE_DESTROY                                       ((VdpFuncId)10)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_SURFACE_GET_PARAMETERS                                ((VdpFuncId)11)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR                              ((VdpFuncId)12)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_SURFACE_PUT_BITS_Y_CB_CR                              ((VdpFuncId)13)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_CAPABILITIES                           ((VdpFuncId)14)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_GET_PUT_BITS_NATIVE_CAPABILITIES       ((VdpFuncId)15)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_INDEXED_CAPABILITIES          ((VdpFuncId)16)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_Y_CB_CR_CAPABILITIES          ((VdpFuncId)17)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_CREATE                                       ((VdpFuncId)18)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY                                      ((VdpFuncId)19)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS                               ((VdpFuncId)20)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_GET_BITS_NATIVE                              ((VdpFuncId)21)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_NATIVE                              ((VdpFuncId)22)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_INDEXED                             ((VdpFuncId)23)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_Y_CB_CR                             ((VdpFuncId)24)
/** \hideinitializer */
#define VDP_FUNC_ID_BITMAP_SURFACE_QUERY_CAPABILITIES                           ((VdpFuncId)25)
/** \hideinitializer */
#define VDP_FUNC_ID_BITMAP_SURFACE_CREATE                                       ((VdpFuncId)26)
/** \hideinitializer */
#define VDP_FUNC_ID_BITMAP_SURFACE_DESTROY                                      ((VdpFuncId)27)
/** \hideinitializer */
#define VDP_FUNC_ID_BITMAP_SURFACE_GET_PARAMETERS                               ((VdpFuncId)28)
/** \hideinitializer */
#define VDP_FUNC_ID_BITMAP_SURFACE_PUT_BITS_NATIVE                              ((VdpFuncId)29)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_OUTPUT_SURFACE                        ((VdpFuncId)33)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_BITMAP_SURFACE                        ((VdpFuncId)34)
/** \hideinitializer */
#define VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_VIDEO_SURFACE_LUMA                    ((VdpFuncId)35)
/** \hideinitializer */
#define VDP_FUNC_ID_DECODER_QUERY_CAPABILITIES                                  ((VdpFuncId)36)
/** \hideinitializer */
#define VDP_FUNC_ID_DECODER_CREATE                                              ((VdpFuncId)37)
/** \hideinitializer */
#define VDP_FUNC_ID_DECODER_DESTROY                                             ((VdpFuncId)38)
/** \hideinitializer */
#define VDP_FUNC_ID_DECODER_GET_PARAMETERS                                      ((VdpFuncId)39)
/** \hideinitializer */
#define VDP_FUNC_ID_DECODER_RENDER                                              ((VdpFuncId)40)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_QUERY_FEATURE_SUPPORT                           ((VdpFuncId)41)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_SUPPORT                         ((VdpFuncId)42)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_SUPPORT                         ((VdpFuncId)43)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_VALUE_RANGE                     ((VdpFuncId)44)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_VALUE_RANGE                     ((VdpFuncId)45)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_CREATE                                          ((VdpFuncId)46)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_SET_FEATURE_ENABLES                             ((VdpFuncId)47)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_SET_ATTRIBUTE_VALUES                            ((VdpFuncId)48)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_SUPPORT                             ((VdpFuncId)49)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_ENABLES                             ((VdpFuncId)50)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_GET_PARAMETER_VALUES                            ((VdpFuncId)51)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_GET_ATTRIBUTE_VALUES                            ((VdpFuncId)52)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_DESTROY                                         ((VdpFuncId)53)
/** \hideinitializer */
#define VDP_FUNC_ID_VIDEO_MIXER_RENDER                                          ((VdpFuncId)54)
/** \hideinitializer */
#define VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_DESTROY                           ((VdpFuncId)55)
/** \hideinitializer */
#define VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE                                   ((VdpFuncId)56)
/** \hideinitializer */
#define VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY                                  ((VdpFuncId)57)
/** \hideinitializer */
#define VDP_FUNC_ID_PRESENTATION_QUEUE_SET_BACKGROUND_COLOR                     ((VdpFuncId)58)
/** \hideinitializer */
#define VDP_FUNC_ID_PRESENTATION_QUEUE_GET_BACKGROUND_COLOR                     ((VdpFuncId)59)
/** \hideinitializer */
#define VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME                                 ((VdpFuncId)62)
/** \hideinitializer */
#define VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY                                  ((VdpFuncId)63)
/** \hideinitializer */
#define VDP_FUNC_ID_PRESENTATION_QUEUE_BLOCK_UNTIL_SURFACE_IDLE                 ((VdpFuncId)64)
/** \hideinitializer */
#define VDP_FUNC_ID_PRESENTATION_QUEUE_QUERY_SURFACE_STATUS                     ((VdpFuncId)65)
/** \hideinitializer */
#define VDP_FUNC_ID_PREEMPTION_CALLBACK_REGISTER                                ((VdpFuncId)66)
/** \hideinitializer */
#define VDP_FUNC_ID_DECODER_QUERY_CAPABILITY                                    ((VdpFuncId)67)

#define VDP_FUNC_ID_BASE_WINSYS 0x1000

/**
 * \brief Retrieve a VDPAU function pointer.
 * \param[in] device The device that the function will operate
 *       against.
 * \param[in] function_id The specific function to retrieve.
 * \param[out] function_pointer The actual pointer for the
 *       application to call.
 * \return VdpStatus The completion status of the operation.
 */
typedef VdpStatus VdpGetProcAddress(
    VdpDevice device,
    VdpFuncId function_id,
    /* output parameters follow */
    void * *  function_pointer
);

/** @} */
/** @} */

/**
 * \defgroup api_winsys Window System Integration Layer
 *
 * The set of VDPAU functionality specific to an individual
 * Windowing System.
 */

#ifdef __cplusplus
}
#endif

#endif
