// The 3D renderer's settings: $XDG_CONFIG_HOME/gta2-native/graphics.txt (or ~/.config/...), one
// `key = value` per line, edited by the overlay's Graphics page too.
//
//   renderer = classic     classic: the game's own picture, as without world3d; world3d: the 3D renderer
//
// $GTA2_RENDERER overrides `renderer` for one run (for tests), without being saved.
#pragma once

#include <string>

namespace world3d
{
enum class Renderer
{
    Classic,
    World3d,
};

struct Settings
{
    Renderer renderer = Renderer::Classic;

    bool operator==(const Settings&) const = default;
};

const char* RendererName(Renderer renderer); // as written in the file
bool ParseRenderer(const char* name, Renderer& out);

std::string SettingsFile();
// Unknown lines and bad values are skipped, so a damaged file gives the defaults for what it lacks
Settings Load(const std::string& file);
// Written to a temporary file and renamed over the old one, so it is never left half written
bool Save(const std::string& file, const Settings& settings);

// The settings in force: the file, read on first use, with $GTA2_RENDERER applied
Settings& Current();
// Writes Current() to the file. $GTA2_RENDERER's value is not written, unless the player has chosen
// a renderer since (rendererChosen)
void SaveCurrent(bool rendererChosen = false);
} // namespace world3d
