// filesystem_manager.hpp

#pragma once

#include "object.hpp"
#include "types.hpp"

namespace realware
{
    class cApplication;
        
    struct sFile
    {
        types::u8* _data = nullptr;
        types::usize _dataByteSize = 0;
    };

    class mFileSystem : public iObject
    {
    public:
        explicit mFileSystem(cContext* context);
        ~mFileSystem() = default;

        inline virtual cType GetType() const override final { return cType("FileSystem"); }

        sFile* CreateDataFile(const std::string& filepath, types::boolean isString);
        void DestroyDataFile(sFile* buffer);
    };
}