// sound_manager.hpp

#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "object.hpp"
#include "id_vec.hpp"
#include "types.hpp"

namespace realware
{
    class cApplication;
    class iSoundContext;

    struct sWAVStructure
    {
        types::u8 _type[5] = {};
        types::u8 _format[5] = {};
        types::u8 _subchunk1ID[5] = {};
        types::u8 _subchunk2ID[5] = {};
        types::u32 _chunkSize = 0;
        types::u32 _subchunk1Size = 0;
        types::u32 _sampleRate = 0;
        types::u32 _byteRate = 0;
        types::u32 _subchunk2Size = 0;
        types::u16 _audioFormat = 0;
        types::u16 _numChannels = 0;
        types::u16 _blockAlign = 0;
        types::u16 _bitsPerSample = 0;
        types::u32 _numSamples = 0;
        types::u32 _dataByteSize = 0;
        types::u16* _data = nullptr;
    };

    class cSound : public cFactoryObject
    {
    public:
        explicit cSound(cContext* context, types::u32 source, types::u32 buffer);
        ~cSound();

        inline virtual cType GetType() const override { return cType("Sound"); }

        inline eCategory GetFormat() const { return _format; }
        inline sWAVStructure* GetFile() const { return _file; }
        inline types::u32 GetSource() const { return _source; }
        inline types::u32 GetBuffer() const { return _buffer; }

    private:
        eCategory _format = eCategory::SOUND_FORMAT_WAV;
        sWAVStructure* _file = nullptr;
        types::u32 _source = 0;
        types::u32 _buffer = 0;
    };

    class mSound : public iObject
    {
    public:
        mSound(cContext* context, iSoundContext* soundContext);
        ~mSound() = default;

        inline virtual cType GetType() const override { return cType("SoundManager"); }

        cSound* CreateSound(const std::string& id, const std::string& filename, eCategory format);
        cSound* FindSound(const std::string& id);
        void DestroySound(const std::string& id);

    private:
        cApplication* _app = nullptr;
        iSoundContext* _context = nullptr;
        cIdVector<cSound> _sounds;
    };
}