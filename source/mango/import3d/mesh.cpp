/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2024 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <map>
#include <mango/core/core.hpp>
#include <mango/import3d/mesh.hpp>
#include "../../external/mikktspace/mikktspace.h"

/*

This is a convenience API for reading various 3D object formats and providing the data in
unified layout for rendering, or processing and dumping into a file that custom engine can
read more efficiently. The intent is accessibility not performance.

*/

namespace
{
    using namespace mango;
    using namespace mango::import3d;

    int callback_getNumFaces(const SMikkTSpaceContext* pContext)
    {
        Mesh& mesh = *reinterpret_cast<Mesh*>(pContext->m_pUserData);
        return int(mesh.triangles.size());
    }

    int callback_getNumVerticesOfFace(const SMikkTSpaceContext* pContext, const int iFace)
    {
        MANGO_UNREFERENCED(pContext);
        MANGO_UNREFERENCED(iFace);
        return 3;
    }

    void callback_getPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
    {
        Mesh& mesh = *reinterpret_cast<Mesh*>(pContext->m_pUserData);
        Vertex& vertex = mesh.triangles[iFace].vertex[iVert];
        fvPosOut[0] = vertex.position[0];
        fvPosOut[1] = vertex.position[1];
        fvPosOut[2] = vertex.position[2];
    }

    void callback_getNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
    {
        Mesh& mesh = *reinterpret_cast<Mesh*>(pContext->m_pUserData);
        Vertex& vertex = mesh.triangles[iFace].vertex[iVert];
        fvNormOut[0] = vertex.normal[0];
        fvNormOut[1] = vertex.normal[1];
        fvNormOut[2] = vertex.normal[2];
    }

    void callback_getTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
    {
        Mesh& mesh = *reinterpret_cast<Mesh*>(pContext->m_pUserData);
        Vertex& vertex = mesh.triangles[iFace].vertex[iVert];
        fvTexcOut[0] = vertex.texcoord[0];
        fvTexcOut[1] = vertex.texcoord[1];
    }

    void callback_setTSpaceBasic(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
    {
        Mesh& mesh = *reinterpret_cast<Mesh*>(pContext->m_pUserData);
        Vertex& vertex = mesh.triangles[iFace].vertex[iVert];
        vertex.tangent = float32x4(fvTangent[0], fvTangent[1], fvTangent[2], fSign);
    }

    void callback_setTSpace(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fvBiTangent[], const float fMagS, const float fMagT,
                            const tbool bIsOrientationPreserving, const int iFace, const int iVert)
    {
        MANGO_UNREFERENCED(pContext);
        MANGO_UNREFERENCED(fvTangent);
        MANGO_UNREFERENCED(fvBiTangent);
        MANGO_UNREFERENCED(fMagS);
        MANGO_UNREFERENCED(fMagT);
        MANGO_UNREFERENCED(bIsOrientationPreserving);
        MANGO_UNREFERENCED(iFace);
        MANGO_UNREFERENCED(iVert);
    }

} // namespace

namespace mango::import3d
{

static
constexpr float pi2 = float(math::pi * 2.0);

static inline
bool operator < (const Vertex& a, const Vertex& b)
{
    return std::memcmp(&a, &b, sizeof(Vertex)) < 0;
}

void computeTangents(Mesh& mesh)
{
    SMikkTSpaceInterface mik_interface;

    mik_interface.m_getNumFaces = callback_getNumFaces;
    mik_interface.m_getNumVerticesOfFace = callback_getNumVerticesOfFace;
    mik_interface.m_getPosition = callback_getPosition;
    mik_interface.m_getNormal = callback_getNormal;
    mik_interface.m_getTexCoord = callback_getTexCoord;
    mik_interface.m_setTSpaceBasic = callback_setTSpaceBasic;
    mik_interface.m_setTSpace = callback_setTSpace;

    SMikkTSpaceContext mik_context;

    mik_context.m_pInterface = &mik_interface;
    mik_context.m_pUserData = &mesh;

    tbool status = genTangSpaceDefault(&mik_context);
    MANGO_UNREFERENCED(status);
}

void loadTexture(Texture& texture, const filesystem::Path& path, const std::string& filename)
{
    /* NOTE:

    This is just a convenience function. What we really need is "TextureProvider" interface,
    which can return a memory mapped view of the texture file, or result of combining two textures
    into one when it is required. We might also want to use compressed textures and directly upload
    them into the GPU, or use compute decoder to decompress JPEG. DX12 also has DirectTexture API
    which could be supported when we don't always provide the data as Bitmap like we do here.

    A texture loading queue can cut the loading time into fraction but we want to keep this simple for now.

    */

    if (filename.empty())
    {
        return;
    }

    bool is_debug_enable = debugPrintIsEnable();
    debugPrintEnable(false);

    filesystem::File file(path, filename);

    image::Format format(32, image::Format::UNORM, image::Format::RGBA, 8, 8, 8, 8);
    texture = std::make_shared<image::Bitmap>(file, filename, format);

    debugPrintEnable(is_debug_enable);
    debugPrintLine("Texture: \"%s\" (%d x %d)", filename.c_str(), texture->width, texture->height);
}

Mesh convertMesh(const IndexedMesh& input)
{
    Mesh output;

    for (const Primitive& primitive : input.primitives)
    {
        switch (primitive.mode)
        {
            case Primitive::Mode::TRIANGLE_LIST:
            {
                Triangle triangle;
                triangle.material = primitive.material;

                const size_t start = primitive.start;
                const size_t end = start + primitive.count;

                for (size_t i = start; i < end; i += 3)
                {
                    triangle.vertex[0] = input.vertices[input.indices[i + 0]];
                    triangle.vertex[1] = input.vertices[input.indices[i + 1]];
                    triangle.vertex[2] = input.vertices[input.indices[i + 2]];

                    output.triangles.push_back(triangle);
                }

                break;
            }

            case Primitive::Mode::TRIANGLE_STRIP:
            {
                Triangle triangle;
                triangle.material = primitive.material;

                const size_t start = primitive.start;
                const size_t end = start + primitive.count;

                Vertex v0 = input.vertices[input.indices[start + 0]];
                Vertex v1 = input.vertices[input.indices[start + 1]];

                for (size_t i = start + 2; i < end; ++i)
                {
                    triangle.vertex[(i + 0) & 1] = v0;
                    triangle.vertex[(i + 1) & 1] = v1;
                    triangle.vertex[2] = input.vertices[input.indices[i]];

                    v0 = v1;
                    v1 = triangle.vertex[2];

                    output.triangles.push_back(triangle);
                }

                break;
            }

            case Primitive::Mode::TRIANGLE_FAN:
            {
                Triangle triangle;
                triangle.material = primitive.material;

                const size_t start = primitive.start;
                const size_t end = start + primitive.count;

                triangle.vertex[0] = input.vertices[input.indices[start + 0]];
                triangle.vertex[2] = input.vertices[input.indices[start + 1]];

                for (size_t i = start + 2; i < end; ++i)
                {
                    triangle.vertex[1] = triangle.vertex[2];
                    triangle.vertex[2] = input.vertices[input.indices[i]];

                    output.triangles.push_back(triangle);
                }

                break;
            }
        }
    }

    return output;
}

IndexedMesh convertMesh(const Mesh& input)
{
    IndexedMesh output;

    std::vector<Triangle> triangles = input.triangles;
 
    std::sort(triangles.begin(), triangles.end(), [] (const Triangle& a, const Triangle& b)
    {
        // sort triangles by material
        return a.material < b.material;
    });

    std::map<Vertex, size_t> unique;

    Primitive primitive;

    primitive.mode = Primitive::Mode::TRIANGLE_LIST;
    primitive.start = 0;
    primitive.count = 0;
    primitive.material = 0;

    for (const Triangle& triangle : input.triangles)
    {
        if (primitive.material != triangle.material)
        {
            if (primitive.count > 0)
            {
                output.primitives.push_back(primitive);

                primitive.start += primitive.count;
                primitive.count = 0;
                primitive.material = triangle.material;
            }
        }

        for (int i = 0; i < 3; ++i)
        {
            const Vertex& vertex = triangle.vertex[i];
            size_t index;

            auto it = unique.find(vertex);
            if (it != unique.end())
            {
                // vertex already exists; use it's index
                index = it->second;
            }
            else
            {
                index = output.vertices.size();
                unique[vertex] = index; // remember the index of this vertex
                output.vertices.push_back(vertex);
            }

            output.indices.push_back(u32(index));
            ++primitive.count;
        }
    }

    if (primitive.count > 0)
    {
        output.primitives.push_back(primitive);
    }

    return output;
}

Cube::Cube(float32x3 size)
{
    const float32x3 pos = size * 0.5f;
    const float32x3 neg = size * -0.5f;

    const float32x3 position0(neg.x, neg.y, neg.z);
    const float32x3 position1(pos.x, neg.y, neg.z);
    const float32x3 position2(neg.x, pos.y, neg.z);
    const float32x3 position3(pos.x, pos.y, neg.z);
    const float32x3 position4(neg.x, neg.y, pos.z);
    const float32x3 position5(pos.x, neg.y, pos.z);
    const float32x3 position6(neg.x, pos.y, pos.z);
    const float32x3 position7(pos.x, pos.y, pos.z);

    const float32x3 normal0( 1.0f, 0.0f, 0.0f);
    const float32x3 normal1(-1.0f, 0.0f, 0.0f);
    const float32x3 normal2( 0.0f, 1.0f, 0.0f);
    const float32x3 normal3( 0.0f,-1.0f, 0.0f);
    const float32x3 normal4( 0.0f, 0.0f, 1.0f);
    const float32x3 normal5( 0.0f, 0.0f,-1.0f);

    const float32x4 tangent0( 0.0f, 0.0f, 1.0f, 1.0f);
    const float32x4 tangent1( 0.0f, 0.0f,-1.0f, 1.0f);
    const float32x4 tangent2( 1.0f, 0.0f, 0.0f, 1.0f);
    const float32x4 tangent3(-1.0f, 0.0f, 0.0f, 1.0f);
    const float32x4 tangent4(-1.0f, 0.0f, 0.0f, 1.0f);
    const float32x4 tangent5( 1.0f, 0.0f, 0.0f, 1.0f);

    const float32x2 texcoord0(0.0f, 1.0f);
    const float32x2 texcoord1(0.0f, 0.0f);
    const float32x2 texcoord2(1.0f, 0.0f);
    const float32x2 texcoord3(1.0f, 1.0f);

    vertices =
    {
        // right (+x)
        { position1, normal0, tangent0, texcoord0 },
        { position3, normal0, tangent0, texcoord1 },
        { position7, normal0, tangent0, texcoord2 },
        { position5, normal0, tangent0, texcoord3 },

        // left (-x)
        { position4, normal1, tangent1, texcoord0 },
        { position6, normal1, tangent1, texcoord1 },
        { position2, normal1, tangent1, texcoord2 },
        { position0, normal1, tangent1, texcoord3 },

        // top (+y)
        { position2, normal2, tangent2, texcoord0 },
        { position6, normal2, tangent2, texcoord1 },
        { position7, normal2, tangent2, texcoord2 },
        { position3, normal2, tangent2, texcoord3 },

        // bottom (-y)
        { position4, normal3, tangent3, texcoord2 },
        { position0, normal3, tangent3, texcoord3 },
        { position1, normal3, tangent3, texcoord0 },
        { position5, normal3, tangent3, texcoord1 },

        // front (+z)
        { position5, normal4, tangent4, texcoord0 },
        { position7, normal4, tangent4, texcoord1 },
        { position6, normal4, tangent4, texcoord2 },
        { position4, normal4, tangent4, texcoord3 },

        // back (-z)
        { position0, normal5, tangent5, texcoord0 },
        { position2, normal5, tangent5, texcoord1 },
        { position3, normal5, tangent5, texcoord2 },
        { position1, normal5, tangent5, texcoord3 },
    };

    indices =
    {
         0,  1,  2,  0,  2,  3,
         4,  5,  6,  4,  6,  7,
         8,  9, 10,  8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23,
    };

    Primitive primitive;

    primitive.mode = Primitive::Mode::TRIANGLE_LIST;
    primitive.start = 0;
    primitive.count = 36;
    primitive.material = 0;

    primitives.push_back(primitive);
}

Torus::Torus(Parameters params)
{
    const float is = pi2 / params.innerSegments;
    const float js = pi2 / params.outerSegments;

    const float uscale = 4.0f / float(params.innerSegments);
    const float vscale = 1.0f / float(params.outerSegments);

    for (int i = 0; i < params.innerSegments + 1; ++i)
    {
        for (int j = 0; j < params.outerSegments + 1; ++j)
        {
            const float icos = std::cos(i * is);
            const float isin = std::sin(i * is);
            const float jcos = std::cos(j * js);
            const float jsin = std::sin(j * js);

            float32x3 position(
                icos * (params.innerRadius + jcos * params.outerRadius),
                isin * (params.innerRadius + jcos * params.outerRadius),
                jsin * params.outerRadius);
            float32x3 tangent = normalize(float32x3(-position.y, position.x, 0.0f));

            Vertex vertex;

            vertex.position = position;
            vertex.normal = normalize(float32x3(jcos * icos, jcos * isin, jsin));
            vertex.tangent = float32x4(tangent, 1.0f);;
            vertex.texcoord = float32x2(i * uscale, j * vscale);

            vertices.push_back(vertex);
        }
    }

    for (int i = 0; i < params.innerSegments; ++i)
    {
        int ci = (i + 0) * (params.outerSegments + 1);
        int ni = (i + 1) * (params.outerSegments + 1);

        for (int j = 0; j < params.outerSegments; ++j)
        {
            int a = ci + j + 0;
            int b = ni + j + 0;
            int c = ci + j + 1;
            int d = ni + j + 1;

            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(c);

            indices.push_back(b);
            indices.push_back(d);
            indices.push_back(c);
        }
    }

    Primitive primitive;

    primitive.mode = Primitive::Mode::TRIANGLE_LIST;
    primitive.start = 0;
    primitive.count = u32(indices.size());
    primitive.material = 0;

    primitives.push_back(primitive);
}

// Torus knot generation
// written by Jari Komppa aka Sol / Trauma
// Based on:
// http://www.blackpawn.com/texts/pqtorus/default.html

Torusknot::Torusknot(Parameters params)
{
    params.scale *= 0.5f;
    params.thickness *= params.scale;

    const float uscale = params.uscale / params.facets;
    const float vscale = params.vscale / params.steps;

    // generate vertices
    vertices.resize((params.steps + 1) * (params.facets + 1) + 1);

    float32x3 centerpoint;
    float Pp = params.p * 0 * pi2 / params.steps;
    float Qp = params.q * 0 * pi2 / params.steps;
    float r = (0.5f * (2.0f + std::sin(Qp))) * params.scale;
    centerpoint.x = r * std::cos(Pp);
    centerpoint.y = r * std::cos(Qp);
    centerpoint.z = r * std::sin(Pp);

    for (int i = 0; i < params.steps; i++)
    {
        float32x3 nextpoint;
        Pp = params.p * (i + 1) * pi2 / params.steps;
        Qp = params.q * (i + 1) * pi2 / params.steps;
        r = (0.5f * (2.0f + std::sin(Qp))) * params.scale;
        nextpoint.x = r * std::cos(Pp);
        nextpoint.y = r * std::cos(Qp);
        nextpoint.z = r * std::sin(Pp);

        float32x3 T = nextpoint - centerpoint;
        float32x3 N = nextpoint + centerpoint;
        float32x3 B = cross(T, N);
        N = cross(B, T);
        B = normalize(B);
        N = normalize(N);

        for (int j = 0; j < params.facets; j++)
        {
            float pointx = std::sin(j * pi2 / params.facets) * params.thickness * ((std::sin(params.clumpOffset + params.clumps * i * pi2 / params.steps) * params.clumpScale) + 1);
            float pointy = std::cos(j * pi2 / params.facets) * params.thickness * ((std::cos(params.clumpOffset + params.clumps * i * pi2 / params.steps) * params.clumpScale) + 1);

            float32x3 normal = N * pointx + B * pointy;
            float32x3 tangent = normalize(B * pointx - N * pointy);

            const int offset = i * (params.facets + 1) + j;

            vertices[offset].position = centerpoint + normal;
            vertices[offset].normal = normalize(normal);
            vertices[offset].tangent = float32x4(tangent, 1.0f);
            vertices[offset].texcoord = float32x2(j * uscale, i * vscale);
        }

        // create duplicate vertex for sideways wrapping
        // otherwise identical to first vertex in the 'ring' except for the U coordinate
        vertices[i * (params.facets + 1) + params.facets] = vertices[i * (params.facets + 1)];
        vertices[i * (params.facets + 1) + params.facets].texcoord.x = params.uscale;
        
        centerpoint = nextpoint;
    }

    // create duplicate ring of vertices for longways wrapping
    // otherwise identical to first 'ring' in the knot except for the V coordinate
    for (int j = 0; j < params.facets; j++)
    {
        vertices[params.steps * (params.facets + 1) + j] = vertices[j];
        vertices[params.steps * (params.facets + 1) + j].texcoord.y = params.vscale;
    }

    // finally, there's one vertex that needs to be duplicated due to both U and V coordinate.
    vertices[params.steps * (params.facets + 1) + params.facets] = vertices[0];
    vertices[params.steps * (params.facets + 1) + params.facets].texcoord = float32x2(params.uscale, params.vscale);

    // generate indices
    std::vector<int> stripIndices((params.steps + 1) * params.facets * 2);

    for (int j = 0; j < params.facets; j++)
    {
        for (int i = 0; i < params.steps + 1; i++)
        {
            stripIndices[i * 2 + 0 + j * (params.steps + 1) * 2] = (j + 1 + i * (params.facets + 1));
            stripIndices[i * 2 + 1 + j * (params.steps + 1) * 2] = (j + 0 + i * (params.facets + 1));
        }
    }

    // convert triangle strip into triangles
    for (size_t i = 2; i < stripIndices.size(); ++i)
    {
        int s = i & 1; // swap triangle winding-order
        indices.push_back(stripIndices[i - 2 + s]);
        indices.push_back(stripIndices[i - 1 - s]);
        indices.push_back(stripIndices[i]);
    }

    Primitive primitive;

    primitive.mode = Primitive::Mode::TRIANGLE_LIST;
    primitive.start = 0;
    primitive.count = u32(indices.size());
    primitive.material = 0;

    primitives.push_back(primitive);
}

} // namespace mango::import3d
