cmake_minimum_required(VERSION 3.31)

project(doomi-project LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# libADLMIDI contains C++ source code.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(DOOM_SOURCES
    src/i_main.c
    src/doomdef.c
    src/doomstat.c
    src/dstrings.c
    src/i_system.c

    # raylib implementations
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

# ---------------------------------------------------------------------------
# raylib
# ---------------------------------------------------------------------------

set(RAYLIB_VERSION 6.0)

find_package(raylib ${RAYLIB_VERSION} CONFIG QUIET)

if(NOT raylib_FOUND)
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

# ---------------------------------------------------------------------------
# libADLMIDI
# ---------------------------------------------------------------------------

# Build only the static library needed by the game.
set(libADLMIDI_STATIC ON CACHE BOOL "" FORCE)
set(libADLMIDI_SHARED OFF CACHE BOOL "" FORCE)

# Required for loading and sequencing MIDI/MUS songs.
set(WITH_MIDI_SEQUENCER ON CACHE BOOL "" FORCE)
set(WITH_MUS_SUPPORT ON CACHE BOOL "" FORCE)

# Keep libADLMIDI's built-in instrument banks.
set(WITH_EMBEDDED_BANKS ON CACHE BOOL "" FORCE)

# Disable utilities, examples, tests and optional integrations.
set(WITH_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(WITH_MIDIPLAY OFF CACHE BOOL "" FORCE)
set(WITH_ADLMIDI2 OFF CACHE BOOL "" FORCE)
set(WITH_VLC_PLUGIN OFF CACHE BOOL "" FORCE)
set(WITH_WINMMDRV OFF CACHE BOOL "" FORCE)
set(WITH_OLD_UTILS OFF CACHE BOOL "" FORCE)
set(WITH_GENADLDATA OFF CACHE BOOL "" FORCE)
set(EXAMPLE_SDL2_AUDIO OFF CACHE BOOL "" FORCE)
set(WITH_HQ_RESAMPLER OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    libADLMIDI
    GIT_REPOSITORY https://github.com/Wohlstand/libADLMIDI.git
    GIT_TAG v1.6.1
    GIT_SHALLOW TRUE
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(libADLMIDI)

# ---------------------------------------------------------------------------
# Doom executable
# ---------------------------------------------------------------------------

add_executable(doom ${DOOM_SOURCES})

target_include_directories(doom PRIVATE
    src
)

target_link_libraries(doom PRIVATE
    raylib
    ADLMIDI_static
)

# The executable contains C source but links a C++ static library.
# This makes CMake invoke the C++ linker and include its runtime automatically.
set_target_properties(doom PROPERTIES
    LINKER_LANGUAGE CXX
)

if(UNIX)
    target_compile_definitions(doom PRIVATE NORMALUNIX)
endif()