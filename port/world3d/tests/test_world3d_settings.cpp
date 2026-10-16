// The 3D renderer's settings (port/world3d/settings.h) and its page in the overlay
#include <doctest.h>

#include "../settings.h"
#include "overlay.h"

#include <dinput.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace world3d;

TEST_CASE("graphics.txt keeps the renderer and survives damage")
{
    char dir[] = "/tmp/gta2-world3d-XXXXXX";
    REQUIRE(mkdtemp(dir));
    const std::string file = std::string(dir) + "/graphics.txt";

    // No file: the classic renderer
    Settings s = Load(file);
    CHECK(s.renderer == Renderer::Classic);

    s.renderer = Renderer::World3d;
    REQUIRE(Save(file, s));
    CHECK(Load(file) == s);

    // Junk lines and unknown values are skipped
    FILE* f = fopen(file.c_str(), "w");
    fputs("garbage\nrenderer = vulkan\nsomething = 1\n", f);
    fclose(f);
    CHECK(Load(file).renderer == Renderer::Classic);

    f = fopen(file.c_str(), "w");
    fputs("# comment\n  renderer=world3d\n", f);
    fclose(f);
    CHECK(Load(file).renderer == Renderer::World3d);

    unlink(file.c_str());
    rmdir(dir);
}

TEST_CASE("renderer names")
{
    Renderer r = Renderer::World3d;
    CHECK(ParseRenderer("classic", r));
    CHECK(r == Renderer::Classic);
    CHECK(ParseRenderer(RendererName(Renderer::World3d), r));
    CHECK(r == Renderer::World3d);
    CHECK_FALSE(ParseRenderer("World3d", r));
    CHECK_FALSE(ParseRenderer("", r));
}

TEST_CASE("the overlay offers the Graphics page and changes the renderer through it")
{
    const Renderer before = Current().renderer;
    REQUIRE_FALSE(overlay::Captured());
    overlay::OnKey(DIK_F2, true); // the settings screen
    overlay::OnKey(DIK_F2, false);
    REQUIRE(overlay::Captured());
    overlay::OnKey(DIK_G, true); // its G: the Graphics page
    overlay::OnKey(DIK_RIGHT, true); // the next renderer
    CHECK(Current().renderer != before);
    CHECK(Load(SettingsFile()).renderer == Current().renderer);
    overlay::OnKey(DIK_LEFT, true); // and back
    CHECK(Current().renderer == before);
    overlay::OnKey(DIK_ESCAPE, true);
    CHECK_FALSE(overlay::Captured());
}
