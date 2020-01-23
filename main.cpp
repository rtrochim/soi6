#include <iostream>
#include "VirtualDisk.h"

superBlock readSuperBlock(FILE* file){
    superBlock sb;
    fseek(file,0,0);
    fread(&sb,sizeof(superBlock),1,file);
    return sb;
}

void readInodeList(FILE* file, INode *buffer){
    fseek(file,BLOCK_SIZE,0);
    fread(buffer,sizeof(INode),INODE_COUNT,file);
}

void readFile(INode inode){}

void writeFileToDisk(FILE* source, FILE* disk){
    long filelen;
    fseek(source, 0, SEEK_END);
    filelen = ftell(source);
    rewind(source);
    char *buffer = (char *)malloc(filelen * sizeof(char));
    fread(buffer, filelen, 1, source);
    fclose(source);
    fseek(disk,2*BLOCK_SIZE,0);
    fwrite(buffer,filelen,1,disk);
    free(buffer);
}

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
    INode inode_arr[INODE_COUNT];
    sb = readSuperBlock(disk);
    readInodeList(disk, inode_arr);

// Copy file
    FILE *source = fopen("../dupa_500b","r");
    writeFileToDisk(source,disk);


    FILE *target = fopen("./dupa_500b_from_disk","wb+");

    char *buffer = (char *)malloc(500 * sizeof(char));
    fseek(disk,2*BLOCK_SIZE,0);
    fread(buffer, 500,1,disk);
    fwrite(buffer,500,1,target);
    return 0;
}
