# User Impacting
- Removed command line arguments:
	- `-cdrom`
- Adde command line arguments:
	- `-iwad` - Loads WAD file (inputed WAD name must be in all lower case)
	- `-pk3` - Loads a .pk3 archive
	- `-5` - window size multiplier
- Keybinds:
	- Added support for WASD movement
- Removed features:
	- Multiplayer
- Startup:
	- Removed need to press enter when playing modded games

# Technical
- x64 changes as explained by [Diego Crespo](https://www.deusinmachina.net/p/lets-compile-linux-doom)
- Replaced X11 and SHM logic with Raylib
- Replaced audio library with Raylib
- Removed the requirement for certain textures as defined in `p_switch.c`
	- So you don't have to create all 32 textures as required by the shareware version
	- Not sure if this will cause WADs missing these textures to not work on other source ports
