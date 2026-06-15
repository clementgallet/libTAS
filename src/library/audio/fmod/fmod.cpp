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

#include "fmod.h"

#include "logging.h"
#include "hook.h"

namespace libtas {

DEFINE_ORIG_POINTER(FMOD_System_Create)
DEFINE_ORIG_POINTER(FMOD_EventSystem_Create)
DEFINE_ORIG_POINTER(FMOD_EventSystem_GetSystemObject)
DEFINE_ORIG_POINTER(FMOD_System_SetOutput)
DEFINE_ORIG_POINTER(_ZN4FMOD6System9setOutputE15FMOD_OUTPUTTYPE)

/* Override */ FMOD_RESULT FMOD_System_Create(void **system, int version)
{
    LOGTRACE_SIMPLE(LCF_SOUND);

    LINK_NAMESPACE(FMOD_System_Create, "fmod");
    LINK_NAMESPACE(FMOD_System_SetOutput, "fmod");

    FMOD_RESULT ret = orig::FMOD_System_Create(system, version);

    /* We force the output to be ALSA */
    orig::FMOD_System_SetOutput(*system, FMOD_OUTPUTTYPE_ALSA);

    return ret;
}

/* Override */ FMOD_RESULT FMOD_EventSystem_Create(void **eventsystem)
{
    LOGTRACE_SIMPLE(LCF_SOUND);

    LINK_NAMESPACE(FMOD_EventSystem_Create, "fmod");
    LINK_NAMESPACE(FMOD_EventSystem_GetSystemObject, "fmod");
    LINK_NAMESPACE(FMOD_System_SetOutput, "fmod");

    FMOD_RESULT ret = orig::FMOD_EventSystem_Create(eventsystem);

    /* We force the output to be ALSA */
    void* system;
    orig::FMOD_EventSystem_GetSystemObject(*eventsystem, &system);
    orig::FMOD_System_SetOutput(system, FMOD_OUTPUTTYPE_ALSA);

    return ret;
}

/* Override */ FMOD_RESULT FMOD_System_SetOutput(void *system, int)
{
    LOGTRACE_SIMPLE(LCF_SOUND);

    LINK_NAMESPACE(FMOD_System_SetOutput, "fmod");

    /* We force the output to be ALSA */
    return orig::FMOD_System_SetOutput(system, FMOD_OUTPUTTYPE_ALSA);
}

/* Override */ FMOD_RESULT _ZN4FMOD6System9setOutputE15FMOD_OUTPUTTYPE(void *system, int output)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::setOutput call with output %d", output);
    LINK_NAMESPACE(_ZN4FMOD6System9setOutputE15FMOD_OUTPUTTYPE, "fmod");

    /* We force the output to be ALSA */
    return orig::_ZN4FMOD6System9setOutputE15FMOD_OUTPUTTYPE(system, FMOD_OUTPUTTYPE_ALSA);
}

/* Override */ FMOD_RESULT _ZN4FMOD6System5closeEv(void *system)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::close call");
    RETURN_NATIVE(_ZN4FMOD6System5closeEv, (system), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD6System4initEijPv(void *system, int maxchannels, FMOD_INITFLAGS flags, void *extradriverdata)
{
    LOGTRACE(LCF_SOUND, "%s call with maxchannels %d, flags %x", __func__, maxchannels, flags);
    RETURN_NATIVE(_ZN4FMOD6System4initEijPv, (system, maxchannels, flags | FMOD_INIT_STREAM_FROM_UPDATE, extradriverdata), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD6System6updateEv(void *system)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::update call");
    RETURN_NATIVE(_ZN4FMOD6System6updateEv, (system), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD6System11createSoundEPKcjP22FMOD_CREATESOUNDEXINFOPPNS_5SoundE(void *system, const char *name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO *exinfo, FMOD_SOUND **sound)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::createSound call with file %s and mode %x", name_or_data, mode);
    RETURN_NATIVE(_ZN4FMOD6System11createSoundEPKcjP22FMOD_CREATESOUNDEXINFOPPNS_5SoundE, (system, name_or_data, mode, exinfo, sound), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD6System13getNumDriversEPi(void *system, int *numdrivers)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::getNumDrivers call");
    RETURN_NATIVE(_ZN4FMOD6System13getNumDriversEPi, (system, numdrivers), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD6System7releaseEv(void *system)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::release call");
    RETURN_NATIVE(_ZN4FMOD6System7releaseEv, (system), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD6System17setSoftwareFormatEi16FMOD_SPEAKERMODEi(void *system, int samplerate, FMOD_SPEAKERMODE speakermode, int numrawspeakers)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::setSoftwareFormat call with samplerate %d, speakermode %x, numrawspeakers %d", samplerate, speakermode, numrawspeakers);
    RETURN_NATIVE(_ZN4FMOD6System17setSoftwareFormatEi16FMOD_SPEAKERMODEi, (system, samplerate, speakermode, numrawspeakers), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD6System9playSoundEPNS_5SoundEPNS_12ChannelGroupEbPPNS_7ChannelE(void *system, FMOD_SOUND* sound, FMOD_CHANNELGROUP* channelgroup, bool paused, FMOD_CHANNEL** channel)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::playSound call with sound %p, channelgroup %p, paused %d", sound, channelgroup, paused);
    RETURN_NATIVE(_ZN4FMOD6System9playSoundEPNS_5SoundEPNS_12ChannelGroupEbPPNS_7ChannelE, (system, sound, channelgroup, paused, channel), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD6System12createStreamEPKcjP22FMOD_CREATESOUNDEXINFOPPNS_5SoundE(void *system, const char *name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO *exinfo, FMOD_SOUND **sound)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::createStream call with file %s and mode %x", name_or_data, mode);
    RETURN_NATIVE(_ZN4FMOD6System12createStreamEPKcjP22FMOD_CREATESOUNDEXINFOPPNS_5SoundE, (system, name_or_data, mode, exinfo, sound), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD6System15createDSPByTypeE13FMOD_DSP_TYPEPPNS_3DSPE(void *system, FMOD_DSP_TYPE type, FMOD_DSP** dsp)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::createDSPByType call with type %x", type);
    RETURN_NATIVE(_ZN4FMOD6System15createDSPByTypeE13FMOD_DSP_TYPEPPNS_3DSPE, (system, type, dsp), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD6System13getDriverInfoEiPciP9FMOD_GUIDPiP16FMOD_SPEAKERMODES4_(void *system, int id, char* name, int name_len, FMOD_GUID* guid, int* systemrate, FMOD_SPEAKERMODE* speakermode, int* speakermodechannels)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::getDriverInfo call with id %d", id);
    RETURN_NATIVE(_ZN4FMOD6System13getDriverInfoEiPciP9FMOD_GUIDPiP16FMOD_SPEAKERMODES4_, (system, id, name, name_len, guid, systemrate, speakermode, speakermodechannels), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD6System9setDriverEi(void *system, int driver)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::setDriver call with driver %d", driver);
    RETURN_NATIVE(_ZN4FMOD6System9setDriverEi, (system, driver), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD6System16setDSPBufferSizeEji(void *system, unsigned int bufferlength, int numbuffers)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::setDSPBufferSize call with bufferlength %d and numbuffers %d", bufferlength, numbuffers);
    RETURN_NATIVE(_ZN4FMOD6System16setDSPBufferSizeEji, (system, bufferlength, numbuffers), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD6System10getVersionEPj(void *system, unsigned int* version)
{
    LOGTRACE(LCF_SOUND, "FMOD::System::getVersion call");
    RETURN_NATIVE(_ZN4FMOD6System10getVersionEPj, (system, version), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD5Sound19getMusicNumChannelsEPi(void *sound, int* numchannels)
{
    LOGTRACE(LCF_SOUND, "FMOD::Sound::getMusicNumChannels call");
    RETURN_NATIVE(_ZN4FMOD5Sound19getMusicNumChannelsEPi, (sound, numchannels), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD5Sound7releaseEv(void *sound)
{
    LOGTRACE(LCF_SOUND, "FMOD::Sound::release call");
    RETURN_NATIVE(_ZN4FMOD5Sound7releaseEv, (sound), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD5Sound13setLoopPointsEjjjj(void *sound, unsigned int loopstart, FMOD_TIMEUNIT loopstarttype, unsigned int loopend, FMOD_TIMEUNIT loopendtype)
{
    LOGTRACE(LCF_SOUND, "FMOD::Sound::setLoopPoints call with loopstart %d, loopstarttype %x, loopend %d, loopendtype %x", loopstart, loopstarttype, loopend, loopendtype);
    RETURN_NATIVE(_ZN4FMOD5Sound13setLoopPointsEjjjj, (sound, loopstart, loopstarttype, loopend, loopendtype), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD7Channel11setPositionEjj(void *channel, unsigned int position, FMOD_TIMEUNIT postype)
{
    LOGTRACE(LCF_SOUND, "FMOD::Channel::setPosition call with position %d, postype %x", position, postype);
    RETURN_NATIVE(_ZN4FMOD7Channel11setPositionEjj, (channel, position, postype), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD7Channel11getPositionEPjj(void *channel, unsigned int *position, FMOD_TIMEUNIT postype)
{
    LOGTRACE(LCF_SOUND, "FMOD::Channel::getPosition call with position %p, postype %x", position, postype);
    RETURN_NATIVE(_ZN4FMOD7Channel11getPositionEPjj, (channel, position, postype), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD7Channel12setFrequencyEf(void *channel, float frequency)
{
    LOGTRACE(LCF_SOUND, "FMOD::Channel::setFrequency call with frequency %f", frequency);
    RETURN_NATIVE(_ZN4FMOD7Channel12setFrequencyEf, (channel, frequency), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD7Channel12setLoopCountEi(void *channel, int loopcount)
{
    LOGTRACE(LCF_SOUND, "FMOD::Channel::setLoopCount call with loopcount %d", loopcount);
    RETURN_NATIVE(_ZN4FMOD7Channel12setLoopCountEi, (channel, loopcount), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD7Channel12getFrequencyEPf(void *channel, float* frequency)
{
    LOGTRACE(LCF_SOUND, "FMOD::Channel::getFrequency call with frequency %p", frequency);
    RETURN_NATIVE(_ZN4FMOD7Channel12getFrequencyEPf, (channel, frequency), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD14ChannelControl9setPausedEb(void *channel_control, bool paused)
{
    LOGTRACE(LCF_SOUND, "FMOD::ChannelControl::setPaused call with paused %d", paused);
    RETURN_NATIVE(_ZN4FMOD14ChannelControl9setPausedEb, (channel_control, paused), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD14ChannelControl4stopEv(void *channel_control)
{
    LOGTRACE(LCF_SOUND, "FMOD::ChannelControl::stop call");
    RETURN_NATIVE(_ZN4FMOD14ChannelControl4stopEv, (channel_control), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD14ChannelControl9setVolumeEf(void *channel_control, float volume)
{
    LOGTRACE(LCF_SOUND, "FMOD::ChannelControl::setVolume call with volume %f", volume);
    RETURN_NATIVE(_ZN4FMOD14ChannelControl9setVolumeEf, (channel_control, volume), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD14ChannelControl9isPlayingEPb(void *channel_control, bool* isplaying)
{
    LOGTRACE(LCF_SOUND, "FMOD::ChannelControl::isPlaying call with isplaying %p", isplaying);
    RETURN_NATIVE(_ZN4FMOD14ChannelControl9isPlayingEPb, (channel_control, isplaying), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD14ChannelControl6addDSPEiPNS_3DSPE(void *channel_control, int index, FMOD_DSP* dsp)
{
    LOGTRACE(LCF_SOUND, "FMOD::ChannelControl::addDSP call with index %d, dsp %p", index, dsp);
    RETURN_NATIVE(_ZN4FMOD14ChannelControl6addDSPEiPNS_3DSPE, (channel_control, index, dsp), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD14ChannelControl9removeDSPEPNS_3DSPE(void *channel_control, FMOD_DSP* dsp)
{
    LOGTRACE(LCF_SOUND, "FMOD::ChannelControl::removeDSP call with dsp %p", dsp);
    RETURN_NATIVE(_ZN4FMOD14ChannelControl9removeDSPEPNS_3DSPE, (channel_control, dsp), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD14ChannelControl7setModeEj(void *channel_control, FMOD_MODE mode)
{
    LOGTRACE(LCF_SOUND, "FMOD::ChannelControl::setMode call with mode %x", mode);
    RETURN_NATIVE(_ZN4FMOD14ChannelControl7setModeEj, (channel_control, mode), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD14ChannelControl6setPanEf(void *channel_control, float pan)
{
    LOGTRACE(LCF_SOUND, "FMOD::ChannelControl::setPan call with pan %f", pan);
    RETURN_NATIVE(_ZN4FMOD14ChannelControl6setPanEf, (channel_control, pan), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD3DSP16getParameterDataEiPPvPjPci(void *dsp, int index, void** data, unsigned int* length, char* valuestr, int valuestrlen)
{
    LOGTRACE(LCF_SOUND, "FMOD::DSP::getParameterData call with index %d, data %p, length %p, valuestr %p, valuestrlen %d", index, data, length, valuestr, valuestrlen);
    RETURN_NATIVE(_ZN4FMOD3DSP16getParameterDataEiPPvPjPci, (dsp, index, data, length, valuestr, valuestrlen), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD3DSP15setParameterIntEii(void *dsp, int index, int value)
{
    LOGTRACE(LCF_SOUND, "FMOD::DSP::setParameterInt call with index %d, value %d", index, value);
    RETURN_NATIVE(_ZN4FMOD3DSP15setParameterIntEii, (dsp, index, value), "fmod");
}

/* Override */ FMOD_RESULT _ZN4FMOD3DSP7releaseEv(void *dsp)
{
    LOGTRACE(LCF_SOUND, "FMOD::DSP::release call");
    RETURN_NATIVE(_ZN4FMOD3DSP7releaseEv, (dsp), "fmod");
}

}
