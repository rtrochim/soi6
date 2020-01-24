#include <iostream>
#include "VirtualDisk.h"

void readFile(INode inode){}

int main() {
    VirtualDisk vd("disk", 32768);
    FILE *disk = vd.createVirtualDisk();

    vd.saveSuperBlock(disk);
    vd.saveInodeList(disk);

    vd.readSuperBlock(disk);
    vd.readInodeList(disk);
    FILE* source = fopen("../dupa_2500b", "rb");
    vd.writeFileToDisk(source,disk);

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
