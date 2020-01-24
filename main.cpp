#include <iostream>
#include "VirtualDisk.h"

void readFile(INode inode){}

int main() {
    VirtualDisk vd("disk", 32768);
    FILE *disk = vd.createVirtualDisk();

    //Save superblock
    superBlock sb;
    fwrite(&sb, BLOCK_SIZE, 1, disk);

    //Save inodes
    INode inodes[INODE_COUNT];
    fwrite(inodes, sizeof(INode), INODE_COUNT, disk);

    // Read superblock and inodes
    vd.readSuperBlock(disk);
    vd.readInodeList(disk);

    // Copy file
    FILE *source = fopen("../dupa_500b","r");
    vd.writeFileToDisk(source,disk);


    FILE *target = fopen("./dupa_500b_from_disk","wb+");

    char *buffer = (char *)malloc(500 * sizeof(char));
    fseek(disk,2*BLOCK_SIZE,0);
    fread(buffer, 500,1,disk);
    fwrite(buffer,500,1,target);
    return 0;
}
