//
// Created by Radke on 19.01.2020.
//

#include "VirtualDisk.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <memory.h>

void VirtualDisk::readSuperBlock() {
    superBlock supblock;
    fseek(this->disk, 0, 0);
    fread(&supblock, sizeof(superBlock), 1, this->disk);
    this->sb = supblock;
}

void VirtualDisk::saveSuperBlock() {
    fseek(this->disk,0,0);
    fwrite(&this->sb, sizeof(this->sb), 1, this->disk);
}

void VirtualDisk::writeFileToDisk(FILE *source) {
    // Load file contents to memory
    long filelen;
    fseek(source, 0, SEEK_END);
    filelen = ftell(source);
    rewind(source);
    char *buffer = (char *)malloc(filelen * sizeof(char));
    fread(buffer, filelen, 1, source);
    fclose(source);

    // Find first free inode
    auto *freeInode = std::find_if(this->inode_arr.begin() + 1, this->inode_arr.end(), [&](const auto& inode) {
        return inode.linksCount == 0;
    });

    freeInode->size = static_cast<short>(filelen);
    freeInode->type = REGULAR_FILE;

    // Calculate required blocks
    short requiredBlocksCount = ceil(static_cast<float>(filelen) / BLOCK_SIZE);
    vector<short> blocksToWrite = findFreeBlocks(requiredBlocksCount);

    // Write file to disk block by block
    for(int i = 0; i < blocksToWrite.size(); i++) {
        char tempBuffer[BLOCK_SIZE];
        memcpy(tempBuffer, buffer, min(static_cast<long>(BLOCK_SIZE),filelen));
        fseek(this->disk, blocksToWrite[i] * BLOCK_SIZE ,0);
        fwrite(tempBuffer,1,min(static_cast<long>(BLOCK_SIZE),filelen),this->disk);
        filelen -= BLOCK_SIZE;
        freeInode->blocks[i] = blocksToWrite[i];
    }

    freeInode->linksCount += 1;
    saveInodeList();
    this->sb.freeInodes -= 1;
    this->sb.freeBlocks -= blocksToWrite.size();
    free(buffer);
}

void VirtualDisk::readInodeList() {
    auto *buffer = new INode[INODE_COUNT];
    fseek(this->disk, INODE_BLOCK_INDEX * BLOCK_SIZE, 0);
    fread(buffer, sizeof(INode), INODE_COUNT, this->disk);

    for (int i=0; i<INODE_COUNT; i++) {
        this->inode_arr[i] = buffer[i];
    }
    delete[] buffer;
}

void VirtualDisk::saveInodeList() {
    fseek(this->disk, BLOCK_SIZE * INODE_BLOCK_INDEX, 0);
    fwrite(&this->inode_arr, sizeof(INode), INODE_COUNT, this->disk);
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
    this->inode_arr[ROOT_INODE_INDEX].type = DIRECTORY;
    this->inode_arr[ROOT_INODE_INDEX].linksCount += 1;
    this->inode_arr[ROOT_INODE_INDEX].blocks[0] = FIRST_DATA_BLOCK_INDEX;
    char buffer[this->size];
    this->disk = fopen(this->name.c_str(), "w+");
    for (int i = 0; i < this->size; i++) {
        buffer[i] = '\0';
    }
    fwrite(buffer, 1, this->size, this->disk);
    std::cout << "Successfully created a virtual disk" << std::endl;
}
