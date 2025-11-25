// filesystem_manager.cpp

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include "application.hpp"
#include "filesystem_manager.hpp"
#include "memory_pool.hpp"

using namespace types;

namespace realware
{
    mFileSystem::mFileSystem(cContext* context) : iObject(context)
    {
    }

    sFile* mFileSystem::CreateDataFile(const std::string& filepath, types::boolean isString)
    {
        std::ifstream inputFile(filepath, std::ios::binary);
        
        inputFile.seekg(0, std::ios::end);
        const usize byteSize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);
        const usize databyteSize = byteSize + (isString == K_TRUE ? 1 : 0);
        
        cApplication* app = GetApplication();

        u8* data = (u8*)app->GetMemoryPool()->Allocate(databyteSize);
        memset(data, 0, databyteSize);
        inputFile.read((char*)&data[0], byteSize);

        sFile* pFile = (sFile*)app->GetMemoryPool()->Allocate(sizeof(sFile));
        sFile* file = new (pFile) sFile;

        file->_data = data;
        file->_dataByteSize = databyteSize;

        return file;
    }

    void mFileSystem::DestroyDataFile(sFile* file)
    {
        void* fileData = file->_data;

        if (fileData == nullptr || file->_dataByteSize == 0)
            return;

        GetApplication()->GetMemoryPool()->Free(fileData);
    }
}