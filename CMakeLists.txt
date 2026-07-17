cmake_minimum_required(VERSION 3.31)
project(doomi-project)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(DOOM_SOURCES
    src/i_main.c
    src/doomdef.c
    src/doomstat.c
    src/dstrings.c
    src/i_system.c
    src/i_sound_stub.c
    src/i_video.c
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

set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

find_package(SDL3 CONFIG QUIET)

if(NOT SDL3_FOUND)
    include(FetchContent)
    message(STATUS "SDL3 not installed; using FetchContent")


    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.4.12
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL
    )

    FetchContent_MakeAvailable(SDL3)

else()
    message(STATUS "Using installed SDL3")
endif()

add_executable(doom ${DOOM_SOURCES})

target_include_directories(doom PRIVATE
    src
)

target_link_libraries(doom PRIVATE
    SDL3::SDL3
)

if(UNIX)
    target_compile_definitions(doom PRIVATE NORMALUNIX)
endif()