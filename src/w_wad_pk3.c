#include "w_wad.h"

/// @brief Alternative to W_AddFile() that loads a .pk3/.ipk3 file
/// @param filename Filename
void W_AddPK3(char *filename) {
    wadinfo_t   header;
    lumpinfo_t* lump_p;
    unsigned int i;
    int handle;
    int length;
    int startlump;
}