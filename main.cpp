#include <iostream>
#include "VirtualDisk.h"

void readFile(INode inode){}

int main() {
    VirtualDisk vd("disk", 32768);
    FILE *disk = vd.createVirtualDisk();

    vd.saveSuperBlock(disk);
    vd.saveInodeList(disk);

    vd.sb.freeBlocks = 9;
    vd.sb.freeInodes = 9;
    vd.sb.firstInode = 9;
    vd.sb.blockSize = 11;

    for(auto &inode : vd.inode_arr){
        inode.size = 9;
        inode.linksCount = 9;
        inode.type = 9;
        for(auto &block : inode.blocks){
            block = 9;
        }
    }
    // Read superblock and inodes
    vd.readSuperBlock(disk);
    vd.readInodeList(disk);

    // Copy file
//    FILE *source = fopen("../dupa_500b","r");
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
