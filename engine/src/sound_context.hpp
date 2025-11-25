// sound_context.hpp

#pragma once

#include <vector>
#include <AL/al.h>
#include <AL/alc.h>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "object.hpp"
#include "id_vec.hpp"
#include "types.hpp"

namespace realware
{
    class cApplication;
    class cSound;
    struct sWAVStructure;

    class iSoundContext : public iObject
    {
    public:
        explicit iSoundContext(cContext* context) : iObject(context) {}
        virtual ~iSoundContext() = default;

        virtual void Create(const std::string& filename, eCategory format, const sWAVStructure** file, types::u32& source, types::u32& buffer) = 0;
        virtual void Destroy(cSound* sound) = 0;
        virtual void Play(const cSound* sound) = 0;
        virtual void Stop(const cSound* sound) = 0;
        virtual void SetPosition(const cSound* sound, const glm::vec3& position) = 0;
        virtual void SetVelocity(const cSound* sound, const glm::vec3& velocity) = 0;
        virtual void SetListenerPosition(const glm::vec3& position) = 0;
        virtual void SetListenerVelocity(const glm::vec3& velocity) = 0;
        virtual void SetListenerOrientation(const glm::vec3& at, const glm::vec3& up) = 0;
    };

    class cOpenALSoundContext : public iSoundContext
    {
    public:
        cOpenALSoundContext(cContext* context);
        virtual ~cOpenALSoundContext() override final;

        inline virtual cType GetType() const override final { return cType("OpenALSoundContext"); }

        virtual void Create(const std::string& filename, eCategory format, const sWAVStructure** file, types::u32& source, types::u32& buffer) override final;
        virtual void Destroy(cSound* sound) override final;
        virtual void Play(const cSound* sound) override final;
        virtual void Stop(const cSound* sound) override final;
        virtual void SetPosition(const cSound* sound, const glm::vec3& position) override final;
        virtual void SetVelocity(const cSound* sound, const glm::vec3& velocity) override final;
        virtual void SetListenerPosition(const glm::vec3& position) override final;
        virtual void SetListenerVelocity(const glm::vec3& velocity) override final;
        virtual void SetListenerOrientation(const glm::vec3& at, const glm::vec3& up) override final;

    private:
        ALCdevice* _device = nullptr;
        ALCcontext* _context = nullptr;
    };
}