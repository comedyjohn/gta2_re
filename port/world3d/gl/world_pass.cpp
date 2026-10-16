#include "world_pass.h"

#include "../camera.h"
#include "../game_view.h"
#include "../scene.h"
#include "gl.h"
#include "renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace world3d
{
namespace
{
constexpr int kRegionSize = 16; // cells
constexpr int kRegionsPerSide = 256 / kRegionSize;
constexpr int kRegions = kRegionsPerSide * kRegionsPerSide;

// The triangles of one batch, sorted by the region of the map they stand in
struct BatchRegions
{
    std::vector<uint32_t> tris; // first index of each triangle (into mesh.indices)
    uint32_t start[kRegions + 1] = {};
};

struct Pass
{
    GLuint program = 0;
    GLint clipLoc = -1;
    GLuint vertexBuffer = 0; // x y z u v per vertex
    GLuint colourBuffer = 0; // RGBA8 per vertex
    GLuint indexBuffer = 0;
    std::vector<BatchRegions> regions; // per batch
    std::vector<uint32_t> colours;
    std::vector<uint32_t> indices; // the frame's, gathered
    size_t vertexCount = 0;
    int triangles = 0;
    bool ready = false;
    bool failed = false;
};
Pass gPass;

int RegionOf(float x, float y)
{
    const int cx = std::clamp(static_cast<int>(x), 0, 255) / kRegionSize;
    const int cy = std::clamp(static_cast<int>(y), 0, 255) / kRegionSize;
    return cy * kRegionsPerSide + cx;
}

GLuint Compile(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, 0);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), 0, log);
        fprintf(stderr, "world3d: shader compile failed: %s\n", log);
        return 0;
    }
    return s;
}

bool CreateProgram()
{
    static const char* kVertex =
        "#version 120\n"
        "attribute vec3 aPos;\n"
        "attribute vec4 aColor;\n"
        "attribute vec2 aUV;\n"
        "uniform mat4 uClip;\n"
        "varying vec4 vColor;\n"
        "varying vec2 vUV;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = uClip * vec4(aPos, 1.0);\n"
        "    vColor = aColor;\n"
        "    vUV = aUV;\n"
        "}\n";
    static const char* kFragment =
        "#version 120\n"
        "uniform sampler2D uTex;\n"
        "varying vec4 vColor;\n"
        "varying vec2 vUV;\n"
        "void main()\n"
        "{\n"
        "    vec4 t = texture2D(uTex, vUV);\n"
        "    if (t.a * vColor.a < 0.5)\n" // palette colour 0 is transparent; a=0: the classic renderer's block
        "        discard;\n"
        "    gl_FragColor = vec4(t.rgb * vColor.rgb, 1.0);\n"
        "}\n";
    const GLuint v = Compile(GL_VERTEX_SHADER, kVertex);
    const GLuint f = Compile(GL_FRAGMENT_SHADER, kFragment);
    if (!v || !f)
    {
        return false;
    }
    gPass.program = glCreateProgram();
    glAttachShader(gPass.program, v);
    glAttachShader(gPass.program, f);
    glBindAttribLocation(gPass.program, 0, "aPos");
    glBindAttribLocation(gPass.program, 1, "aColor");
    glBindAttribLocation(gPass.program, 2, "aUV");
    glLinkProgram(gPass.program);
    GLint ok = 0;
    glGetProgramiv(gPass.program, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetProgramInfoLog(gPass.program, sizeof(log), 0, log);
        fprintf(stderr, "world3d: shader link failed: %s\n", log);
        return false;
    }
    gPass.clipLoc = glGetUniformLocation(gPass.program, "uClip");
    glUseProgram(gPass.program);
    glUniform1i(glGetUniformLocation(gPass.program, "uTex"), 0);
    return true;
}

bool EnsureReady()
{
    if (gPass.ready)
    {
        return true;
    }
    if (gPass.failed)
    {
        return false;
    }
    if (!CreateProgram())
    {
        gPass.failed = true;
        port_renderer_restore_state();
        return false;
    }
    glGenBuffers(1, &gPass.vertexBuffer);
    glGenBuffers(1, &gPass.colourBuffer);
    glGenBuffers(1, &gPass.indexBuffer);
    gPass.ready = true;
    port_renderer_restore_state();
    return true;
}

// The classic renderer's vertex colour (renderer.cpp ComputeLighting) for a face of grey level `shade`
struct LightSet
{
    std::vector<const PortLight*> lights;
    float ambient = 255;
    bool lit = false;
};

uint32_t VertexColour(const gta2::Vertex& v, uint8_t shade, const LightSet& ls)
{
    int r = shade, g = shade, b = shade;
    if (ls.lit)
    {
        float lr = 0, lg = 0, lb = 0;
        for (const PortLight* light : ls.lights)
        {
            const float dx = v.x - light->x, dy = v.y - light->y, dz = v.z - light->z;
            const float d2 = dx * dx + dy * dy + dz * dz;
            if (d2 > light->radiusSquared)
            {
                continue;
            }
            const float attenuation = (light->radius - std::sqrt(d2)) * light->radiusNormalized;
            if (attenuation > 0.0f)
            {
                const float brightness = attenuation * light->brightness;
                lr += light->r * brightness;
                lg += light->g * brightness;
                lb += light->b * brightness;
            }
        }
        const float n = shade / 255.0f;
        r = static_cast<int>(std::min(n * shade * lr + ls.ambient, 255.0f));
        g = static_cast<int>(std::min(n * shade * lg + ls.ambient, 255.0f));
        b = static_cast<int>(std::min(n * shade * lb + ls.ambient, 255.0f));
    }
    return static_cast<uint32_t>(r) | static_cast<uint32_t>(g) << 8 | static_cast<uint32_t>(b) << 16 | 0xFF000000u;
}
} // namespace

int WorldPassTriangles()
{
    return gPass.triangles;
}

void WorldPassUpload(const Scene& scene)
{
    if (!EnsureReady())
    {
        return;
    }
    const gta2::WorldMesh& mesh = scene.mesh;
    // Interleaved x y z u v
    std::vector<float> data;
    data.reserve(mesh.vertices.size() * 5);
    for (const gta2::Vertex& v : mesh.vertices)
    {
        data.insert(data.end(), {v.x, v.y, v.z, v.u, v.v});
    }
    gPass.vertexCount = mesh.vertices.size();
    gPass.colours.assign(gPass.vertexCount, 0);
    glBindBuffer(GL_ARRAY_BUFFER, gPass.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.size() * sizeof(float)), data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, gPass.colourBuffer);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(gPass.vertexCount * 4), nullptr, GL_DYNAMIC_DRAW);

    // Sort each batch's triangles by region
    gPass.regions.assign(mesh.batches.size(), BatchRegions());
    for (size_t b = 0; b < mesh.batches.size(); b++)
    {
        const gta2::TileBatch& batch = mesh.batches[b];
        BatchRegions& br = gPass.regions[b];
        const uint32_t triCount = batch.indexCount / 3;
        std::vector<uint32_t> regionOf(triCount);
        for (uint32_t t = 0; t < triCount; t++)
        {
            const uint32_t i = batch.indexStart + t * 3;
            const gta2::Vertex& a = mesh.vertices[mesh.indices[i]];
            const gta2::Vertex& c = mesh.vertices[mesh.indices[i + 2]];
            regionOf[t] = static_cast<uint32_t>(RegionOf((a.x + c.x) * 0.5f, (a.y + c.y) * 0.5f));
            br.start[regionOf[t] + 1]++;
        }
        for (int r = 0; r < kRegions; r++)
        {
            br.start[r + 1] += br.start[r];
        }
        br.tris.resize(triCount);
        uint32_t fill[kRegions];
        std::copy(br.start, br.start + kRegions, fill);
        for (uint32_t t = 0; t < triCount; t++)
        {
            br.tris[fill[regionOf[t]]++] = batch.indexStart + t * 3;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    port_renderer_restore_state();
}

bool WorldPassDraw(const Scene& scene)
{
    gPass.triangles = 0;
    if (!scene.valid || !EnsureReady() || gPass.regions.size() != scene.mesh.batches.size() ||
        gPass.vertexCount != scene.mesh.vertices.size())
    {
        return false;
    }
    const GameCamera cam = ReadCamera();
    if (!cam.valid || cam.f <= 0)
    {
        return false;
    }
    const gta2::WorldMesh& mesh = scene.mesh;

    // The regions the camera can see: the ground is the widest part of the view
    const float distance = cam.camZ + kEyeHeight;
    const float halfW = 320.0f / cam.f * distance + 1, halfH = 240.0f / cam.f * distance + 1;
    const int x0 = std::clamp(static_cast<int>(std::floor(cam.camX - halfW)), 0, 255) / kRegionSize;
    const int x1 = std::clamp(static_cast<int>(std::floor(cam.camX + halfW)), 0, 255) / kRegionSize;
    const int y0 = std::clamp(static_cast<int>(std::floor(cam.camY - halfH)), 0, 255) / kRegionSize;
    const int y1 = std::clamp(static_cast<int>(std::floor(cam.camY + halfH)), 0, 255) / kRegionSize;

    const Shades shades = ReadShades();
    LightSet ls;
    {
        const PortLight* all = nullptr;
        const int n = port_renderer_lights(&all, &ls.ambient);
        for (int i = 0; i < n; i++)
        {
            if ((all[i].flags & 0x30000) == 0x10000)
            {
                ls.lights.push_back(&all[i]);
            }
        }
        ls.lit = shades.lit && ls.ambient != 255.0f;
    }
    // One colour per face kind that doesn't depend on the position, when no light is on
    uint32_t flat[kShadeCount];
    for (int k = 0; k < kShadeCount; k++)
    {
        flat[k] = VertexColour(gta2::Vertex(), shades.level[k], LightSet());
    }

    struct Draw
    {
        unsigned texture;
        size_t first;
        size_t count;
    };
    std::vector<Draw> draws;
    gPass.indices.clear();
    for (size_t b = 0; b < mesh.batches.size(); b++)
    {
        const unsigned texture = TileTexture(mesh.batches[b].tile);
        if (!texture)
        {
            continue;
        }
        const BatchRegions& br = gPass.regions[b];
        const size_t first = gPass.indices.size();
        for (int ry = y0; ry <= y1; ry++)
        {
            for (int rx = x0; rx <= x1; rx++)
            {
                const int r = ry * kRegionsPerSide + rx;
                for (uint32_t t = br.start[r]; t < br.start[r + 1]; t++)
                {
                    const uint32_t i = br.tris[t];
                    for (int k = 0; k < 3; k++)
                    {
                        const uint32_t vi = mesh.indices[i + k];
                        gPass.indices.push_back(vi);
                        const uint8_t kind = scene.kind[vi];
                        gPass.colours[vi] = kind == kShadeClassic ? 0u
                                            : ls.lit          ? VertexColour(mesh.vertices[vi], shades.level[kind], ls)
                                                              : flat[kind];
                    }
                }
            }
        }
        if (gPass.indices.size() > first)
        {
            draws.push_back({texture, first, gPass.indices.size() - first});
        }
    }
    if (draws.empty())
    {
        return false;
    }

    float clip[16];
    ClipMatrix(cam, 640, 480, clip);
    glUseProgram(gPass.program);
    glUniformMatrix4fv(gPass.clipLoc, 1, GL_FALSE, clip);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glActiveTexture(GL_TEXTURE0);

    glBindBuffer(GL_ARRAY_BUFFER, gPass.vertexBuffer);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<const void*>(0));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<const void*>(3 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, gPass.colourBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(gPass.vertexCount * 4), gPass.colours.data());
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4, reinterpret_cast<const void*>(0));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gPass.indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(gPass.indices.size() * sizeof(uint32_t)),
                 gPass.indices.data(), GL_DYNAMIC_DRAW);
    for (const Draw& d : draws)
    {
        glBindTexture(GL_TEXTURE_2D, d.texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(d.count), GL_UNSIGNED_INT,
                       reinterpret_cast<const void*>(d.first * sizeof(uint32_t)));
        gPass.triangles += static_cast<int>(d.count / 3);
    }
    port_renderer_restore_state();
    return true;
}
} // namespace world3d
