// audio.cpp

#include <fstream>
#include "application.hpp"
#include "audio.hpp"
#include "context.hpp"
#include "sound_context.hpp"
#include "engine.hpp"
#include "log.hpp"

using namespace types;

namespace harpy
{
	cSound::cSound(cContext* context, eFormat format, const std::string& path) : iObject(context), _audioBackend(context->GetSubsystem<cAudio>()->GetAPI())
	{
		const sApplicationCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

		sWAVStructure wav = {};

		std::ifstream inputFile(path, std::ios::binary);
		if (!inputFile)
		{
			Print("Error: can't open sound file at '" + path + "'!");

			return;
		}

		if (format == eFormat::WAV)
		{
			// Chunk
			inputFile.read((char*)&wav._type[0], 4);
			if (std::string((const char*)&wav._type[0]) != std::string("RIFF"))
				Print("Error: not a RIFF file!");
			inputFile.read((char*)&wav._chunkSize, sizeof(types::u32));
			inputFile.read((char*)&wav._format[0], 4);
			if (std::string((const char*)&wav._format[0]) != std::string("WAVE"))
				Print("Error: not a WAVE file!");

			// 1st Subchunk
			inputFile.read((char*)&wav._subchunk1ID, 4);
			if (std::string((const char*)&wav._subchunk1ID[0]) != std::string("fmt "))
				Print("Error: missing fmt header!");
			inputFile.read((char*)&wav._subchunk1Size, sizeof(types::u32));
			inputFile.read((char*)&wav._audioFormat, sizeof(types::u16));
			inputFile.read((char*)&_channelCount, sizeof(types::u16));
			inputFile.read((char*)&_sampleRate, sizeof(types::u32));
			inputFile.read((char*)&wav._byteRate, sizeof(types::u32));
			inputFile.read((char*)&wav._blockAlign, sizeof(types::u16));
			inputFile.read((char*)&_bitsPerSample, sizeof(types::u16));

			// 2nd Subchunk
			inputFile.read((char*)&wav._subchunk2ID, 4);
			if (std::string((const char*)&wav._subchunk2ID[0]) != std::string("data"))
				Print("Error: missing data header!");
			inputFile.read((char*)&wav._subchunk2Size, sizeof(types::u32));

			// Data
			const usize numSamples = wav._subchunk2Size / (_channelCount * (_bitsPerSample / 8));
			_dataByteSize = numSamples * (_bitsPerSample / 8) * _channelCount;
			_data = (u16*)memoryAllocator->Allocate(_dataByteSize, caps->memoryAlignment);
			if (_bitsPerSample == 16 && _channelCount == 2)
			{
				for (usize i = 0; i < numSamples; i++)
				{
					const usize idx = i * 2;
					inputFile.read((char*)&_data[idx], sizeof(u16));
					inputFile.read((char*)&_data[idx + 1], sizeof(u16));
				}
			}
			inputFile.close();
		}

		_audioBackend->Create(this);
	}

	cSound::~cSound()
	{
		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
		memoryAllocator->Deallocate(_data);
		_audioBackend->Destroy(this);
	}

	cAudio::cAudio(cContext* context) : iObject(context)
	{
		const sApplicationCapabilities* caps = context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
		_sounds = _context->Create<cIdVector<cSound>>(_context, caps->maxSoundCount);
	}

	cAudio::~cAudio()
	{
		if (_audioBackendAPI == API::NONE)
		{
			Print("Error: sound API not selected!");

			return;
		}
		else if (_audioBackendAPI == API::OAL)
		{
			if (_audioBackend)
				delete _audioBackend;
		}
	}

	cSound* cAudio::CreateSound(const std::string& id, cSound::eFormat format, const std::string& path)
	{
		return _sounds->Add(id, _context, format, path);
	}

	cSound* cAudio::FindSound(const std::string& id)
	{
		return _sounds->Find(id);
	}

	void cAudio::DestroySound(const std::string& id)
	{
		_sounds->Delete(id);
	}

	void cAudio::SetAPI(API api)
	{
		_audioBackendAPI = api;

		if (api == API::NONE)
		{
			Print("Error: sound API not selected!");

			return;
		}
		else if (api == API::OAL)
		{
			_audioBackend = new cOpenALSoundAPI(_context);
		}
	}
}