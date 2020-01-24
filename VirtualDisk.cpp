//
// Created by radek on 19.01.2020.
//

#include "VirtualDisk.h"
#include <iostream>
#include <algorithm>
#include <cmath>

FILE *VirtualDisk::createVirtualDisk() {
    char buffer[this->size];
    FILE* disk = fopen("disk", "w+");
    for (int i = 0; i < this->size; i++) {
        buffer[i] = '\0';
    }
    fwrite(buffer, 1, this->size, disk);
    std::cout << "Successfully created a virtual disk" << std::endl;
    return disk;
}

void VirtualDisk::readSuperBlock(FILE *disk) {
    superBlock supblock;
    fseek(disk, 0, 0);
    fread(&supblock, sizeof(superBlock), 1, disk);
    this->sb = supblock;
}

void VirtualDisk::saveSuperBlock(FILE *disk){
    fseek(disk,0,0);
    fwrite(&this->sb, sizeof(this->sb), 1, disk);
}

void VirtualDisk::writeFileToDisk(FILE *source, FILE *disk) {
    long filelen;
    fseek(source, 0, SEEK_END);
    filelen = ftell(source);
    rewind(source);
    char *buffer = (char *)malloc(filelen * sizeof(char));
    fread(buffer, filelen, 1, source);
    fclose(source);
    auto *freeInode = std::find_if(this->inode_arr.begin(), this->inode_arr.end(), [&](const auto& inode) {
        return inode.linksCount == 0;
    });

    cout << "freeInode: " << freeInode->linksCount << endl;
    short blocksCount = ceil(filelen / BLOCK_SIZE);
    fseek(disk,2*BLOCK_SIZE,0);
    fwrite(buffer,filelen,1,disk);
    free(buffer);
    readInodeList(disk);
    for(auto inode : this->inode_arr){

    }
}

void VirtualDisk::readInodeList(FILE *disk) {
    INode *buffer = new INode[INODE_COUNT];
    fseek(disk, INODE_BLOCK_INDEX * BLOCK_SIZE, 0);
    fread(buffer, sizeof(INode), INODE_COUNT, disk);

    for (int i=0; i<INODE_COUNT; i++) {
        this->inode_arr[i] = buffer[i];
    }

    delete[] buffer;
}

void VirtualDisk::saveInodeList(FILE *disk){
    fseek(disk, BLOCK_SIZE * INODE_BLOCK_INDEX, 0);
    fwrite(&this->inode_arr, sizeof(INode), INODE_COUNT, disk);
}
