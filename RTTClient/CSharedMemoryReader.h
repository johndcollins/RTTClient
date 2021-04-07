#pragma once

#include <iostream>

class CSharedMemoryReader
{
public:
    CSharedMemoryReader();
    virtual ~CSharedMemoryReader();
    
    bool Update();

private:
    const std::string PrimarySharedMemoryAreaFileName = "FalconSharedMemoryArea";
    const std::string SecondarySharedMemoryFileName = "FalconSharedMemoryArea2";
    const std::string OsbSharedMemoryAreaFileName = "FalconSharedOsbMemoryArea";
    const std::string IntelliVibeSharedMemoryAreaFileName = "FalconIntellivibeSharedMemoryArea";
    //const std::string RadioClientControlSharedMemoryAreaFileName = "FalconRccSharedMemoryArea";
    //const std::string RadioClientStatusSharedMemoryAreaFileName = "FalconRcsSharedMemoryArea";
    const std::string StringSharedMemoryAreaFileName = "FalconSharedMemoryAreaString";
    const std::string DrawingSharedMemoryAreaFileName = "FalconSharedMemoryAreaDrawing";
    const std::string TexturesSharedMemoryAreaFileName = "FalconTexturesSharedMemoryArea";

    int g_mapPrimaryFileHeader;
    int g_mapSecondaryFileHeader;
    int g_mapOSBFileHeader;
    int g_mapIntelliVibeFileHeader;
    int g_mapStringFileHeader;
    int g_mapDrawingFileHeader;
    int g_mapTextureFileHeader;
    int g_mapTextureFileData;

    void Initialize();
    void Destroy();

    bool OpenPrimaryMemoryArea();
    void ClosePrimaryMemoryArea();

    bool OpenSecondaryMemoryArea();
    void CloseSecondaryMemoryArea();

    bool OpenOSBMemoryArea();
    void CloseOSBMemoryArea();

    bool OpenIntelliVibeMemoryArea();
    void CloseIntelliVibeMemoryArea();

    bool OpenStringMemoryArea();
    void CloseStringMemoryArea();

    bool OpenDrawingMemoryArea();
    void CloseDrawingMemoryArea();

    bool OpenTextureMemoryArea();
    void CloseTextureMemoryArea();
};