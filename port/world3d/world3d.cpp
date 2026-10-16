// The hooks of include/port_world3d.h and the overlay's Graphics page. Phase 1 of port/world3d/PLAN.md: they keep track of what the
// game is drawing and hand every draw back to the classic renderer, so `renderer = world3d` draws
// exactly what `classic` does.
#include "port_world3d.h"
#include "camera.h"
#include "camera_check.h"
#include "game_view.h"
#include "gl/world_pass.h"
#include "scene.h"
#include "settings.h"

#include "overlay.h"
#include <cstdlib>

namespace world3d
{
namespace
{
// What the game is drawing right now, from its hooks
struct DrawContext
{
    bool hud = false;
    bool worldText = false;
    const Sprite* sprite = nullptr;
    int blockX = -1;
    int blockY = -1;
    unsigned int blockZ = 0;
};
DrawContext gContext;
Scene gScene;
bool gMapDirty = true; // the game's map changed since the scene was last built
bool gWorldTried = false; // this frame's world pass was attempted
bool gWorldDrawn = false; // ... and drew

// Brings the scene up to date with the game's map, and the GL copy of it
void SyncScene()
{
    if (UpdateScene(gScene, gMapDirty))
    {
        WorldPassUpload(gScene);
    }
    gMapDirty = false;
}

enum SettingRow
{
    RowRenderer,
    kRows
};
} // namespace
} // namespace world3d

using namespace world3d;

void port_world3d_hud(bool drawing)
{
    gContext.hud = drawing;
}

void port_world3d_world_text(bool drawing)
{
    gContext.worldText = drawing;
}

void port_world3d_sprite(const Sprite* sprite)
{
    gContext.sprite = sprite;
}

void port_world3d_block(int x, int y, unsigned int z)
{
    gContext.blockX = x;
    gContext.blockY = y;
    gContext.blockZ = z;
}

void port_world3d_map_changed()
{
    gMapDirty = true;
}

bool port_world3d_draw(PortWorld3dDraw kind, int, STexture*, Vert* verts, int)
{
    if (ActiveRenderer() != Renderer::World3d || kind != kPortWorld3dTile || gContext.hud || gContext.sprite ||
        gContext.worldText)
    {
        return false;
    }
    CheckCamera(verts, BlockSlopeType(gContext.blockX, gContext.blockY, static_cast<int>(gContext.blockZ)));
    if (!gWorldTried)
    {
        // The first map tile of the frame: the lights are in and the whole world can be drawn
        gWorldTried = true;
        SyncScene();
        gWorldDrawn = WorldPassDraw(gScene);
    }
    if (!gWorldDrawn)
    {
        return false;
    }
    // Blocks the mesh draws are skipped by the classic renderer; the others (triangular sides, partial
    // blocks: slope types 49..61) it keeps drawing, over the world
    const int slope = BlockSlopeType(gContext.blockX, gContext.blockY, static_cast<int>(gContext.blockZ));
    return slope >= 0 && !(slope >= 49 && slope <= 61);
}

void port_world3d_present()
{
    if (ActiveRenderer() == Renderer::World3d)
    {
        SyncScene();
    }
    gWorldTried = false;
    gWorldDrawn = false;
    gContext = DrawContext();
}

namespace
{
// The Graphics page of the overlay's pause menu (platform/overlay.h)
int PageRows()
{
    return kRows;
}

const char* PageName(int row)
{
    return row == RowRenderer ? "Renderer" : "";
}

const char* PageValue(int row)
{
    if (row != RowRenderer)
    {
        return "";
    }
    // The renderer is chosen for the next start; this run keeps the one it started with
    const Renderer chosen = Current().renderer;
    if (chosen != ActiveRenderer())
    {
        return chosen == Renderer::World3d ? "3D (after a restart)" : "Classic (after a restart)";
    }
    return chosen == Renderer::World3d ? "3D (in development: draws as classic)" : "Classic";
}

void PageChange(int row, int)
{
    if (row == RowRenderer)
    {
        Settings& s = Current();
        s.renderer = s.renderer == Renderer::World3d ? Renderer::Classic : Renderer::World3d;
        SaveCurrent(true);
    }
}

const overlay::SettingsPage kPage = {"Graphics", "graphics.txt", PageRows, PageName, PageValue, PageChange};

// Added when the program starts: this file is linked whenever the game is (it calls the hooks above)
[[maybe_unused]] const bool kPageAdded = (overlay::AddSettingsPage(&kPage), true);
} // namespace
