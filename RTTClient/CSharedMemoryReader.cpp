#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "CSharedMemoryReader.h"
#include "FlightData.h"
#include "IntelliVibe.h"
#include "BMSTexture.h"

CSharedMemoryReader::CSharedMemoryReader()
{
	Initialize();
};

CSharedMemoryReader::~CSharedMemoryReader()
{
	Destroy();
};

void CSharedMemoryReader::Initialize()
{
	OpenPrimaryMemoryArea();
	OpenSecondaryMemoryArea();
	OpenOSBMemoryArea();
	OpenIntelliVibeMemoryArea();
	OpenStringMemoryArea();
	OpenDrawingMemoryArea();
	OpenTextureMemoryArea();
};

void CSharedMemoryReader::Destroy()
{
	ClosePrimaryMemoryArea();
	CloseSecondaryMemoryArea();
	CloseOSBMemoryArea();
	CloseIntelliVibeMemoryArea();
	CloseStringMemoryArea();
	CloseDrawingMemoryArea();
	CloseTextureMemoryArea();
};

bool CSharedMemoryReader::Update()
{
	if (OpenPrimaryMemoryArea())
	{
		void* fdPrimary = mmap(NULL, sizeof(class FlightData), PROT_READ, MAP_SHARED, g_mapPrimaryFileHeader, 0);
		if (fdPrimary == MAP_FAILED)
		{
			return false;
		}

		FlightData flightData = FlightData();
		memcpy(&flightData, fdPrimary, sizeof(class FlightData));
		munmap(fdPrimary, sizeof(class FlightData));

		// Update something
	}

	if (OpenSecondaryMemoryArea())
	{
		void* fdSecondary = mmap(NULL, sizeof(class FlightData2), PROT_READ, MAP_SHARED, g_mapSecondaryFileHeader, 0);
		if (fdSecondary == MAP_FAILED)
		{
			return false;
		}

		FlightData2 flightData2 = FlightData2();
		memcpy(&flightData2, fdSecondary, sizeof(class FlightData2));
		munmap(fdSecondary, sizeof(class FlightData2));

		// Update something

		// Get String Data
		if (OpenStringMemoryArea())
		{
			void* fdString = mmap(NULL, flightData2.StringAreaSize, PROT_READ, MAP_SHARED, g_mapStringFileHeader, 0);
			if (fdString == MAP_FAILED)
			{
				return false;
			}

			StringData stringData = StringData();
			memcpy(&stringData, fdString, flightData2.StringAreaSize);
			munmap(fdString, flightData2.StringAreaSize);

			// Update something
		}

		// Get Drawing Data
		if (OpenDrawingMemoryArea())
		{
			void* fdDrawing = mmap(NULL, flightData2.DrawingAreaSize, PROT_READ, MAP_SHARED, g_mapDrawingFileHeader, 0);
			if (fdDrawing == MAP_FAILED)
			{
				return false;
			}

			DrawingData drawingData = DrawingData();
			memcpy(&drawingData, fdDrawing, flightData2.DrawingAreaSize);
			munmap(fdDrawing, flightData2.DrawingAreaSize);

			// Update something

		}
	}

	if (OpenOSBMemoryArea())
	{
		void* fdOSB = mmap(NULL, sizeof(class OSBData), PROT_READ, MAP_SHARED, g_mapOSBFileHeader, 0);
		if (fdOSB == MAP_FAILED)
		{
			return false;
		}
		
		OSBData osbData = OSBData();
		memcpy(&osbData, fdOSB, sizeof(class OSBData));
		munmap(fdOSB, sizeof(class OSBData));

		// Update something
	}

	if (OpenIntelliVibeMemoryArea())
	{
		void* fdIntelliVibe = mmap(NULL, sizeof(class IntelliVibeData), PROT_READ, MAP_SHARED, g_mapIntelliVibeFileHeader, 0);
		if (fdIntelliVibe == MAP_FAILED)
		{
			return false;
		}

		IntelliVibeData intelliVibeData = IntelliVibeData();
		memcpy(&intelliVibeData, fdIntelliVibe, sizeof(class IntelliVibeData));
		munmap(fdIntelliVibe, sizeof(class IntelliVibeData));

		// Update something
	}

	// Get Texture Data
	if (OpenTextureMemoryArea())
	{
		DDS_HEADER *fdTextureHeader = (DDS_HEADER*)mmap(NULL, 40, PROT_READ, MAP_SHARED, g_mapTextureFileHeader, 0);
		if (fdTextureHeader == MAP_FAILED)
		{
			return false;
		}

		DDS_HEADER textureHeader;
		memcpy(&textureHeader, fdTextureHeader, 40);

		long imageDataSize = textureHeader.dwWidth * textureHeader.dwHeight * (textureHeader.ddspf.dwRGBBitCount / 8);

		void* fdTextureData = mmap(NULL, 40 + imageDataSize, PROT_READ, MAP_SHARED, g_mapTextureFileHeader, 0);
		if (fdTextureData == MAP_FAILED)
		{
			return false;
		}

		unsigned char *textureData;
		memcpy(&textureData, fdTextureData, 40 + imageDataSize);
		munmap(fdTextureData, 40 + imageDataSize);

		textureData += 128; // if there are issues try and change to 40

		// Update something
	}

	return true;
}

bool CSharedMemoryReader::OpenPrimaryMemoryArea()
{
	if (g_mapPrimaryFileHeader > 0)
		return true;

	g_mapPrimaryFileHeader = shm_open(PrimarySharedMemoryAreaFileName.c_str(), 0, O_RDONLY);
	if (g_mapPrimaryFileHeader < 0)
		return -1;

	return true;
};

void CSharedMemoryReader::ClosePrimaryMemoryArea()
{
	if (!g_mapPrimaryFileHeader)
		return;

	close(g_mapPrimaryFileHeader);
};

bool CSharedMemoryReader::OpenSecondaryMemoryArea()
{
	if (g_mapSecondaryFileHeader > 0)
		return true;

	g_mapSecondaryFileHeader = shm_open(SecondarySharedMemoryFileName.c_str(), 0, O_RDONLY);
	if (g_mapSecondaryFileHeader < 0)
		return -1;

	return true;
};

void CSharedMemoryReader::CloseSecondaryMemoryArea()
{
	if (!g_mapSecondaryFileHeader)
		return;

	close(g_mapSecondaryFileHeader);
};

bool CSharedMemoryReader::OpenOSBMemoryArea()
{
	if (g_mapOSBFileHeader > 0)
		return true;

	g_mapOSBFileHeader = shm_open(OsbSharedMemoryAreaFileName.c_str(), 0, O_RDONLY);
	if (g_mapOSBFileHeader < 0)
		return -1;

	return true;
};

void CSharedMemoryReader::CloseOSBMemoryArea()
{
	if (!g_mapOSBFileHeader)
		return;

	close(g_mapOSBFileHeader);
};


bool CSharedMemoryReader::OpenIntelliVibeMemoryArea()
{
	if (g_mapIntelliVibeFileHeader > 0)
		return true;

	g_mapIntelliVibeFileHeader = shm_open(IntelliVibeSharedMemoryAreaFileName.c_str(), 0, O_RDONLY);
	if (g_mapIntelliVibeFileHeader < 0)
		return -1;

	return true;
};

void CSharedMemoryReader::CloseIntelliVibeMemoryArea()
{
	if (!g_mapIntelliVibeFileHeader)
		return;

	close(g_mapIntelliVibeFileHeader);
};

bool CSharedMemoryReader::OpenStringMemoryArea()
{
	if (g_mapStringFileHeader > 0)
		return true;

	g_mapStringFileHeader = shm_open(StringSharedMemoryAreaFileName.c_str(), 0, O_RDONLY);
	if (g_mapStringFileHeader < 0)
		return -1;

	return true;
};

void CSharedMemoryReader::CloseStringMemoryArea()
{
	if (!g_mapStringFileHeader)
		return;

	close(g_mapStringFileHeader);
};

bool CSharedMemoryReader::OpenDrawingMemoryArea()
{
	if (g_mapDrawingFileHeader > 0)
		return true;

	g_mapDrawingFileHeader = shm_open(DrawingSharedMemoryAreaFileName.c_str(), 0, O_RDONLY);
	if (g_mapDrawingFileHeader < 0)
		return -1;

	return true;
};

void CSharedMemoryReader::CloseDrawingMemoryArea()
{
	if (!g_mapDrawingFileHeader)
		return;

	close(g_mapDrawingFileHeader);
};

bool CSharedMemoryReader::OpenTextureMemoryArea()
{
	if (!g_mapTextureFileHeader)
	{
		g_mapTextureFileHeader = shm_open(TexturesSharedMemoryAreaFileName.c_str(), 0, O_RDONLY);
		if (g_mapTextureFileHeader < 0)
			return -1;
	}

	if (!g_mapTextureFileData)
	{
		g_mapTextureFileData = shm_open(TexturesSharedMemoryAreaFileName.c_str(), 0, O_RDONLY);
		if (g_mapTextureFileData < 0)
			return -1;
	}

	return true;
};

void CSharedMemoryReader::CloseTextureMemoryArea()
{
	if (!g_mapTextureFileHeader)
		close(g_mapTextureFileHeader);

	if (!g_mapTextureFileData)
		close(g_mapTextureFileData);
};