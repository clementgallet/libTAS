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

#ifndef LIBTAS_SDLAUDIO_H_INCL
#define LIBTAS_SDLAUDIO_H_INCL

#include "hook.h"

#include "../external/SDL2.h"
#include "../external/SDL3.h"

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
 * Get a list of currently-connected audio playback devices.
 *
 * This returns of list of available devices that play sound, perhaps to
 * speakers or headphones ("playback" devices). If you want devices that
 * record audio, like a microphone ("recording" devices), use
 * SDL_GetAudioRecordingDevices() instead.
 *
 * This only returns a list of physical devices; it will not have any device
 * IDs returned by SDL_OpenAudioDevice().
 *
 * If this function returns NULL, to signify an error, `*count` will be set to
 * zero.
 *
 * \param count a pointer filled in with the number of devices returned, may
 *              be NULL.
 * \returns a 0 terminated array of device instance IDs or NULL on error; call
 *          SDL_GetError() for more information. This should be freed with
 *          SDL_free() when it is no longer needed.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_OpenAudioDevice
 * \sa SDL_GetAudioRecordingDevices
 */
OVERRIDE sdl3::SDL_AudioDeviceID * SDL_GetAudioPlaybackDevices(int *count);

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
OVERRIDE int SDL_OpenAudio(sdl2::SDL_AudioSpec * desired,
                                          sdl2::SDL_AudioSpec * obtained);

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
const char *sdl2::SDL_GetAudioDeviceName(int index, int iscapture);

/**
 * Get the human-readable name of a specific audio device.
 *
 * \param devid the instance ID of the device to query.
 * \returns the name of the audio device, or NULL on failure; call
 *          SDL_GetError() for more information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetAudioPlaybackDevices
 * \sa SDL_GetAudioRecordingDevices
 */
const char * sdl3::SDL_GetAudioDeviceName(SDL_AudioDeviceID devid);

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

OVERRIDE int SDL_GetAudioDeviceSpec(int index, int iscapture, sdl2::SDL_AudioSpec *spec);

/**
 * Get the current audio format of a specific audio device.
 *
 * For an opened device, this will report the format the device is currently
 * using. If the device isn't yet opened, this will report the device's
 * preferred format (or a reasonable default if this can't be determined).
 *
 * You may also specify SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK or
 * SDL_AUDIO_DEVICE_DEFAULT_RECORDING here, which is useful for getting a
 * reasonable recommendation before opening the system-recommended default
 * device.
 *
 * You can also use this to request the current device buffer size. This is
 * specified in sample frames and represents the amount of data SDL will feed
 * to the physical hardware in each chunk. This can be converted to
 * milliseconds of audio with the following equation:
 *
 * `ms = (int) ((((Sint64) frames) * 1000) / spec.freq);`
 *
 * Buffer size is only important if you need low-level control over the audio
 * playback timing. Most apps do not need this.
 *
 * \param devid the instance ID of the device to query.
 * \param spec on return, will be filled with device details.
 * \param sample_frames pointer to store device buffer size, in sample frames.
 *                      Can be NULL.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE bool SDL_GetAudioDeviceFormat(sdl3::SDL_AudioDeviceID devid, sdl3::SDL_AudioSpec *spec, int *sample_frames);

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
OVERRIDE int SDL_GetDefaultAudioInfo(char **name, sdl2::SDL_AudioSpec *spec, int iscapture);

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
OVERRIDE sdl2::SDL_AudioDeviceID SDL_OpenAudioDevice(const char
                                                              *device,
                                                              int iscapture,
                                                              const
                                                              sdl2::SDL_AudioSpec *
                                                              desired,
                                                              sdl2::SDL_AudioSpec *
                                                              obtained,
                                                              int
                                                              allowed_changes);

/**
 * Open a specific audio device.
 *
 * You can open both playback and recording devices through this function.
 * Playback devices will take data from bound audio streams, mix it, and send
 * it to the hardware. Recording devices will feed any bound audio streams
 * with a copy of any incoming data.
 *
 * An opened audio device starts out with no audio streams bound. To start
 * audio playing, bind a stream and supply audio data to it. Unlike SDL2,
 * there is no audio callback; you only bind audio streams and make sure they
 * have data flowing into them (however, you can simulate SDL2's semantics
 * fairly closely by using SDL_OpenAudioDeviceStream instead of this
 * function).
 *
 * If you don't care about opening a specific device, pass a `devid` of either
 * `SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK` or
 * `SDL_AUDIO_DEVICE_DEFAULT_RECORDING`. In this case, SDL will try to pick
 * the most reasonable default, and may also switch between physical devices
 * seamlessly later, if the most reasonable default changes during the
 * lifetime of this opened device (user changed the default in the OS's system
 * preferences, the default got unplugged so the system jumped to a new
 * default, the user plugged in headphones on a mobile device, etc). Unless
 * you have a good reason to choose a specific device, this is probably what
 * you want.
 *
 * You may request a specific format for the audio device, but there is no
 * promise the device will honor that request for several reasons. As such,
 * it's only meant to be a hint as to what data your app will provide. Audio
 * streams will accept data in whatever format you specify and manage
 * conversion for you as appropriate. SDL_GetAudioDeviceFormat can tell you
 * the preferred format for the device before opening and the actual format
 * the device is using after opening.
 *
 * It's legal to open the same device ID more than once; each successful open
 * will generate a new logical SDL_AudioDeviceID that is managed separately
 * from others on the same physical device. This allows libraries to open a
 * device separately from the main app and bind its own streams without
 * conflicting.
 *
 * It is also legal to open a device ID returned by a previous call to this
 * function; doing so just creates another logical device on the same physical
 * device. This may be useful for making logical groupings of audio streams.
 *
 * This function returns the opened device ID on success. This is a new,
 * unique SDL_AudioDeviceID that represents a logical device.
 *
 * Some backends might offer arbitrary devices (for example, a networked audio
 * protocol that can connect to an arbitrary server). For these, as a change
 * from SDL2, you should open a default device ID and use an SDL hint to
 * specify the target if you care, or otherwise let the backend figure out a
 * reasonable default. Most backends don't offer anything like this, and often
 * this would be an end user setting an environment variable for their custom
 * need, and not something an application should specifically manage.
 *
 * When done with an audio device, possibly at the end of the app's life, one
 * should call SDL_CloseAudioDevice() on the returned device id.
 *
 * \param devid the device instance id to open, or
 *              SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK or
 *              SDL_AUDIO_DEVICE_DEFAULT_RECORDING for the most reasonable
 *              default device.
 * \param spec the requested device configuration. Can be NULL to use
 *             reasonable defaults.
 * \returns the device ID on success or 0 on failure; call SDL_GetError() for
 *          more information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_CloseAudioDevice
 * \sa SDL_GetAudioDeviceFormat
 */
sdl3::SDL_AudioDeviceID sdl3::SDL_OpenAudioDevice(SDL_AudioDeviceID devid, const SDL_AudioSpec *spec);

OVERRIDE sdl2::SDL_AudioStatus SDL_GetAudioStatus(void);

OVERRIDE sdl2::SDL_AudioStatus SDL_GetAudioDeviceStatus(sdl2::SDL_AudioDeviceID dev);
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
void sdl2::SDL_PauseAudioDevice(sdl2::SDL_AudioDeviceID dev, int pause_on);
/* @} *//* Pause audio functions */

/**
 * Use this function to pause audio playback on a specified device.
 *
 * This function pauses audio processing for a given device. Any bound audio
 * streams will not progress, and no audio will be generated. Pausing one
 * device does not prevent other unpaused devices from running.
 *
 * Unlike in SDL2, audio devices start in an _unpaused_ state, since an app
 * has to bind a stream before any audio will flow. Pausing a paused device is
 * a legal no-op.
 *
 * Pausing a device can be useful to halt all audio without unbinding all the
 * audio streams. This might be useful while a game is paused, or a level is
 * loading, etc.
 *
 * Physical devices can not be paused or unpaused, only logical devices
 * created through SDL_OpenAudioDevice() can be.
 *
 * \param devid a device opened by SDL_OpenAudioDevice().
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_ResumeAudioDevice
 * \sa SDL_AudioDevicePaused
 */
bool sdl3::SDL_PauseAudioDevice(SDL_AudioDeviceID devid);

/**
 * Use this function to unpause audio playback on a specified device.
 *
 * This function unpauses audio processing for a given device that has
 * previously been paused with SDL_PauseAudioDevice(). Once unpaused, any
 * bound audio streams will begin to progress again, and audio can be
 * generated.
 *
 * Unlike in SDL2, audio devices start in an _unpaused_ state, since an app
 * has to bind a stream before any audio will flow. Unpausing an unpaused
 * device is a legal no-op.
 *
 * Physical devices can not be paused or unpaused, only logical devices
 * created through SDL_OpenAudioDevice() can be.
 *
 * \param devid a device opened by SDL_OpenAudioDevice().
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_AudioDevicePaused
 * \sa SDL_PauseAudioDevice
 */
OVERRIDE bool SDL_ResumeAudioDevice(sdl3::SDL_AudioDeviceID devid);

/**
 * Use this function to query if an audio device is paused.
 *
 * Unlike in SDL2, audio devices start in an _unpaused_ state, since an app
 * has to bind a stream before any audio will flow.
 *
 * Physical devices can not be paused or unpaused, only logical devices
 * created through SDL_OpenAudioDevice() can be. Physical and invalid device
 * IDs will report themselves as unpaused here.
 *
 * \param devid a device opened by SDL_OpenAudioDevice().
 * \returns true if device is valid and paused, false otherwise.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_PauseAudioDevice
 * \sa SDL_ResumeAudioDevice
 */
OVERRIDE bool SDL_AudioDevicePaused(sdl3::SDL_AudioDeviceID devid);

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
 * Get the gain of an audio device.
 *
 * The gain of a device is its volume; a larger gain means a louder output,
 * with a gain of zero being silence.
 *
 * Audio devices default to a gain of 1.0f (no change in output).
 *
 * Physical devices may not have their gain changed, only logical devices, and
 * this function will always return -1.0f when used on physical devices.
 *
 * \param devid the audio device to query.
 * \returns the gain of the device or -1.0f on failure; call SDL_GetError()
 *          for more information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetAudioDeviceGain
 */
OVERRIDE float SDL_GetAudioDeviceGain(sdl3::SDL_AudioDeviceID devid);

/**
 * Change the gain of an audio device.
 *
 * The gain of a device is its volume; a larger gain means a louder output,
 * with a gain of zero being silence.
 *
 * Audio devices default to a gain of 1.0f (no change in output).
 *
 * Physical devices may not have their gain changed, only logical devices, and
 * this function will always return false when used on physical devices. While
 * it might seem attractive to adjust several logical devices at once in this
 * way, it would allow an app or library to interfere with another portion of
 * the program's otherwise-isolated devices.
 *
 * This is applied, along with any per-audiostream gain, during playback to
 * the hardware, and can be continuously changed to create various effects. On
 * recording devices, this will adjust the gain before passing the data into
 * an audiostream; that recording audiostream can then adjust its gain further
 * when outputting the data elsewhere, if it likes, but that second gain is
 * not applied until the data leaves the audiostream again.
 *
 * \param devid the audio device on which to change gain.
 * \param gain the gain. 1.0f is no change, 0.0f is silence.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread, as it holds
 *               a stream-specific mutex while running.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetAudioDeviceGain
 */
OVERRIDE bool SDL_SetAudioDeviceGain(sdl3::SDL_AudioDeviceID devid, float gain);

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
OVERRIDE int SDL_QueueAudio(sdl2::SDL_AudioDeviceID dev, const void *data, Uint32 len);

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
OVERRIDE Uint32 SDL_GetQueuedAudioSize(sdl2::SDL_AudioDeviceID dev);

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
OVERRIDE void SDL_ClearQueuedAudio(sdl2::SDL_AudioDeviceID dev);

/**
 * Bind a list of audio streams to an audio device.
 *
 * Audio data will flow through any bound streams. For a playback device, data
 * for all bound streams will be mixed together and fed to the device. For a
 * recording device, a copy of recorded data will be provided to each bound
 * stream.
 *
 * Audio streams can only be bound to an open device. This operation is
 * atomic--all streams bound in the same call will start processing at the
 * same time, so they can stay in sync. Also: either all streams will be bound
 * or none of them will be.
 *
 * It is an error to bind an already-bound stream; it must be explicitly
 * unbound first.
 *
 * Binding a stream to a device will set its output format for playback
 * devices, and its input format for recording devices, so they match the
 * device's settings. The caller is welcome to change the other end of the
 * stream's format at any time with SDL_SetAudioStreamFormat().
 *
 * \param devid an audio device to bind a stream to.
 * \param streams an array of audio streams to bind.
 * \param num_streams number streams listed in the `streams` array.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_BindAudioStreams
 * \sa SDL_UnbindAudioStream
 * \sa SDL_GetAudioStreamDevice
 */
OVERRIDE bool SDL_BindAudioStreams(sdl3::SDL_AudioDeviceID devid, SDL_AudioStream * const *streams, int num_streams);

/**
 * Bind a single audio stream to an audio device.
 *
 * This is a convenience function, equivalent to calling
 * `SDL_BindAudioStreams(devid, &stream, 1)`.
 *
 * \param devid an audio device to bind a stream to.
 * \param stream an audio stream to bind to a device.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_BindAudioStreams
 * \sa SDL_UnbindAudioStream
 * \sa SDL_GetAudioStreamDevice
 */
OVERRIDE bool SDL_BindAudioStream(sdl3::SDL_AudioDeviceID devid, SDL_AudioStream *stream);

/**
 * Unbind a list of audio streams from their audio devices.
 *
 * The streams being unbound do not all have to be on the same device. All
 * streams on the same device will be unbound atomically (data will stop
 * flowing through all unbound streams on the same device at the same time).
 *
 * Unbinding a stream that isn't bound to a device is a legal no-op.
 *
 * \param streams an array of audio streams to unbind. Can be NULL or contain
 *                NULL.
 * \param num_streams number streams listed in the `streams` array.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_BindAudioStreams
 */
OVERRIDE void SDL_UnbindAudioStreams(SDL_AudioStream * const *streams, int num_streams);

/**
 * Unbind a single audio stream from its audio device.
 *
 * This is a convenience function, equivalent to calling
 * `SDL_UnbindAudioStreams(&stream, 1)`.
 *
 * \param stream an audio stream to unbind from a device. Can be NULL.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_BindAudioStream
 */
OVERRIDE void SDL_UnbindAudioStream(SDL_AudioStream *stream);

/**
 * Query an audio stream for its currently-bound device.
 *
 * This reports the audio device that an audio stream is currently bound to.
 *
 * If not bound, or invalid, this returns zero, which is not a valid device
 * ID.
 *
 * \param stream the audio stream to query.
 * \returns the bound audio device, or 0 if not bound or invalid.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_BindAudioStream
 * \sa SDL_BindAudioStreams
 */
OVERRIDE sdl3::SDL_AudioDeviceID SDL_GetAudioStreamDevice(SDL_AudioStream *stream);

/**
 * Create a new audio stream.
 *
 * \param src_spec the format details of the input audio.
 * \param dst_spec the format details of the output audio.
 * \returns a new audio stream on success or NULL on failure; call
 *          SDL_GetError() for more information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_PutAudioStreamData
 * \sa SDL_GetAudioStreamData
 * \sa SDL_GetAudioStreamAvailable
 * \sa SDL_FlushAudioStream
 * \sa SDL_ClearAudioStream
 * \sa SDL_SetAudioStreamFormat
 * \sa SDL_DestroyAudioStream
 */
OVERRIDE SDL_AudioStream * SDL_CreateAudioStream(const sdl3::SDL_AudioSpec *src_spec, const sdl3::SDL_AudioSpec *dst_spec);

/**
 * Get the properties associated with an audio stream.
 *
 * \param stream the SDL_AudioStream to query.
 * \returns a valid property ID on success or 0 on failure; call
 *          SDL_GetError() for more information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
// OVERRIDE sdl3::SDL_PropertiesID SDL_GetAudioStreamProperties(SDL_AudioStream *stream);

/**
 * Query the current format of an audio stream.
 *
 * \param stream the SDL_AudioStream to query.
 * \param src_spec where to store the input audio format; ignored if NULL.
 * \param dst_spec where to store the output audio format; ignored if NULL.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread, as it holds
 *               a stream-specific mutex while running.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetAudioStreamFormat
 */
OVERRIDE bool SDL_GetAudioStreamFormat(SDL_AudioStream *stream, sdl3::SDL_AudioSpec *src_spec, sdl3::SDL_AudioSpec *dst_spec);

/**
 * Change the input and output formats of an audio stream.
 *
 * Future calls to and SDL_GetAudioStreamAvailable and SDL_GetAudioStreamData
 * will reflect the new format, and future calls to SDL_PutAudioStreamData
 * must provide data in the new input formats.
 *
 * Data that was previously queued in the stream will still be operated on in
 * the format that was current when it was added, which is to say you can put
 * the end of a sound file in one format to a stream, change formats for the
 * next sound file, and start putting that new data while the previous sound
 * file is still queued, and everything will still play back correctly.
 *
 * If a stream is bound to a device, then the format of the side of the stream
 * bound to a device cannot be changed (src_spec for recording devices,
 * dst_spec for playback devices). Attempts to make a change to this side will
 * be ignored, but this will not report an error. The other side's format can
 * be changed.
 *
 * \param stream the stream the format is being changed.
 * \param src_spec the new format of the audio input; if NULL, it is not
 *                 changed.
 * \param dst_spec the new format of the audio output; if NULL, it is not
 *                 changed.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread, as it holds
 *               a stream-specific mutex while running.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetAudioStreamFormat
 * \sa SDL_SetAudioStreamFrequencyRatio
 */
OVERRIDE bool SDL_SetAudioStreamFormat(SDL_AudioStream *stream, const sdl3::SDL_AudioSpec *src_spec, const sdl3::SDL_AudioSpec *dst_spec);

/**
 * Get the frequency ratio of an audio stream.
 *
 * \param stream the SDL_AudioStream to query.
 * \returns the frequency ratio of the stream or 0.0 on failure; call
 *          SDL_GetError() for more information.
 *
 * \threadsafety It is safe to call this function from any thread, as it holds
 *               a stream-specific mutex while running.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetAudioStreamFrequencyRatio
 */
OVERRIDE float SDL_GetAudioStreamFrequencyRatio(SDL_AudioStream *stream);

/**
 * Change the frequency ratio of an audio stream.
 *
 * The frequency ratio is used to adjust the rate at which input data is
 * consumed. Changing this effectively modifies the speed and pitch of the
 * audio. A value greater than 1.0 will play the audio faster, and at a higher
 * pitch. A value less than 1.0 will play the audio slower, and at a lower
 * pitch.
 *
 * This is applied during SDL_GetAudioStreamData, and can be continuously
 * changed to create various effects.
 *
 * \param stream the stream the frequency ratio is being changed.
 * \param ratio the frequency ratio. 1.0 is normal speed. Must be between 0.01
 *              and 100.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread, as it holds
 *               a stream-specific mutex while running.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetAudioStreamFrequencyRatio
 * \sa SDL_SetAudioStreamFormat
 */
OVERRIDE bool SDL_SetAudioStreamFrequencyRatio(SDL_AudioStream *stream, float ratio);

/**
 * Get the gain of an audio stream.
 *
 * The gain of a stream is its volume; a larger gain means a louder output,
 * with a gain of zero being silence.
 *
 * Audio streams default to a gain of 1.0f (no change in output).
 *
 * \param stream the SDL_AudioStream to query.
 * \returns the gain of the stream or -1.0f on failure; call SDL_GetError()
 *          for more information.
 *
 * \threadsafety It is safe to call this function from any thread, as it holds
 *               a stream-specific mutex while running.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetAudioStreamGain
 */
OVERRIDE float SDL_GetAudioStreamGain(SDL_AudioStream *stream);

/**
 * Change the gain of an audio stream.
 *
 * The gain of a stream is its volume; a larger gain means a louder output,
 * with a gain of zero being silence.
 *
 * Audio streams default to a gain of 1.0f (no change in output).
 *
 * This is applied during SDL_GetAudioStreamData, and can be continuously
 * changed to create various effects.
 *
 * \param stream the stream on which the gain is being changed.
 * \param gain the gain. 1.0f is no change, 0.0f is silence.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread, as it holds
 *               a stream-specific mutex while running.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetAudioStreamGain
 */
OVERRIDE bool SDL_SetAudioStreamGain(SDL_AudioStream *stream, float gain);

/**
 * Get the current input channel map of an audio stream.
 *
 * Channel maps are optional; most things do not need them, instead passing
 * data in the [order that SDL expects](CategoryAudio#channel-layouts).
 *
 * Audio streams default to no remapping applied. This is represented by
 * returning NULL, and does not signify an error.
 *
 * \param stream the SDL_AudioStream to query.
 * \param count On output, set to number of channels in the map. Can be NULL.
 * \returns an array of the current channel mapping, with as many elements as
 *          the current output spec's channels, or NULL if default. This
 *          should be freed with SDL_free() when it is no longer needed.
 *
 * \threadsafety It is safe to call this function from any thread, as it holds
 *               a stream-specific mutex while running.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetAudioStreamInputChannelMap
 */
OVERRIDE int * SDL_GetAudioStreamInputChannelMap(SDL_AudioStream *stream, int *count);

/**
 * Get the current output channel map of an audio stream.
 *
 * Channel maps are optional; most things do not need them, instead passing
 * data in the [order that SDL expects](CategoryAudio#channel-layouts).
 *
 * Audio streams default to no remapping applied. This is represented by
 * returning NULL, and does not signify an error.
 *
 * \param stream the SDL_AudioStream to query.
 * \param count On output, set to number of channels in the map. Can be NULL.
 * \returns an array of the current channel mapping, with as many elements as
 *          the current output spec's channels, or NULL if default. This
 *          should be freed with SDL_free() when it is no longer needed.
 *
 * \threadsafety It is safe to call this function from any thread, as it holds
 *               a stream-specific mutex while running.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetAudioStreamInputChannelMap
 */
OVERRIDE int * SDL_GetAudioStreamOutputChannelMap(SDL_AudioStream *stream, int *count);

/**
 * Set the current input channel map of an audio stream.
 *
 * Channel maps are optional; most things do not need them, instead passing
 * data in the [order that SDL expects](CategoryAudio#channel-layouts).
 *
 * The input channel map reorders data that is added to a stream via
 * SDL_PutAudioStreamData. Future calls to SDL_PutAudioStreamData must provide
 * data in the new channel order.
 *
 * Each item in the array represents an input channel, and its value is the
 * channel that it should be remapped to. To reverse a stereo signal's left
 * and right values, you'd have an array of `{ 1, 0 }`. It is legal to remap
 * multiple channels to the same thing, so `{ 1, 1 }` would duplicate the
 * right channel to both channels of a stereo signal. An element in the
 * channel map set to -1 instead of a valid channel will mute that channel,
 * setting it to a silence value.
 *
 * You cannot change the number of channels through a channel map, just
 * reorder/mute them.
 *
 * Data that was previously queued in the stream will still be operated on in
 * the order that was current when it was added, which is to say you can put
 * the end of a sound file in one order to a stream, change orders for the
 * next sound file, and start putting that new data while the previous sound
 * file is still queued, and everything will still play back correctly.
 *
 * Audio streams default to no remapping applied. Passing a NULL channel map
 * is legal, and turns off remapping.
 *
 * SDL will copy the channel map; the caller does not have to save this array
 * after this call.
 *
 * If `count` is not equal to the current number of channels in the audio
 * stream's format, this will fail. This is a safety measure to make sure a
 * race condition hasn't changed the format while this call is setting the
 * channel map.
 *
 * Unlike attempting to change the stream's format, the input channel map on a
 * stream bound to a recording device is permitted to change at any time; any
 * data added to the stream from the device after this call will have the new
 * mapping, but previously-added data will still have the prior mapping.
 *
 * \param stream the SDL_AudioStream to change.
 * \param chmap the new channel map, NULL to reset to default.
 * \param count The number of channels in the map.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread, as it holds
 *               a stream-specific mutex while running. Don't change the
 *               stream's format to have a different number of channels from a
 *               a different thread at the same time, though!
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetAudioStreamInputChannelMap
 */
OVERRIDE bool SDL_SetAudioStreamInputChannelMap(SDL_AudioStream *stream, const int *chmap, int count);

/**
 * Set the current output channel map of an audio stream.
 *
 * Channel maps are optional; most things do not need them, instead passing
 * data in the [order that SDL expects](CategoryAudio#channel-layouts).
 *
 * The output channel map reorders data that leaving a stream via
 * SDL_GetAudioStreamData.
 *
 * Each item in the array represents an input channel, and its value is the
 * channel that it should be remapped to. To reverse a stereo signal's left
 * and right values, you'd have an array of `{ 1, 0 }`. It is legal to remap
 * multiple channels to the same thing, so `{ 1, 1 }` would duplicate the
 * right channel to both channels of a stereo signal. An element in the
 * channel map set to -1 instead of a valid channel will mute that channel,
 * setting it to a silence value.
 *
 * You cannot change the number of channels through a channel map, just
 * reorder/mute them.
 *
 * The output channel map can be changed at any time, as output remapping is
 * applied during SDL_GetAudioStreamData.
 *
 * Audio streams default to no remapping applied. Passing a NULL channel map
 * is legal, and turns off remapping.
 *
 * SDL will copy the channel map; the caller does not have to save this array
 * after this call.
 *
 * If `count` is not equal to the current number of channels in the audio
 * stream's format, this will fail. This is a safety measure to make sure a
 * race condition hasn't changed the format while this call is setting the
 * channel map.
 *
 * Unlike attempting to change the stream's format, the output channel map on
 * a stream bound to a recording device is permitted to change at any time;
 * any data added to the stream after this call will have the new mapping, but
 * previously-added data will still have the prior mapping. When the channel
 * map doesn't match the hardware's channel layout, SDL will convert the data
 * before feeding it to the device for playback.
 *
 * \param stream the SDL_AudioStream to change.
 * \param chmap the new channel map, NULL to reset to default.
 * \param count The number of channels in the map.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread, as it holds
 *               a stream-specific mutex while running. Don't change the
 *               stream's format to have a different number of channels from a
 *               a different thread at the same time, though!
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetAudioStreamInputChannelMap
 */
OVERRIDE bool SDL_SetAudioStreamOutputChannelMap(SDL_AudioStream *stream, const int *chmap, int count);

/**
 * Add data to the stream.
 *
 * This data must match the format/channels/samplerate specified in the latest
 * call to SDL_SetAudioStreamFormat, or the format specified when creating the
 * stream if it hasn't been changed.
 *
 * Note that this call simply copies the unconverted data for later. This is
 * different than SDL2, where data was converted during the Put call and the
 * Get call would just dequeue the previously-converted data.
 *
 * \param stream the stream the audio data is being added to.
 * \param buf a pointer to the audio data to add.
 * \param len the number of bytes to write to the stream.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread, but if the
 *               stream has a callback set, the caller might need to manage
 *               extra locking.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_ClearAudioStream
 * \sa SDL_FlushAudioStream
 * \sa SDL_GetAudioStreamData
 * \sa SDL_GetAudioStreamQueued
 */
OVERRIDE bool SDL_PutAudioStreamData(SDL_AudioStream *stream, const void *buf, int len);

/**
 * Get converted/resampled data from the stream.
 *
 * The input/output data format/channels/samplerate is specified when creating
 * the stream, and can be changed after creation by calling
 * SDL_SetAudioStreamFormat.
 *
 * Note that any conversion and resampling necessary is done during this call,
 * and SDL_PutAudioStreamData simply queues unconverted data for later. This
 * is different than SDL2, where that work was done while inputting new data
 * to the stream and requesting the output just copied the converted data.
 *
 * \param stream the stream the audio is being requested from.
 * \param buf a buffer to fill with audio data.
 * \param len the maximum number of bytes to fill.
 * \returns the number of bytes read from the stream or -1 on failure; call
 *          SDL_GetError() for more information.
 *
 * \threadsafety It is safe to call this function from any thread, but if the
 *               stream has a callback set, the caller might need to manage
 *               extra locking.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_ClearAudioStream
 * \sa SDL_GetAudioStreamAvailable
 * \sa SDL_PutAudioStreamData
 */
OVERRIDE int SDL_GetAudioStreamData(SDL_AudioStream *stream, void *buf, int len);

/**
 * Get the number of converted/resampled bytes available.
 *
 * The stream may be buffering data behind the scenes until it has enough to
 * resample correctly, so this number might be lower than what you expect, or
 * even be zero. Add more data or flush the stream if you need the data now.
 *
 * If the stream has so much data that it would overflow an int, the return
 * value is clamped to a maximum value, but no queued data is lost; if there
 * are gigabytes of data queued, the app might need to read some of it with
 * SDL_GetAudioStreamData before this function's return value is no longer
 * clamped.
 *
 * \param stream the audio stream to query.
 * \returns the number of converted/resampled bytes available or -1 on
 *          failure; call SDL_GetError() for more information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetAudioStreamData
 * \sa SDL_PutAudioStreamData
 */
OVERRIDE int SDL_GetAudioStreamAvailable(SDL_AudioStream *stream);


/**
 * Get the number of bytes currently queued.
 *
 * This is the number of bytes put into a stream as input, not the number that
 * can be retrieved as output. Because of several details, it's not possible
 * to calculate one number directly from the other. If you need to know how
 * much usable data can be retrieved right now, you should use
 * SDL_GetAudioStreamAvailable() and not this function.
 *
 * Note that audio streams can change their input format at any time, even if
 * there is still data queued in a different format, so the returned byte
 * count will not necessarily match the number of _sample frames_ available.
 * Users of this API should be aware of format changes they make when feeding
 * a stream and plan accordingly.
 *
 * Queued data is not converted until it is consumed by
 * SDL_GetAudioStreamData, so this value should be representative of the exact
 * data that was put into the stream.
 *
 * If the stream has so much data that it would overflow an int, the return
 * value is clamped to a maximum value, but no queued data is lost; if there
 * are gigabytes of data queued, the app might need to read some of it with
 * SDL_GetAudioStreamData before this function's return value is no longer
 * clamped.
 *
 * \param stream the audio stream to query.
 * \returns the number of bytes queued or -1 on failure; call SDL_GetError()
 *          for more information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_PutAudioStreamData
 * \sa SDL_ClearAudioStream
 */
OVERRIDE int SDL_GetAudioStreamQueued(SDL_AudioStream *stream);


/**
 * Tell the stream that you're done sending data, and anything being buffered
 * should be converted/resampled and made available immediately.
 *
 * It is legal to add more data to a stream after flushing, but there may be
 * audio gaps in the output. Generally this is intended to signal the end of
 * input, so the complete output becomes available.
 *
 * \param stream the audio stream to flush.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_PutAudioStreamData
 */
OVERRIDE bool SDL_FlushAudioStream(SDL_AudioStream *stream);

/**
 * Clear any pending data in the stream.
 *
 * This drops any queued data, so there will be nothing to read from the
 * stream until more is added.
 *
 * \param stream the audio stream to clear.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetAudioStreamAvailable
 * \sa SDL_GetAudioStreamData
 * \sa SDL_GetAudioStreamQueued
 * \sa SDL_PutAudioStreamData
 */
OVERRIDE bool SDL_ClearAudioStream(SDL_AudioStream *stream);

/**
 * Use this function to pause audio playback on the audio device associated
 * with an audio stream.
 *
 * This function pauses audio processing for a given device. Any bound audio
 * streams will not progress, and no audio will be generated. Pausing one
 * device does not prevent other unpaused devices from running.
 *
 * Pausing a device can be useful to halt all audio without unbinding all the
 * audio streams. This might be useful while a game is paused, or a level is
 * loading, etc.
 *
 * \param stream the audio stream associated with the audio device to pause.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_ResumeAudioStreamDevice
 */
OVERRIDE bool SDL_PauseAudioStreamDevice(SDL_AudioStream *stream);

/**
 * Use this function to unpause audio playback on the audio device associated
 * with an audio stream.
 *
 * This function unpauses audio processing for a given device that has
 * previously been paused. Once unpaused, any bound audio streams will begin
 * to progress again, and audio can be generated.
 *
 * Remember, SDL_OpenAudioDeviceStream opens device in a paused state, so this
 * function call is required for audio playback to begin on such device.
 *
 * \param stream the audio stream associated with the audio device to resume.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_PauseAudioStreamDevice
 */
OVERRIDE bool SDL_ResumeAudioStreamDevice(SDL_AudioStream *stream);

/**
 * Use this function to query if an audio device associated with a stream is
 * paused.
 *
 * Unlike in SDL2, audio devices start in an _unpaused_ state, since an app
 * has to bind a stream before any audio will flow.
 *
 * \param stream the audio stream associated with the audio device to query.
 * \returns true if device is valid and paused, false otherwise.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_PauseAudioStreamDevice
 * \sa SDL_ResumeAudioStreamDevice
 */
OVERRIDE bool SDL_AudioStreamDevicePaused(SDL_AudioStream *stream);


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
OVERRIDE void SDL_LockAudioDevice(sdl2::SDL_AudioDeviceID dev);
OVERRIDE void SDL_UnlockAudio(void);
OVERRIDE void SDL_UnlockAudioDevice(sdl2::SDL_AudioDeviceID dev);
/* @} *//* Audio lock functions */

/**
 *  This function shuts down audio processing and closes the audio device.
 */
OVERRIDE void SDL_CloseAudio(void);
OVERRIDE void SDL_CloseAudioDevice(sdl2::SDL_AudioDeviceID dev);

}

#endif
