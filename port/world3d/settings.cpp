#include "settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

namespace world3d
{
namespace
{
const char* const kRendererNames[] = {"classic", "world3d"};

Settings gCurrent;
bool gLoaded = false;
bool gOverridden = false; // $GTA2_RENDERER set the renderer
Renderer gFileRenderer = Renderer::Classic; // what the file says, written back when overridden
Renderer gActive = Renderer::Classic; // the renderer of this run
} // namespace

const char* RendererName(Renderer renderer)
{
    return kRendererNames[static_cast<int>(renderer)];
}

bool ParseRenderer(const char* name, Renderer& out)
{
    for (int i = 0; i < 2; i++)
    {
        if (!strcmp(name, kRendererNames[i]))
        {
            out = static_cast<Renderer>(i);
            return true;
        }
    }
    return false;
}

std::string SettingsFile()
{
    const char* base = getenv("XDG_CONFIG_HOME");
    std::string dir = base && *base ? base : std::string(getenv("HOME") ? getenv("HOME") : ".") + "/.config";
    mkdir(dir.c_str(), 0755); // may not exist yet (a fresh $XDG_CONFIG_HOME)
    dir += "/gta2-native";
    mkdir(dir.c_str(), 0755);
    return dir + "/graphics.txt";
}

Settings Load(const std::string& file)
{
    Settings s;
    FILE* f = fopen(file.c_str(), "r");
    if (!f)
    {
        return s;
    }
    char line[256];
    while (fgets(line, sizeof(line), f))
    {
        char key[64];
        char value[64];
        if (line[0] == '#' || sscanf(line, " %63[^= ] = %63s", key, value) != 2)
        {
            continue;
        }
        if (!strcmp(key, "renderer"))
        {
            ParseRenderer(value, s.renderer);
        }
    }
    fclose(f);
    return s;
}

bool Save(const std::string& file, const Settings& settings)
{
    const std::string temp = file + ".new";
    FILE* f = fopen(temp.c_str(), "w");
    if (!f)
    {
        return false;
    }
    fprintf(f, "# GTA2 graphics settings (port/world3d/PLAN.md).\n"
               "# renderer: classic (the game's own picture) or world3d (the 3D renderer, in development)\n");
    fprintf(f, "renderer = %s\n", RendererName(settings.renderer));
    const bool ok = fclose(f) == 0;
    return ok && rename(temp.c_str(), file.c_str()) == 0;
}

Settings& Current()
{
    if (!gLoaded)
    {
        gLoaded = true;
        gCurrent = Load(SettingsFile());
        gFileRenderer = gCurrent.renderer;
        const char* env = getenv("GTA2_RENDERER");
        if (env && *env)
        {
            if (ParseRenderer(env, gCurrent.renderer))
            {
                gOverridden = true;
            }
            else
            {
                fprintf(stderr, "gta2: GTA2_RENDERER=%s is not a renderer (classic, world3d)\n", env);
            }
        }
        gActive = gCurrent.renderer;
    }
    return gCurrent;
}

Renderer ActiveRenderer()
{
    Current();
    return gActive;
}

void SaveCurrent(bool rendererChosen)
{
    Settings s = Current();
    if (rendererChosen)
    {
        gOverridden = false;
    }
    if (gOverridden)
    {
        s.renderer = gFileRenderer;
    }
    if (!Save(SettingsFile(), s))
    {
        fprintf(stderr, "gta2: could not save the graphics settings to %s\n", SettingsFile().c_str());
    }
}
} // namespace world3d
