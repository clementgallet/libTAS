/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_SDLAUDIO_H_INCL
#define LIBTAS_SDLAUDIO_H_INCL

#include "../../global.h"
#include <SDL2/SDL.h>

namespace libtas {

/**
 *  \name Driver discovery functions
 *
 *  These functions return the list of built in audio drivers, in the
 *  order that they are normally initialized by default.
 */
OVERRIDE int SDL_GetNumAudioDrivers(void);
OVERRIDE const char *SDL_GetAudioDriver(int index);

/**
 * This function fills the given character buffer with the name of the
 * current audio driver, and returns a pointer to it if the audio driver has
 * been initialized.  It returns NULL if no driver has been initialized.
 */
OVERRIDE char * SDL_AudioDriverName(char *namebuf, int maxlen);

/**
 *  \name Initialization and cleanup
 *
 *  \internal These functions are used internally, and should not be used unless
 *            you have a specific need to specify the audio driver you want to
 *            use.  You should normally use SDL_Init() or SDL_InitSubSystem().
 */
/* @{ */
OVERRIDE int SDL_AudioInit(const char *driver_name);
OVERRIDE void SDL_AudioQuit(void);
/* @} */

/**
 *  This function returns the name of the current audio driver, or NULL
 *  if no driver has been initialized.
 */
OVERRIDE const char *SDL_GetCurrentAudioDriver(void);

/**
 *  This function opens the audio device with the desired parameters, and
 *  returns 0 if successful, placing the actual hardware parameters in the
 *  structure pointed to by \c obtained.  If \c obtained is NULL, the audio
 *  data passed to the callback function will be guaranteed to be in the
 *  requested format, and will be automatically converted to the hardware
 *  audio format if necessary.  This function returns -1 if it failed
 *  to open the audio device, or couldn't set up the audio thread.
 *
 *  When filling in the desired audio spec structure,
 *    - \c desired->freq should be the desired audio frequency in samples-per-
 *      second.
 *    - \c desired->format should be the desired audio format.
 *    - \c desired->samples is the desired size of the audio buffer, in
 *      samples.  This number should be a power of two, and may be adjusted by
 *      the audio driver to a value more suitable for the hardware.  Good values
 *      seem to range between 512 and 8096 inclusive, depending on the
 *      application and CPU speed.  Smaller values yield faster response time,
 *      but can lead to underflow if the application is doing heavy processing
 *      and cannot fill the audio buffer in time.  A stereo sample consists of
 *      both right and left channels in LR ordering.
 *      Note that the number of samples is directly related to time by the
 *      following formula:  \code ms = (samples*1000)/freq \endcode
 *    - \c desired->size is the size in bytes of the audio buffer, and is
 *      calculated by SDL_OpenAudio().
 *    - \c desired->silence is the value used to set the buffer to silence,
 *      and is calculated by SDL_OpenAudio().
 *    - \c desired->callback should be set to a function that will be called
 *      when the audio device is ready for more data.  It is passed a pointer
 *      to the audio buffer, and the length in bytes of the audio buffer.
 *      This function usually runs in a separate thread, and so you should
 *      protect data structures that it accesses by calling SDL_LockAudio()
 *      and SDL_UnlockAudio() in your code. Alternately, you may pass a NULL
 *      pointer here, and call SDL_QueueAudio() with some frequency, to queue
 *      more audio samples to be played.
 *    - \c desired->userdata is passed as the first parameter to your callback
 *      function. If you passed a NULL callback, this value is ignored.
 *
 *  The audio device starts out playing silence when it's opened, and should
 *  be enabled for playing by calling \c SDL_PauseAudio(0) when you are ready
 *  for your audio callback function to be called.  Since the audio driver
 *  may modify the requested size of the audio buffer, you should allocate
 *  any local mixing buffers after you open the audio device.
 */
OVERRIDE int SDL_OpenAudio(SDL_AudioSpec * desired,
                                          SDL_AudioSpec * obtained);

/**
 *  Get the number of available devices exposed by the current driver.
 *  Only valid after a successfully initializing the audio subsystem.
 *  Returns -1 if an explicit list of devices can't be determined; this is
 *  not an error. For example, if SDL is set up to talk to a remote audio
 *  server, it can't list every one available on the Internet, but it will
 *  still allow a specific host to be specified to SDL_OpenAudioDevice().
 *
 *  In many common cases, when this function returns a value <= 0, it can still
 *  successfully open the default device (NULL for first argument of
 *  SDL_OpenAudioDevice()).
 */
OVERRIDE int SDL_GetNumAudioDevices(int iscapture);

/**
 *  Get the human-readable name of a specific audio device.
 *  Must be a value between 0 and (number of audio devices-1).
 *  Only valid after a successfully initializing the audio subsystem.
 *  The values returned by this function reflect the latest call to
 *  SDL_GetNumAudioDevices(); recall that function to redetect available
 *  hardware.
 *
 *  The string returned by this function is UTF-8 encoded, read-only, and
 *  managed internally. You are not to free it. If you need to keep the
 *  string for any length of time, you should make your own copy of it, as it
 *  will be invalid next time any of several other SDL functions is called.
 */
OVERRIDE const char *SDL_GetAudioDeviceName(int index, int iscapture);

/**
 * Get the preferred audio format of a specific audio device.
 *
 * This function is only valid after a successfully initializing the audio
 * subsystem. The values returned by this function reflect the latest call to
 * SDL_GetNumAudioDevices(); re-call that function to redetect available
 * hardware.
 *
 * `spec` will be filled with the sample rate, sample format, and channel
 * count.
 *
 * \param index the index of the audio device; valid values range from 0 to
 *              SDL_GetNumAudioDevices() - 1
 * \param iscapture non-zero to query the list of recording devices, zero to
 *                  query the list of output devices.
 * \param spec The SDL_AudioSpec to be initialized by this function.
 * \returns 0 on success, nonzero on error
 *
 * \since This function is available since SDL 2.0.16.
 *
 * \sa SDL_GetNumAudioDevices
 * \sa SDL_GetDefaultAudioInfo
 */

OVERRIDE int SDL_GetAudioDeviceSpec(int index, int iscapture, SDL_AudioSpec *spec);

/**
 * Get the name and preferred format of the default audio device.
 *
 * Some (but not all!) platforms have an isolated mechanism to get information
 * about the "default" device. This can actually be a completely different
 * device that's not in the list you get from SDL_GetAudioDeviceSpec(). It can
 * even be a network address! (This is discussed in SDL_OpenAudioDevice().)
 *
 * As a result, this call is not guaranteed to be performant, as it can query
 * the sound server directly every time, unlike the other query functions. You
 * should call this function sparingly!
 *
 * `spec` will be filled with the sample rate, sample format, and channel
 * count, if a default device exists on the system. If `name` is provided,
 * will be filled with either a dynamically-allocated UTF-8 string or NULL.
 *
 * \param name A pointer to be filled with the name of the default device (can
 *             be NULL). Please call SDL_free() when you are done with this
 *             pointer!
 * \param spec The SDL_AudioSpec to be initialized by this function.
 * \param iscapture non-zero to query the default recording device, zero to
 *                  query the default output device.
 * \returns 0 on success, nonzero on error
 *
 * \since This function is available since SDL 2.24.0.
 *
 * \sa SDL_GetAudioDeviceName
 * \sa SDL_GetAudioDeviceSpec
 * \sa SDL_OpenAudioDevice
 */
OVERRIDE int SDL_GetDefaultAudioInfo(char **name, SDL_AudioSpec *spec, int iscapture);

/**
 *  Open a specific audio device. Passing in a device name of NULL requests
 *  the most reasonable default (and is equivalent to calling SDL_OpenAudio()).
 *
 *  The device name is a UTF-8 string reported by SDL_GetAudioDeviceName(), but
 *  some drivers allow arbitrary and driver-specific strings, such as a
 *  hostname/IP address for a remote audio server, or a filename in the
 *  diskaudio driver.
 *
 *  \return 0 on error, a valid device ID that is >= 2 on success.
 *
 *  SDL_OpenAudio(), unlike this function, always acts on device ID 1.
 */
OVERRIDE SDL_AudioDeviceID SDL_OpenAudioDevice(const char
                                                              *device,
                                                              int iscapture,
                                                              const
                                                              SDL_AudioSpec *
                                                              desired,
                                                              SDL_AudioSpec *
                                                              obtained,
                                                              int
                                                              allowed_changes);




OVERRIDE SDL_AudioStatus SDL_GetAudioStatus(void);

OVERRIDE SDL_AudioStatus SDL_GetAudioDeviceStatus(SDL_AudioDeviceID dev);
/* @} *//* Audio State */

/**
 *  \name Pause audio functions
 *
 *  These functions pause and unpause the audio callback processing.
 *  They should be called with a parameter of 0 after opening the audio
 *  device to start playing sound.  This is so you can safely initialize
 *  data for your callback function after opening the audio device.
 *  Silence will be written to the audio device during the pause.
 */
/* @{ */
OVERRIDE void SDL_PauseAudio(int pause_on);
OVERRIDE void SDL_PauseAudioDevice(SDL_AudioDeviceID dev,
                                                  int pause_on);
/* @} *//* Pause audio functions */

/**
 *  This function loads a WAVE from the data source, automatically freeing
 *  that source if \c freesrc is non-zero.  For example, to load a WAVE file,
 *  you could do:
 *  \code
 *      SDL_LoadWAV_RW(SDL_RWFromFile("sample.wav", "rb"), 1, ...);
 *  \endcode
 *
 *  If this function succeeds, it returns the given SDL_AudioSpec,
 *  filled with the audio data format of the wave data, and sets
 *  \c *audio_buf to a malloc()'d buffer containing the audio data,
 *  and sets \c *audio_len to the length of that audio buffer, in bytes.
 *  You need to free the audio buffer with SDL_FreeWAV() when you are
 *  done with it.
 *
 *  This function returns NULL and sets the SDL error message if the
 *  wave file cannot be opened, uses an unknown data format, or is
 *  corrupt.  Currently raw and MS-ADPCM WAVE files are supported.
 */
/*OVERRIDE SDL_AudioSpec *SDL_LoadWAV_RW(SDL_RWops * src,
                                                      int freesrc,
                                                      SDL_AudioSpec * spec,
                                                      Uint8 ** audio_buf,
                                                      Uint32 * audio_len);
*/

/**
 *  This function frees data previously allocated with SDL_LoadWAV_RW()
 */
//OVERRIDE void SDL_FreeWAV(Uint8 * audio_buf);

/**
 *  This function takes a source format and rate and a destination format
 *  and rate, and initializes the \c cvt structure with information needed
 *  by SDL_ConvertAudio() to convert a buffer of audio data from one format
 *  to the other.
 *
 *  \return -1 if the format conversion is not supported, 0 if there's
 *  no conversion needed, or 1 if the audio filter is set up.
 */
/*OVERRIDE int SDL_BuildAudioCVT(SDL_AudioCVT * cvt,
                                              SDL_AudioFormat src_format,
                                              Uint8 src_channels,
                                              int src_rate,
                                              SDL_AudioFormat dst_format,
                                              Uint8 dst_channels,
                                              int dst_rate);
*/
/**
 *  Once you have initialized the \c cvt structure using SDL_BuildAudioCVT(),
 *  created an audio buffer \c cvt->buf, and filled it with \c cvt->len bytes of
 *  audio data in the source format, this function will convert it in-place
 *  to the desired format.
 *
 *  The data conversion may expand the size of the audio data, so the buffer
 *  \c cvt->buf should be allocated after the \c cvt structure is initialized by
 *  SDL_BuildAudioCVT(), and should be \c cvt->len*cvt->len_mult bytes long.
 */
//OVERRIDE int SDL_ConvertAudio(SDL_AudioCVT * cvt);

/**
 *  This takes two audio buffers of the playing audio format and mixes
 *  them, performing addition, volume adjustment, and overflow clipping.
 *  The volume ranges from 0 - 128, and should be set to ::SDL_MIX_MAXVOLUME
 *  for full audio volume.  Note this does not change hardware volume.
 *  This is provided for convenience -- you can mix your own audio data.
 */
OVERRIDE void SDL_MixAudio(Uint8 * dst, const Uint8 * src,
                                          Uint32 len, int volume);

/**
 *  This works like SDL_MixAudio(), but you specify the audio format instead of
 *  using the format of audio device 1. Thus it can be used when no audio
 *  device is open at all.
 */
/*OVERRIDE void SDL_MixAudioFormat(Uint8 * dst,
                                                const Uint8 * src,
                                                SDL_AudioFormat format,
                                                Uint32 len, int volume);
*/
/**
 *  Queue more audio on non-callback devices.
 *
 *  SDL offers two ways to feed audio to the device: you can either supply a
 *  callback that SDL triggers with some frequency to obtain more audio
 *  (pull method), or you can supply no callback, and then SDL will expect
 *  you to supply data at regular intervals (push method) with this function.
 *
 *  There are no limits on the amount of data you can queue, short of
 *  exhaustion of address space. Queued data will drain to the device as
 *  necessary without further intervention from you. If the device needs
 *  audio but there is not enough queued, it will play silence to make up
 *  the difference. This means you will have skips in your audio playback
 *  if you aren't routinely queueing sufficient data.
 *
 *  This function copies the supplied data, so you are safe to free it when
 *  the function returns. This function is thread-safe, but queueing to the
 *  same device from two threads at once does not promise which buffer will
 *  be queued first.
 *
 *  You may not queue audio on a device that is using an application-supplied
 *  callback; doing so returns an error. You have to use the audio callback
 *  or queue audio with this function, but not both.
 *
 *  You should not call SDL_LockAudio() on the device before queueing; SDL
 *  handles locking internally for this function.
 *
 *  \param dev The device ID to which we will queue audio.
 *  \param data The data to queue to the device for later playback.
 *  \param len The number of bytes (not samples!) to which (data) points.
 *  \return zero on success, -1 on error.
 *
 *  \sa SDL_GetQueuedAudioSize
 *  \sa SDL_ClearQueuedAudio
 */
OVERRIDE int SDL_QueueAudio(SDL_AudioDeviceID dev, const void *data, Uint32 len);

/**
 *  Get the number of bytes of still-queued audio.
 *
 *  This is the number of bytes that have been queued for playback with
 *  SDL_QueueAudio(), but have not yet been sent to the hardware.
 *
 *  Once we've sent it to the hardware, this function can not decide the exact
 *  byte boundary of what has been played. It's possible that we just gave the
 *  hardware several kilobytes right before you called this function, but it
 *  hasn't played any of it yet, or maybe half of it, etc.
 *
 *  You may not queue audio on a device that is using an application-supplied
 *  callback; calling this function on such a device always returns 0.
 *  You have to use the audio callback or queue audio with SDL_QueueAudio(),
 *  but not both.
 *
 *  You should not call SDL_LockAudio() on the device before querying; SDL
 *  handles locking internally for this function.
 *
 *  \param dev The device ID of which we will query queued audio size.
 *  \return Number of bytes (not samples!) of queued audio.
 *
 *  \sa SDL_QueueAudio
 *  \sa SDL_ClearQueuedAudio
 */
OVERRIDE Uint32 SDL_GetQueuedAudioSize(SDL_AudioDeviceID dev);

/**
 *  Drop any queued audio data waiting to be sent to the hardware.
 *
 *  Immediately after this call, SDL_GetQueuedAudioSize() will return 0 and
 *  the hardware will start playing silence if more audio isn't queued.
 *
 *  This will not prevent playback of queued audio that's already been sent
 *  to the hardware, as we can not undo that, so expect there to be some
 *  fraction of a second of audio that might still be heard. This can be
 *  useful if you want to, say, drop any pending music during a level change
 *  in your game.
 *
 *  You may not queue audio on a device that is using an application-supplied
 *  callback; calling this function on such a device is always a no-op.
 *  You have to use the audio callback or queue audio with SDL_QueueAudio(),
 *  but not both.
 *
 *  You should not call SDL_LockAudio() on the device before clearing the
 *  queue; SDL handles locking internally for this function.
 *
 *  This function always succeeds and thus returns void.
 *
 *  \param dev The device ID of which to clear the audio queue.
 *
 *  \sa SDL_QueueAudio
 *  \sa SDL_GetQueuedAudioSize
 */
OVERRIDE void SDL_ClearQueuedAudio(SDL_AudioDeviceID dev);


/**
 *  \name Audio lock functions
 *
 *  The lock manipulated by these functions protects the callback function.
 *  During a SDL_LockAudio()/SDL_UnlockAudio() pair, you can be guaranteed that
 *  the callback function is not running.  Do not call these from the callback
 *  function or you will cause deadlock.
 */
/* @{ */
OVERRIDE void SDL_LockAudio(void);
OVERRIDE void SDL_LockAudioDevice(SDL_AudioDeviceID dev);
OVERRIDE void SDL_UnlockAudio(void);
OVERRIDE void SDL_UnlockAudioDevice(SDL_AudioDeviceID dev);
/* @} *//* Audio lock functions */

/**
 *  This function shuts down audio processing and closes the audio device.
 */
OVERRIDE void SDL_CloseAudio(void);
OVERRIDE void SDL_CloseAudioDevice(SDL_AudioDeviceID dev);

}

#endif
