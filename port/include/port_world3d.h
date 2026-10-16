// The 3D renderer (port/world3d/PLAN.md): what the game and the renderer tell it. Every call site is
// tagged "port: world3d", so `grep -rn "port: world3d" port/` lists them all (a tagged `if` comes
// with its braced block).
//
// Built without it (cmake -DGTA2_WORLD3D=OFF), these are empty inlines: the call sites stay and do
// nothing. Built with it, nothing changes either until the player picks the 3D renderer
// (renderer = world3d in graphics.txt, or the overlay's Graphics page): none of these may change
// what the game computes.
#pragma once

struct STexture;
struct Vert;
class Sprite;

// What a draw call of the game's renderer draws (port_world3d_draw)
enum PortWorld3dDraw
{
    kPortWorld3dTile, // gbh_DrawTile / gbh_DrawTilePart: a map face, UVs and flags as the game passed them
    kPortWorld3dQuad, // gbh_DrawQuad
    kPortWorld3dTriangle, // gbh_DrawTriangle
};

#ifdef GTA2_WORLD3D

// ---- from the game (render only)
// Game_0x40::Draw_4B92D0: the HUD (Hud_2B00::DrawGui_5D6860) is being drawn
void port_world3d_hud(bool drawing);
// Game_0x40::Draw_4B92D0: the score popups over the world (ExplodingScorePool) are being drawn
void port_world3d_world_text(bool drawing);
// SpriteTree_4::Draw_5C5DF0: the draws that follow are this sprite's, until it is called with null
void port_world3d_sprite(const Sprite* sprite);
// MapRenderer::RenderBlockAt_4F6880: the tiles that follow are the block at this cell (z as the
// renderer's raw gZCoord_6F63E0)
void port_world3d_block(int x, int y, unsigned int z);

// Map_0x370: the map's blocks changed (a block changed, added or removed, a saved game or a level loaded)
void port_world3d_map_changed();

// ---- from the renderer (platform/renderer.cpp)
// True when the 3D renderer took the draw; false: the classic renderer draws it
bool port_world3d_draw(PortWorld3dDraw kind, int flags, STexture* texture, Vert* verts, int colour);
// Once per presented frame, before the overlay is drawn over it
void port_world3d_present();

#else

inline void port_world3d_hud(bool) {}
inline void port_world3d_world_text(bool) {}
inline void port_world3d_sprite(const Sprite*) {}
inline void port_world3d_block(int, int, unsigned int) {}
inline void port_world3d_map_changed() {}
inline bool port_world3d_draw(PortWorld3dDraw, int, STexture*, Vert*, int) { return false; }
inline void port_world3d_present() {}

#endif
