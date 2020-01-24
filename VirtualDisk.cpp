//
// Created by radek on 19.01.2020.
//

#include "VirtualDisk.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <memory.h>

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
    // Load file contents to memory
    long filelen;
    fseek(source, 0, SEEK_END);
    filelen = ftell(source);
    rewind(source);
    char *buffer = (char *)malloc(filelen * sizeof(char));
    fread(buffer, filelen, 1, source);
    fclose(source);

    // Find free inode
    auto *freeInode = std::find_if(this->inode_arr.begin() + 1, this->inode_arr.end(), [&](const auto& inode) {
        return inode.linksCount == 0;
    });

    freeInode->size = static_cast<short>(filelen);
    freeInode->type = REGULAR_FILE;

    short requiredBlocksCount = ceil(static_cast<float>(filelen) / BLOCK_SIZE);
    vector<short> blocksToWrite = findFreeBlocks(requiredBlocksCount);

    for(int i = 0; i < blocksToWrite.size(); i++) {
        char tempBuffer[BLOCK_SIZE];
        memcpy(tempBuffer, buffer, min(static_cast<long>(BLOCK_SIZE),filelen));
        fseek(disk, blocksToWrite[i] * BLOCK_SIZE ,0);
        fwrite(tempBuffer,1,min(static_cast<long>(BLOCK_SIZE),filelen),disk);
        filelen -= BLOCK_SIZE;
        freeInode->blocks[i] = blocksToWrite[i];
    }

    freeInode->linksCount += 1;
    saveInodeList(disk);
    this->sb.freeInodes -= 1;
    free(buffer);
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

vector<short> VirtualDisk::findFreeBlocks(short requiredBlocksCount){
    std::vector<short> busyBlocks;
    std::vector<short> blocksToWrite;
    for (auto &inode : this->inode_arr) {
        if (inode.linksCount != 0) {
            for (auto &block : inode.blocks) {
                if (block) {
                    busyBlocks.push_back(block);
                }
            }
        }
    }

    short i = FIRST_DATA_BLOCK_INDEX;
    while (blocksToWrite.size() < requiredBlocksCount) {
        if (std::find(busyBlocks.begin(), busyBlocks.end(), i) == busyBlocks.end()) {
            blocksToWrite.push_back(i);
        }
        i++;
    }
    return blocksToWrite;
}

VirtualDisk::VirtualDisk(string name, int size) : name(std::move(name)), size(size){
    this->sb.freeBlocks = floor(static_cast<float>(size)/BLOCK_SIZE);
}
