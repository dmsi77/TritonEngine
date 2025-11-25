// sound_context_al.cpp

#include <iostream>
#include <cstdio>
#include <string>
#include <windows.h>
#include "application.hpp"
#include "sound_context.hpp"
#include "sound_manager.hpp"
#include "memory_pool.hpp"
#include "log.hpp"

using namespace types;

namespace realware
{
    sWAVStructure* LoadWAVFile(cMemoryPool* memoryPool, const std::string& filename)
    {
        sWAVStructure* pWav = (sWAVStructure*)memoryPool->Allocate(sizeof(sWAVStructure));
        sWAVStructure* wav = new (pWav) sWAVStructure();

        FILE* fp = nullptr;
        const errno_t err = fopen_s(&fp, &filename.c_str()[0], "rb");
        if (err != 0)
            Print("Error: can't open WAV file at '" + filename + "'!");

        // Chunk
        fread(&wav->_type[0], sizeof(char), 4, fp);
        if (std::string((const char*)&wav->_type[0]) != std::string("RIFF"))
            Print("Error: not a RIFF file!");

        fread(&wav->_chunkSize, sizeof(int), 1, fp);
        fread(&wav->_format[0], sizeof(char), 4, fp);
        if (std::string((const char*)&wav->_format[0]) != std::string("WAVE"))
            Print("Error: not a WAVE file!");

        // 1st Subchunk
        fread(&wav->_subchunk1ID[0], sizeof(char), 4, fp);
        if (std::string((const char*)&wav->_subchunk1ID[0]) != std::string("fmt "))
            Print("Error: missing fmt header!");
        fread(&wav->_subchunk1Size, sizeof(int), 1, fp);
        fread(&wav->_audioFormat, sizeof(short), 1, fp);
        fread(&wav->_numChannels, sizeof(short), 1, fp);
        fread(&wav->_sampleRate, sizeof(int), 1, fp);
        fread(&wav->_byteRate, sizeof(int), 1, fp);
        fread(&wav->_blockAlign, sizeof(short), 1, fp);
        fread(&wav->_bitsPerSample, sizeof(short), 1, fp);

        // 2nd Subchunk
        fread(&wav->_subchunk2ID[0], sizeof(char), 4, fp);
        if (std::string((const char*)&wav->_subchunk2ID[0]) != std::string("data"))
            Print("Error: missing data header!");
        fread(&wav->_subchunk2Size, sizeof(int), 1, fp);

        // Data
        const int NumSamples = wav->_subchunk2Size / (wav->_numChannels * (wav->_bitsPerSample / 8));
        wav->_dataByteSize = NumSamples * (wav->_bitsPerSample / 8) * wav->_numChannels;
        wav->_data = (unsigned short*)memoryPool->Allocate(wav->_dataByteSize);
        if (wav->_bitsPerSample == 16 && wav->_numChannels == 2)
        {
            for (int i = 0; i < NumSamples; i++)
            {
                const int idx = i * 2;
                fread(&wav->_data[idx], sizeof(short), 1, fp);
                fread(&wav->_data[idx + 1], sizeof(short), 1, fp);
            }
        }
        fclose(fp);

        return wav;
    }

    cOpenALSoundContext::cOpenALSoundContext(cContext* context) : iSoundContext(context)
    {
        _device = alcOpenDevice(nullptr);
        _context = alcCreateContext(_device, nullptr);
        alcMakeContextCurrent(_context);
    }

    cOpenALSoundContext::~cOpenALSoundContext()
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(_context);
        alcCloseDevice(_device);
    }

    void cOpenALSoundContext::Create(const std::string& filename, eCategory format, const sWAVStructure** file, types::u32& source, types::u32& buffer)
    {
        if (format == eCategory::SOUND_FORMAT_WAV)
        {
            const sWAVStructure* wavFile = LoadWAVFile(_app->GetMemoryPool(), filename);
            *file = wavFile;

            alGenSources(1, (ALuint*)&source);
            alSourcef(source, AL_PITCH, 1);
            alSourcef(source, AL_GAIN, 1);
            alSource3f(source, AL_POSITION, 0, 0, 0);
            alSource3f(source, AL_VELOCITY, 0, 0, 0);
            alSourcei(source, AL_LOOPING, AL_FALSE);

            alGenBuffers(1, (ALuint*)&buffer);

            ALenum wavFormat = AL_FORMAT_STEREO16;
            bool stereo = (wavFile->_numChannels > 1);
            switch (wavFile->_bitsPerSample) {
                case 16:
                    if (stereo) {
                        wavFormat = AL_FORMAT_STEREO16;
                        break;
                    } else {
                        wavFormat = AL_FORMAT_MONO16;
                        break;
                    }
                case 8:
                    if (stereo) {
                        wavFormat = AL_FORMAT_STEREO8;
                        break;
                    } else {
                        wavFormat = AL_FORMAT_MONO8;
                        break;
                    }
                default:
                    break;
            }

            alBufferData(buffer, wavFormat, wavFile->_data, wavFile->_dataByteSize, wavFile->_sampleRate);
            alSourcei(source, AL_BUFFER, buffer);
        }
    }

    void cOpenALSoundContext::Destroy(cSound* sound)
    {
        u32 buffer = sound->GetBuffer();
        u32 source = sound->GetSource();
        alDeleteBuffers(1, (ALuint*)&buffer);
        alDeleteSources(1, (ALuint*)&source);

        sound->~cSound();
        _app->GetMemoryPool()->Free(sound);
    }

    void cOpenALSoundContext::Play(const cSound* sound)
    {
        alSourcePlay(sound->GetSource());
    }

    void cOpenALSoundContext::Stop(const cSound* sound)
    {
        alSourceStop(sound->GetSource());
    }

    void cOpenALSoundContext::SetPosition(const cSound* sound, const glm::vec3& position)
    {
        alSource3f(sound->GetSource(), AL_POSITION, position.x, position.y, position.z);
    }

    void cOpenALSoundContext::SetVelocity(const cSound* sound, const glm::vec3& velocity)
    {
        alSource3f(sound->GetSource(), AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    }

    void cOpenALSoundContext::SetListenerPosition(const glm::vec3& position)
    {
        alListener3f(AL_POSITION, position.x, position.y, position.z);
    }

    void cOpenALSoundContext::SetListenerVelocity(const glm::vec3& velocity)
    {
        alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    }

    void cOpenALSoundContext::SetListenerOrientation(const glm::vec3& at, const glm::vec3& up)
    {
        ALfloat values[] = { at.x, at.y, at.z, up.x, up.y, up.z };
        alListenerfv(AL_ORIENTATION, &values[0]);
    }
}