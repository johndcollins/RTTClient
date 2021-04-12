// ------------------------------------------------------------------------------------------------
// Copyright (c) John Collins. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------

#pragma once

typedef struct _DDCOLORKEY
{
    unsigned int       dwColorSpaceLowValue;
    unsigned int       dwColorSpaceHighValue;
} DDCOLORKEY;

#define DUMMYUNIONNAMEN(a)

typedef struct _DDSCAPS2
{
    unsigned int       dwCaps;
    unsigned int       dwCaps2;
    unsigned int       dwCaps3;
    union
    {
        unsigned int       dwCaps4;
        unsigned int       dwVolumeDepth;
    } DUMMYUNIONNAMEN(1);
} DDSCAPS2;

typedef struct _DDPIXELFORMAT
{
    unsigned int       dwSize;
    unsigned int       dwFlags;
    unsigned int       dwFourCC;
    union
    {
        unsigned int   dwRGBBitCount;
        unsigned int   dwYUVBitCount;
        unsigned int   dwZBufferBitDepth;
        unsigned int   dwAlphaBitDepth;
        unsigned int   dwLuminanceBitCount;
        unsigned int   dwBumpBitCount;
        unsigned int   dwPrivateFormatBitCount;
    } DUMMYUNIONNAMEN(1);
    union
    {
        unsigned int   dwRBitMask;
        unsigned int   dwYBitMask;
        unsigned int   dwStencilBitDepth;
        unsigned int   dwLuminanceBitMask;
        unsigned int   dwBumpDuBitMask;
        unsigned int   dwOperations;
    } DUMMYUNIONNAMEN(2);
    union
    {
        unsigned int   dwGBitMask;             // mask for green bits
        unsigned int   dwUBitMask;             // mask for U bits
        unsigned int   dwZBitMask;             // mask for Z bits
        unsigned int   dwBumpDvBitMask;        // mask for bump map V delta bits
        struct
        {
            unsigned short    wFlipMSTypes;       // Multisample methods supported via flip for this D3DFORMAT
            unsigned short    wBltMSTypes;        // Multisample methods supported via blt for this D3DFORMAT
        } MultiSampleCaps;
    } DUMMYUNIONNAMEN(3);
    union
    {
        unsigned int   dwBBitMask;             // mask for blue bits
        unsigned int   dwVBitMask;             // mask for V bits
        unsigned int   dwStencilBitMask;       // mask for stencil bits
        unsigned int   dwBumpLuminanceBitMask; // mask for luminance in bump map
    } DUMMYUNIONNAMEN(4);
    union
    {
        unsigned int   dwRGBAlphaBitMask;      // mask for alpha channel
        unsigned int   dwYUVAlphaBitMask;      // mask for alpha channel
        unsigned int   dwLuminanceAlphaBitMask;// mask for alpha channel
        unsigned int   dwRGBZBitMask;          // mask for Z channel
        unsigned int   dwYUVZBitMask;          // mask for Z channel
    } DUMMYUNIONNAMEN(5);
} DDPIXELFORMAT;

typedef struct _DDSURFACEDESC2
{
    unsigned int               dwSize;                 // size of the DDSURFACEDESC structure
    unsigned int               dwFlags;                // determines what fields are valid
    unsigned int               dwHeight;               // height of surface to be created
    unsigned int               dwWidth;                // width of input surface
    union
    {
        int            lPitch;                 // distance to start of next line (return value only)
        unsigned int           dwLinearSize;           // Formless late-allocated optimized surface size
    } DUMMYUNIONNAMEN(1);
    union
    {
        unsigned int           dwBackBufferCount;      // number of back buffers requested
        unsigned int           dwDepth;                // the depth if this is a volume texture
    } DUMMYUNIONNAMEN(5);
    union
    {
        unsigned int           dwMipMapCount;          // number of mip-map levels requestde
        // dwZBufferBitDepth removed, use ddpfPixelFormat one instead
        unsigned int           dwRefreshRate;          // refresh rate (used when display mode is described)
        unsigned int           dwSrcVBHandle;          // The source used in VB::Optimize
    } DUMMYUNIONNAMEN(2);
    unsigned int               dwAlphaBitDepth;        // depth of alpha buffer requested
    unsigned int               dwReserved;             // reserved
    void* lpSurface;              // pointer to the associated surface memory
    union
    {
        DDCOLORKEY      ddckCKDestOverlay;      // color key for destination overlay use
        unsigned int           dwEmptyFaceColor;       // Physical color for empty cubemap faces
    } DUMMYUNIONNAMEN(3);
    DDCOLORKEY          ddckCKDestBlt;          // color key for destination blt use
    DDCOLORKEY          ddckCKSrcOverlay;       // color key for source overlay use
    DDCOLORKEY          ddckCKSrcBlt;           // color key for source blt use
    union
    {
        DDPIXELFORMAT   ddpfPixelFormat;        // pixel format description of the surface
        unsigned int           dwFVF;                  // vertex format description of vertex buffers
    } DUMMYUNIONNAMEN(4);
    DDSCAPS2            ddsCaps;                // direct draw surface capabilities
    unsigned int               dwTextureStage;         // stage in multitexture cascade
} DDSURFACEDESC2;

struct DDS_PIXELFORMAT {
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwFourCC;
    unsigned long dwRGBBitCount;
    unsigned long dwRBitMask;
    unsigned long dwGBitMask;
    unsigned long dwBBitMask;
    unsigned long dwABitMask;
};

typedef struct {
    unsigned long           dwSize;
    unsigned long           dwFlags;
    unsigned long           dwHeight;
    unsigned long           dwWidth;
    unsigned long           dwPitchOrLinearSize;
    unsigned long           dwDepth;
    unsigned long           dwMipMapCount;
    unsigned long           dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    unsigned long           dwCaps;
    unsigned long           dwCaps2;
    unsigned long           dwCaps3;
    unsigned long           dwCaps4;
    unsigned long           dwReserved2;
} DDS_HEADER;

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} PIXEL;

typedef struct sPIXDIFF
{
    unsigned short x;
    unsigned short y;
    PIXEL color;

    sPIXDIFF()
    {
        x = 0xffff;
        y = 0xffff;
        color.r = 0x01;
        color.g = 0x02;
        color.b = 0x01;
    }
} PIXDIFF;