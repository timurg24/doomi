# User Impacting
- Removed command line arguments:
	- `-cdrom`
- Adde command line arguments:
	- `-iwad` - Loads WAD file
- Keybinds:
	- Added support for WASD movement
- Removed features:
	- Music (will be added shortly)
	- Multiplayer

# Technical
- x64 changes as explained by [Diego Crespo](https://www.deusinmachina.net/p/lets-compile-linux-doom)
- Replaced X11 and SHM logic with Raylib
- Replaced audio library with Raylib
- Removed the requirement for certain textures as defined in `p_switch.c`
	- So you don't have to create all 32 textures as required by the shareware version
	- Not sure if this will cause WADs missing these textures to not work on other source ports
