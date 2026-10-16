# The 3D renderer (PLAN.md). Built when GTA2_WORLD3D is on (port/CMakeLists.txt); without it, the hooks
# in include/port_world3d.h are empty inlines and nothing here is compiled.
add_library(gta2_world3d STATIC
    settings.cpp
    world3d.cpp
    game_view.cpp
    upstream/gta2_map.cpp
    upstream/gta2_style.cpp
    upstream/world_mesh.cpp
    upstream/ground.cpp)
set_source_files_properties(upstream/gta2_map.cpp upstream/gta2_style.cpp upstream/world_mesh.cpp upstream/ground.cpp
    PROPERTIES COMPILE_OPTIONS "-w")
gta2_game_target(gta2_world3d -Wall -Wextra)
target_include_directories(gta2_world3d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../platform)
target_include_directories(gta2_world3d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/upstream)
target_link_libraries(gta2_world3d PUBLIC gta2_game)

file(GLOB world3d_test_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
set(WORLD3D_TEST_SOURCES ${world3d_test_sources} PARENT_SCOPE)
# the game headers it includes have unused parameters (-Wextra)
set_source_files_properties(game_view.cpp PROPERTIES COMPILE_OPTIONS "-Wno-unused-parameter")
