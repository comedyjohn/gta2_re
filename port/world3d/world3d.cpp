// The hooks of include/port_world3d.h and the overlay's Graphics page. Phase 1 of port/world3d/PLAN.md: they keep track of what the
// game is drawing and hand every draw back to the classic renderer, so `renderer = world3d` draws
// exactly what `classic` does.
#include "port_world3d.h"
#include "settings.h"

#include "overlay.h"

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

bool port_world3d_draw(PortWorld3dDraw, int, STexture*, Vert*, int)
{
    return false; // phase 1: the classic renderer draws everything, whichever renderer is chosen
}

void port_world3d_present()
{
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
    return Current().renderer == Renderer::World3d ? "3D (in development: draws as classic)" : "Classic";
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
