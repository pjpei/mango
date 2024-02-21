/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2022 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#pragma once

#include <mango/core/bits.hpp>
#include <mango/core/memory.hpp>
#include <mango/core/string.hpp>
#include <mango/core/exception.hpp>
#include <mango/image/format.hpp>
#include <mango/image/fourcc.hpp>

namespace mango::image
{
    class Surface;

    static constexpr
    u32 makeTextureCompression(u32 format, u32 index, u32 flags) noexcept
    {
        return flags | (index << 8) | format;
    }

    struct TextureCompression
    {
        struct Status : mango::Status
        {
            bool direct = false;
        };

        using DecodeFunc = void (*)(const TextureCompression& info, u8* output, const u8* input, size_t stride);
        using EncodeFunc = void (*)(const TextureCompression& info, u8* output, const u8* input, size_t stride);

        enum BaseFormat : u32
        {
            FXT1        = 1,
            ATC         = 2, 
            AMD_3DC     = 3,
            LATC        = 4,
            DXT         = 5,
            RGTC        = 6,
            BPTC        = 7,
            ETC1        = 8,
            ETC2_EAC    = 9,
            PVRTC1      = 10,
            PVRTC2      = 11,
            PVRTC_EXT   = 12,
            ASTC        = 13,
            ASTC_HDR    = 14,
            PACKED      = 15,
        };

        enum Flags : u32
        {
            PVR      = 0x00010000, // Imagination PVR compressed texture
            BC       = 0x00020000, // DirectX Block Compression
            YUV      = 0x02000000, // YUV colorspace
            FLOAT    = 0x04000000, // 16 or 32 bit floating point color
            SURFACE  = 0x08000000, // Surface (not block) compression
            YFLIP    = 0x10000000, // Origin is at bottom left
            SIGNED   = 0x20000000, // Signed normalized color
            ALPHA    = 0x40000000, // Color has alpha bits
            SRGB     = 0x80000000, // sRGB colorspace
            MASK     = 0xffff0000
        };

        enum : u32
        {
            NONE                          = makeTextureCompression(0, 0, 0),

            // 3DFX_texture_compression_FXT1
            FXT1_RGB                      = makeTextureCompression(FXT1, 0, 0),
            FXT1_RGBA                     = makeTextureCompression(FXT1, 1, ALPHA),

            // AMD_compressed_ATC_texture
            ATC_RGB                       = makeTextureCompression(ATC, 0, 0),
            ATC_RGBA_EXPLICIT_ALPHA       = makeTextureCompression(ATC, 1, ALPHA),
            ATC_RGBA_INTERPOLATED_ALPHA   = makeTextureCompression(ATC, 2, ALPHA),

            // AMD_compressed_3DC_texture
            AMD_3DC_X                     = makeTextureCompression(AMD_3DC, 0, 0),
            AMD_3DC_XY                    = makeTextureCompression(AMD_3DC, 1, 0),

            // LATC
            LATC1_LUMINANCE               = makeTextureCompression(LATC, 0, 0),
            LATC1_SIGNED_LUMINANCE        = makeTextureCompression(LATC, 1, SIGNED),
            LATC2_LUMINANCE_ALPHA         = makeTextureCompression(LATC, 2,          ALPHA),
            LATC2_SIGNED_LUMINANCE_ALPHA  = makeTextureCompression(LATC, 3, SIGNED | ALPHA),

            // DXT
            DXT1                          = makeTextureCompression(DXT, 0, BC),
            DXT1_ALPHA1                   = makeTextureCompression(DXT, 1, BC | ALPHA),
            DXT3                          = makeTextureCompression(DXT, 2, BC | ALPHA),
            DXT5                          = makeTextureCompression(DXT, 3, BC | ALPHA),
            DXT1_SRGB                     = makeTextureCompression(DXT, 4, BC |         SRGB),
            DXT1_ALPHA1_SRGB              = makeTextureCompression(DXT, 5, BC | ALPHA | SRGB),
            DXT3_SRGB                     = makeTextureCompression(DXT, 6, BC | ALPHA | SRGB),
            DXT5_SRGB                     = makeTextureCompression(DXT, 7, BC | ALPHA | SRGB),

            // RGTC
            RGTC1_RED                     = makeTextureCompression(RGTC, 0, BC),
            RGTC1_SIGNED_RED              = makeTextureCompression(RGTC, 1, BC | SIGNED),
            RGTC2_RG                      = makeTextureCompression(RGTC, 2, BC),
            RGTC2_SIGNED_RG               = makeTextureCompression(RGTC, 3, BC | SIGNED),

            // BPTC
            BPTC_RGB_UNSIGNED_FLOAT       = makeTextureCompression(BPTC, 0, BC | FLOAT),
            BPTC_RGB_SIGNED_FLOAT         = makeTextureCompression(BPTC, 1, BC | FLOAT | SIGNED),
            BPTC_RGBA_UNORM               = makeTextureCompression(BPTC, 2, BC | ALPHA),
            BPTC_SRGB_ALPHA_UNORM         = makeTextureCompression(BPTC, 3, BC | ALPHA | SRGB),

            // OES_compressed_ETC1_RGB8_texture
            ETC1_RGB                      = makeTextureCompression(ETC1, 0, 0),

            // ETC2 / EAC
            EAC_R11                       = makeTextureCompression(ETC2_EAC, 0, 0),
            EAC_SIGNED_R11                = makeTextureCompression(ETC2_EAC, 1, SIGNED),
            EAC_RG11                      = makeTextureCompression(ETC2_EAC, 2, 0),
            EAC_SIGNED_RG11               = makeTextureCompression(ETC2_EAC, 3, SIGNED),
            ETC2_RGB                      = makeTextureCompression(ETC2_EAC, 4, 0),
            ETC2_SRGB                     = makeTextureCompression(ETC2_EAC, 5,         SRGB),
            ETC2_RGB_ALPHA1               = makeTextureCompression(ETC2_EAC, 6, ALPHA),
            ETC2_SRGB_ALPHA1              = makeTextureCompression(ETC2_EAC, 7, ALPHA | SRGB),
            ETC2_RGBA                     = makeTextureCompression(ETC2_EAC, 8, ALPHA),
            ETC2_SRGB_ALPHA8              = makeTextureCompression(ETC2_EAC, 9, ALPHA | SRGB),

            // IMG_texture_compression_pvrtc
            PVRTC_RGB_4BPP                = makeTextureCompression(PVRTC1, 0, PVR | SURFACE),
            PVRTC_RGB_2BPP                = makeTextureCompression(PVRTC1, 1, PVR | SURFACE),
            PVRTC_RGBA_4BPP               = makeTextureCompression(PVRTC1, 2, PVR | SURFACE | ALPHA),
            PVRTC_RGBA_2BPP               = makeTextureCompression(PVRTC1, 3, PVR | SURFACE | ALPHA),

            // IMG_texture_compression_pvrtc2
            PVRTC2_RGBA_2BPP              = makeTextureCompression(PVRTC2, 4, PVR | SURFACE | ALPHA),
            PVRTC2_RGBA_4BPP              = makeTextureCompression(PVRTC2, 5, PVR | SURFACE | ALPHA),

            // EXT_pvrtc_sRGB
            PVRTC_SRGB_2BPP               = makeTextureCompression(PVRTC_EXT, 6, PVR | SURFACE |         SRGB),
            PVRTC_SRGB_4BPP               = makeTextureCompression(PVRTC_EXT, 7, PVR | SURFACE |         SRGB),
            PVRTC_SRGB_ALPHA_2BPP         = makeTextureCompression(PVRTC_EXT, 8, PVR | SURFACE | ALPHA | SRGB),
            PVRTC_SRGB_ALPHA_4BPP         = makeTextureCompression(PVRTC_EXT, 9, PVR | SURFACE | ALPHA | SRGB),

            // VK_IMG_format_pvrtc
            PVRTC1_2BPP_SRGB_BLOCK_IMG    = PVRTC_SRGB_ALPHA_2BPP,
            PVRTC1_2BPP_UNORM_BLOCK_IMG   = PVRTC_RGBA_2BPP,
            PVRTC1_4BPP_SRGB_BLOCK_IMG    = PVRTC_SRGB_ALPHA_4BPP,
            PVRTC1_4BPP_UNORM_BLOCK_IMG   = PVRTC_RGBA_4BPP,
            PVRTC2_2BPP_SRGB_BLOCK_IMG    = makeTextureCompression(PVRTC2, 4, PVR | SURFACE | ALPHA | SRGB),
            PVRTC2_2BPP_UNORM_BLOCK_IMG   = PVRTC2_RGBA_2BPP,
            PVRTC2_4BPP_SRGB_BLOCK_IMG    = makeTextureCompression(PVRTC2, 5, PVR | SURFACE | ALPHA | SRGB),
            PVRTC2_4BPP_UNORM_BLOCK_IMG   = PVRTC2_RGBA_4BPP,

            // KHR_texture_compression_astc_ldr
            // KHR_texture_compression_astc_hdr
            ASTC_RGBA_4x4                 = makeTextureCompression(ASTC,  0, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_5x4                 = makeTextureCompression(ASTC,  1, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_5x5                 = makeTextureCompression(ASTC,  2, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_6x5                 = makeTextureCompression(ASTC,  3, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_6x6                 = makeTextureCompression(ASTC,  4, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_8x5                 = makeTextureCompression(ASTC,  5, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_8x6                 = makeTextureCompression(ASTC,  6, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_8x8                 = makeTextureCompression(ASTC,  7, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_10x5                = makeTextureCompression(ASTC,  8, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_10x6                = makeTextureCompression(ASTC,  9, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_10x8                = makeTextureCompression(ASTC, 10, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_10x10               = makeTextureCompression(ASTC, 11, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_12x10               = makeTextureCompression(ASTC, 12, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_12x12               = makeTextureCompression(ASTC, 13, SURFACE | ALPHA | FLOAT),
            ASTC_SRGB_ALPHA_4x4           = makeTextureCompression(ASTC, 14, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_5x4           = makeTextureCompression(ASTC, 15, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_5x5           = makeTextureCompression(ASTC, 16, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_6x5           = makeTextureCompression(ASTC, 17, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_6x6           = makeTextureCompression(ASTC, 18, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_8x5           = makeTextureCompression(ASTC, 19, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_8x6           = makeTextureCompression(ASTC, 20, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_8x8           = makeTextureCompression(ASTC, 21, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_10x5          = makeTextureCompression(ASTC, 22, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_10x6          = makeTextureCompression(ASTC, 23, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_10x8          = makeTextureCompression(ASTC, 24, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_10x10         = makeTextureCompression(ASTC, 25, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_12x10         = makeTextureCompression(ASTC, 26, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_12x12         = makeTextureCompression(ASTC, 27, SURFACE | ALPHA | SRGB),

            // OES_texture_compression_astc
            ASTC_RGBA_3x3x3               = makeTextureCompression(ASTC_HDR,  0, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_4x3x3               = makeTextureCompression(ASTC_HDR,  1, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_4x4x3               = makeTextureCompression(ASTC_HDR,  2, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_4x4x4               = makeTextureCompression(ASTC_HDR,  3, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_5x4x4               = makeTextureCompression(ASTC_HDR,  4, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_5x5x4               = makeTextureCompression(ASTC_HDR,  5, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_5x5x5               = makeTextureCompression(ASTC_HDR,  6, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_6x5x5               = makeTextureCompression(ASTC_HDR,  7, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_6x6x5               = makeTextureCompression(ASTC_HDR,  8, SURFACE | ALPHA | FLOAT),
            ASTC_RGBA_6x6x6               = makeTextureCompression(ASTC_HDR,  9, SURFACE | ALPHA | FLOAT),
            ASTC_SRGB_ALPHA_3x3x3         = makeTextureCompression(ASTC_HDR, 10, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_4x3x3         = makeTextureCompression(ASTC_HDR, 11, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_4x4x3         = makeTextureCompression(ASTC_HDR, 12, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_4x4x4         = makeTextureCompression(ASTC_HDR, 13, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_5x4x4         = makeTextureCompression(ASTC_HDR, 14, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_5x5x4         = makeTextureCompression(ASTC_HDR, 15, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_5x5x5         = makeTextureCompression(ASTC_HDR, 16, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_6x5x5         = makeTextureCompression(ASTC_HDR, 17, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_6x6x5         = makeTextureCompression(ASTC_HDR, 18, SURFACE | ALPHA | SRGB),
            ASTC_SRGB_ALPHA_6x6x6         = makeTextureCompression(ASTC_HDR, 19, SURFACE | ALPHA | SRGB),

            // Packed Pixels
            RGB9_E5                       = makeTextureCompression(PACKED, 0, FLOAT),
            R11F_G11F_B10F                = makeTextureCompression(PACKED, 1, FLOAT),
            R10F_G11F_B11F                = makeTextureCompression(PACKED, 2, FLOAT),
            UYVY                          = makeTextureCompression(PACKED, 3, YUV),
            YUY2                          = makeTextureCompression(PACKED, 4, YUV),
            G8R8G8B8                      = makeTextureCompression(PACKED, 5, 0),
            R8G8B8G8                      = makeTextureCompression(PACKED, 6, 0),

            // BC (these alias with DXT, RGTC and BPTC)
            BC1_UNORM                     = DXT1,
            BC1_UNORM_SRGB                = DXT1_SRGB,
            BC1_UNORM_ALPHA               = DXT1_ALPHA1,
            BC1_UNORM_ALPHA_SRGB          = DXT1_ALPHA1_SRGB,
            BC2_UNORM                     = DXT3,
            BC2_UNORM_SRGB                = DXT3_SRGB,
            BC3_UNORM                     = DXT5,
            BC3_UNORM_SRGB                = DXT5_SRGB,
            BC4_UNORM                     = RGTC1_RED,
            BC4_SNORM                     = RGTC1_SIGNED_RED,
            BC5_UNORM                     = RGTC2_RG,
            BC5_SNORM                     = RGTC2_SIGNED_RG,
            BC6H_UF16                     = BPTC_RGB_UNSIGNED_FLOAT,
            BC6H_SF16                     = BPTC_RGB_SIGNED_FLOAT,
            BC7_UNORM                     = BPTC_RGBA_UNORM,
            BC7_UNORM_SRGB                = BPTC_SRGB_ALPHA_UNORM
        };

        u32 compression;    // block format (including flags)
        u32 dxgi;           // DXGI format
        u32 opengl;         // OpenGL format
        u32 vulkan;         // Vulkan format

        int width;          // block width
        int height;         // block height
        int depth;          // block depth
        int bytes;          // block size in bytes
        Format format;      // pixel format for encode/decode

        DecodeFunc decode;  // decoding function
        EncodeFunc encode;  // encoding function

        TextureCompression();
        TextureCompression(u32 compression, u32 dxgi, u32 gl, u32 vk,
                           int width, int height, int depth, int bytes,
                           const Format& format, DecodeFunc decode, EncodeFunc encode);
        TextureCompression(u32 compression);
        TextureCompression(dxgi::TextureFormat format);
        TextureCompression(opengl::TextureFormat format);
        TextureCompression(vulkan::TextureFormat format);

        Status decompress(const Surface& surface, ConstMemory memory) const;
        Status compress(Memory memory, const Surface& surface) const;

        // number of blocks horizontally required to compress the surface
        int getBlocksX(int width) const;

        // number of blocks vertically required to compress the surface
        int getBlocksY(int height) const;

        // number of blocks required to compress the surface
        int getBlockCount(int width, int height) const;

        // amount of memory required to store compressed blocks
        u64 getBlockBytes(int width, int height) const;
    };

} // namespace mango::image

namespace mango
{

    namespace opengl
    {
        static inline
        u32 getTextureCompression(u32 format)
        {
            return image::TextureCompression(opengl::TextureFormat(format)).compression;
        }

        static inline
        u32 getTextureFormat(u32 compression)
        {
            return image::TextureCompression(compression).opengl;
        }
    }

    namespace vulkan
    {
        static inline
        u32 getTextureCompression(u32 format)
        {
            return image::TextureCompression(vulkan::TextureFormat(format)).compression;
        }

        static inline
        u32 getTextureFormat(u32 compression)
        {
            return image::TextureCompression(compression).vulkan;
        }
    }

    namespace dxgi
    {
        static inline
        u32 getTextureCompression(u32 format)
        {
            return image::TextureCompression(dxgi::TextureFormat(format)).compression;
        }

        static inline
        u32 getTextureFormat(u32 compression)
        {
            return image::TextureCompression(compression).dxgi;
        }
    }

} // namespace mango
