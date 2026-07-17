cmake_minimum_required(VERSION 3.31)

project(doomi-project LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(DOOM_SOURCES
    src/i_main.c
    src/doomdef.c
    src/doomstat.c
    src/dstrings.c
    src/i_system.c

    # Replace these with your raylib implementations:
    src/i_sound_raylib.c
    src/i_video_raylib.c

    src/i_net.c
    src/tables.c
    src/f_finale.c
    src/f_wipe.c
    src/d_main.c
    src/d_net.c
    src/d_items.c
    src/g_game.c
    src/m_menu.c
    src/m_misc.c
    src/m_argv.c
    src/m_bbox.c
    src/m_fixed.c
    src/m_swap.c
    src/m_cheat.c
    src/m_random.c
    src/am_map.c
    src/p_ceilng.c
    src/p_doors.c
    src/p_enemy.c
    src/p_floor.c
    src/p_inter.c
    src/p_lights.c
    src/p_map.c
    src/p_maputl.c
    src/p_plats.c
    src/p_pspr.c
    src/p_setup.c
    src/p_sight.c
    src/p_spec.c
    src/p_switch.c
    src/p_mobj.c
    src/p_telept.c
    src/p_tick.c
    src/p_saveg.c
    src/p_user.c
    src/r_bsp.c
    src/r_data.c
    src/r_draw.c
    src/r_main.c
    src/r_plane.c
    src/r_segs.c
    src/r_sky.c
    src/r_things.c
    src/w_wad.c
    src/wi_stuff.c
    src/v_video.c
    src/st_lib.c
    src/st_stuff.c
    src/hu_stuff.c
    src/hu_lib.c
    src/s_sound.c
    src/z_zone.c
    src/info.c
    src/sounds.c
)

set(RAYLIB_VERSION 6.0)

find_package(raylib ${RAYLIB_VERSION} CONFIG QUIET)

if(NOT raylib_FOUND)
    include(FetchContent)

    message(STATUS "raylib not installed; using FetchContent")

    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG ${RAYLIB_VERSION}
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL
    )

    FetchContent_MakeAvailable(raylib)
else()
    message(STATUS "Using installed raylib ${RAYLIB_VERSION}")
endif()

add_executable(doom ${DOOM_SOURCES})

target_include_directories(doom PRIVATE
    src
)

target_link_libraries(doom PRIVATE
    raylib
)

if(UNIX)
    target_compile_definitions(doom PRIVATE NORMALUNIX)
endif()