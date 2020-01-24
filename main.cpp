#include <iostream>
#include "VirtualDisk.h"

int main() {
    VirtualDisk vd("disk", 32768);

    vd.saveSuperBlock();
    vd.saveInodeList();

    vd.readSuperBlock();
    vd.readInodeList();

    FILE* source = fopen("../dupa_2500b", "rb");
    vd.writeFileToDisk(source);

    // Copy file
//    FILE *source = fopen("../dupa_2500b","r");
//    vd.writeFileToDisk(source,disk);
//
//
//    FILE *target = fopen("./dupa_500b_from_disk","wb+");
//
//    char *buffer = (char *)malloc(500 * sizeof(char));
//    fseek(disk,2*BLOCK_SIZE,0);
//    fread(buffer, 500,1,disk);
//    fwrite(buffer,500,1,target);
    return 0;
}
