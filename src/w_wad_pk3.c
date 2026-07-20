#include "w_wad.h"
#include "i_system.h"
#include "m_swap.h"
#include "miniz.h"

void *virtual_wads[MAX_VIRTUAL_WADS];
size_t resource_wad_size = 0;
int virtual_wad_index = RESOURCE_VIRTUAL_WAD + 1;

/// @brief Copies some data into virtual wad #0 and appends it to the lumpinfo and cache
/// @param Data Data
/// @param size Data size
void CopyDataToLump(
    char name[9],
    void *data,
    size_t data_size
) {
    if (data == NULL || data_size == 0) I_Error("Lump \"%s\" has no data", name);

    // resize res wad
    void *new_resource_array = realloc(
        virtual_wads[RESOURCE_VIRTUAL_WAD],
        resource_wad_size + data_size
    );

    if(!new_resource_array) I_Error("Couldn't realloc resource virtual WAD for lump \"%s\"", name);

    virtual_wads[RESOURCE_VIRTUAL_WAD] = new_resource_array;
    
    // copy
    memcpy(
        (byte *)new_resource_array + resource_wad_size,
        data,
        data_size
    );
    
    // lumps
    int startlump = numlumps;
    
    // lumpinfo
    numlumps++;
    lumpinfo_t *new_lumpinfo = realloc(
        lumpinfo,
        (size_t)numlumps * sizeof(*lumpinfo)
    );

    if (!new_lumpinfo)
        I_Error("Couldn't realloc lumpinfo for lump \"%s\" in virtual WAD", name);

    lumpinfo = new_lumpinfo;
    // lumpcache
    void **new_lumpcache = realloc(
        lumpcache,
        (size_t)numlumps * sizeof(*lumpcache)
    );
    
    if(!new_lumpcache) I_Error("Couldn't realloac lumpcache for virtual WAD");
    
    lumpcache = new_lumpcache;
    
    memset(
        &lumpcache[startlump],
        0,
        sizeof(*lumpcache)
    );
    lumpinfo_t *lump_i = &lumpinfo[startlump];
    lump_i->handle = RESOURCE_VIRTUAL_WAD;
    lump_i->position = resource_wad_size;
    lump_i->size = data_size;
    lump_i->virtual_origin = true;
    
    memset(lump_i->name, 0, 8);
    memcpy(lump_i->name, name, strlen(name) < 8 ? strlen(name) : 8);
    resource_wad_size += data_size;
}


/// @brief Adds a Map WAD to a virtual wad
/// @param name Lump name
/// @param data Map name
/// @param data_size Map data size
void AddMapToVirtualWad(
    char name[9],
    void *data,
    size_t data_size
) {
    wadinfo_t header;
    int lumplengths;
    filelump_t *lumps;
    int startlump = numlumps;

    if (data == NULL || data_size < sizeof(wadinfo_t)) I_Error("Archived WAD/Map \"%s\" has no data", name);
    if (memcmp(data, "IWAD", 4) != 0 &&
        memcmp(data, "PWAD", 4) != 0)
            I_Error("Archived WAD/Map \"%s\" has invalid header", name);
    
    if(virtual_wad_index >= MAX_VIRTUAL_WADS) I_Error("Too many Virtual WADS (%u/%u)", virtual_wad_index, MAX_VIRTUAL_WADS); 

    virtual_wads[virtual_wad_index] = malloc(data_size);
    memcpy(virtual_wads[virtual_wad_index], data, data_size);
    void *vdata = virtual_wads[virtual_wad_index];

    memcpy(&header, vdata, sizeof(header));

    header.numlumps = LONG(header.numlumps);
    header.infotableofs = LONG(header.infotableofs);

    lumplengths = header.numlumps * sizeof(filelump_t);
    lumps = (filelump_t *)((byte *)vdata + header.infotableofs);

    memcpy(
        lumps, 
        (byte*) vdata + header.infotableofs, 
        lumplengths
    );

    // add more space in lumpinfo
    numlumps += header.numlumps;
    lumpinfo = realloc(
        lumpinfo,
        numlumps * sizeof(lumpinfo_t)
    );

    if(!lumpinfo) I_Error("Couldn't realloc lumpinfo for virtual WAD");

    // new cache fsr
    void **new_lumpcache = realloc(
        lumpcache,
        (size_t)numlumps * sizeof(*lumpcache)
    );

    if(!new_lumpcache) I_Error("Couldn't realloac lumpcache for virtual WAD");

    lumpcache = new_lumpcache;

    memset(
        &lumpcache[startlump],
        0,
        (size_t)header.numlumps * sizeof(*lumpcache)
    );

    lumpinfo_t *lump_i = &lumpinfo[startlump];

    for(int i = startlump; i < numlumps; lump_i++, lumps++, i++) {
        lump_i->handle = virtual_wad_index;
        lump_i->position = LONG(lumps->filepos);
        lump_i->size = LONG(lumps->size);
        lump_i->virtual_origin = true;
        memcpy(lump_i->name, lumps->name, 8);
    }
    virtual_wad_index++;
}

const char *GetFirstFolder(
    const char *path,
    char *folder,
    size_t folder_size
)
{
    const char *separator;
    size_t length;

    if (path == NULL || folder == NULL || folder_size == 0)
        return NULL;

    folder[0] = '\0';

    separator = strchr(path, '/');

    /* No folder, only a filename. */
    if (separator == NULL || separator == path)
        return NULL;

    length = (size_t)(separator - path);

    if (length >= folder_size)
        length = folder_size - 1;

    memcpy(folder, path, length);
    folder[length] = '\0';

    return folder;
}

/// @brief Returns true if the extension less file is a common control
/// @param filename Extension less file name
/// @return True if usual control file
boolean IsControlFile(const char* filename) {
    if (strcmp(filename, "MAPINFO") == 0 ||
        strcmp(filename, "GAMEINFO") == 0 ||
        strcmp(filename, "DECORATE") == 0 ||
        strcmp(filename, "ZSCRIPT") == 0 ||
        strcmp(filename, "SNDINFO") == 0 ||
        strcmp(filename, "SNDSEQ") == 0 ||
        strcmp(filename, "ANIMDEFS") == 0 ||
        strcmp(filename, "GLDEFS") == 0 ||
        strcmp(filename, "LANGUAGE") == 0 ||
        strcmp(filename, "KEYCONF") == 0 ||
        strcmp(filename, "MENUDEF") == 0 ||
        strcmp(filename, "LOCKDEFS") == 0 ||
        strcmp(filename, "SBARINFO") == 0 ||
        strcmp(filename, "LOADACS") == 0 ||
        strcmp(filename, "CVARINFO") == 0 ||
        strcmp(filename, "TERRAIN") == 0 ||
        strcmp(filename, "MODELDEF") == 0 ||
        strcmp(filename, "TEXTURES") == 0 ||
        strcmp(filename, "VOXELDEF") == 0 ||
        strcmp(filename, "FONTDEFS") == 0 ||
        strcmp(filename, "DEHACKED") == 0)
    {
        return true;
    }

    return false;
}

/// @brief Loads a PK3 archive into memory.
/// This data can be later be accessed by normal ReadLump functions. 
/// @param filename 
void W_AddPK3(const char *filename) {
    mz_zip_archive zip;
    mz_uint file_count;

    memset(&zip, 0, sizeof(zip));

    if(!mz_zip_reader_init_file(&zip, filename, 0)) 
        I_Error("W_AddPK3: Failed to open %s", filename);
    
    file_count = mz_zip_reader_get_num_files(&zip);

    printf("Reading Archive: %s, file count: %u, status: ",filename, file_count);
    
    for(int i = 0; i < file_count; i++) {
        mz_zip_archive_file_stat stat;
        void *data;
        size_t data_size;

        if(!mz_zip_reader_file_stat(&zip, i, &stat)) {
            printf("FAIL\n");
            fprintf(stderr, "\tFailed to read entry: %u\n", i);
            continue;
        }

        if(mz_zip_reader_is_file_a_directory(&zip, i)) continue;

        data_size = 0;

        data = mz_zip_reader_extract_to_heap(
            &zip,
            i,
            &data_size,
            0
        );

        if(data == NULL) {
            printf("FAIL\n");
            fprintf(stderr, "\tFailed to extract: %s\n", stat.m_filename);
            continue;
        }

        const char* path = stat.m_filename;
        // extension
        const char* extension = strrchr(path, '.');
        if(extension == NULL) {
            if(IsControlFile(path)) {
                char lumpname[9];
                snprintf(lumpname, sizeof(lumpname), "%.*s", (int)(extension - (strrchr(path, '/') ? strrchr(path, '/') + 1 : path)), strrchr(path, '/') ? strrchr(path, '/') + 1 : path);
                CopyDataToLump(lumpname, data, data_size);
            }
            mz_free(data);
            continue;
        }

        // folder
        char folder[32];
        const char* result = GetFirstFolder(
            path,
            folder,
            sizeof(folder)
        );

        // lump name
        char lumpname[9];
        snprintf(lumpname, sizeof(lumpname), "%.*s", (int)(extension - (strrchr(path, '/') ? strrchr(path, '/') + 1 : path)), strrchr(path, '/') ? strrchr(path, '/') + 1 : path);

        if (strcmp(extension, ".wad") == 0 ||
            strcmp(extension, ".WAD") == 0)
            AddMapToVirtualWad(lumpname, data, data_size);

        mz_free(data);
    }
    printf("Before mz_zip_reader_end\n");

mz_bool result = mz_zip_reader_end(&zip);

printf("After mz_zip_reader_end, result=%d\n", result);
printf("OK\n");

}