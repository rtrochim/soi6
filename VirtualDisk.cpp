//
// Created by Radke on 19.01.2020.
//

#include "VirtualDisk.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <utility>
#include <memory.h>

void VirtualDisk::readSuperBlock() {
    SuperBlock supblock;
    fseek(this->disk, 0, 0);
    fread(&supblock, sizeof(SuperBlock), 1, this->disk);
    this->sb = supblock;
}

void VirtualDisk::saveSuperBlock() {
    fseek(this->disk,0,0);
    fwrite(&this->sb, sizeof(this->sb), 1, this->disk);
}

void VirtualDisk::writeFileToDisk(string srcPath, string dstPath) {
    FILE* source = fopen(srcPath.c_str(), "rb");

    // Load file contents to memory
    long filelen;
    fseek(source, 0, SEEK_END);
    filelen = ftell(source);
    rewind(source);
    char *buffer = (char *)malloc(filelen * sizeof(char));
    fread(buffer, filelen, 1, source);
    fclose(source);

    // Find first free inode
    readInodeList();

    INode freeInode;
    int freeInodeIndex = -1;

    for (int i=1; i<this->inode_arr.size(); i++) {
        if (this->inode_arr[i].linksCount == 0) {
            freeInode = this->inode_arr[i];
            freeInodeIndex = i;
            break;
        }
    }

    if (freeInodeIndex == -1) {
        cout << "Not enough space to write this file" << endl;
        return;
    }

    freeInode.size = static_cast<short>(filelen);
    freeInode.type = REGULAR_FILE;

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
        freeInode.blocks[i] = blocksToWrite[i];
    }


    // Split path by '/'
    vector<string> entriesToTraverse = splitPath(std::move(dstPath));


    // Add written file entry to directory
    // TODO IMPLEMENT FIRST FREE DATA BLOCK FINDING FOR FILE ENTRY STORAGE IN DIRECTORY
    short blockIndex = this->inode_arr[ROOT_INODE_INDEX].blocks[0];

    // TODO CALCULATE OFFSET FOR CURRENT FILE ENTRY
    short blockOffset = 0;

    FileEntry f{};
    string filename = splitPath(srcPath).back();
    filename.copy(f.name,filename.length());
    f.inodeIndex = static_cast<short>(freeInodeIndex);
    // TODO CALCULATE REQUIRED LENGTH
    f.recLength = BLOCK_SIZE;

    // Write file entry to data block
    fseek(this->disk, blockIndex * BLOCK_SIZE + blockOffset,0);
    fwrite(&f, sizeof(f),1, this->disk);

    // Update inode array
    freeInode.linksCount += 1;
    this->inode_arr[freeInodeIndex] = freeInode;
    saveInodeList();
    this->sb.freeInodes -= 1;
    this->sb.freeBlocks -= blocksToWrite.size();
    saveSuperBlock();
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
    readInodeList();
    for (auto &inode : this->inode_arr) {
        if (inode.linksCount != 0) {
            for (auto &block : inode.blocks) {
                if (block) {
                    busyBlocks.push_back(block);
                }
            }
        }
    }

    short i = FIRST_DATA_BLOCK_INDEX + 1;
    while (blocksToWrite.size() < requiredBlocksCount) {
        if (std::find(busyBlocks.begin(), busyBlocks.end(), i) == busyBlocks.end()) {
            blocksToWrite.push_back(i);
        }
        i++;
    }
    return blocksToWrite;
}

vector<string> VirtualDisk::splitPath(string path) {
    vector<string> result;
    while(path.length()) {
        short index = path.find_first_of('/');
        if (index == -1) {
            result.push_back(path);
            break;
        }

        result.push_back(path.substr(0, index+1));
        path = path.substr(index + 1,path.length());
    }

    return result;
}

VirtualDisk::VirtualDisk(string name, int size) : name(std::move(name)), size(size){
    this->sb.freeBlocks = floor(static_cast<float>(size)/BLOCK_SIZE) - 2;
    this->inode_arr[ROOT_INODE_INDEX].type = DIRECTORY;
    this->inode_arr[ROOT_INODE_INDEX].linksCount += 1;
    this->inode_arr[ROOT_INODE_INDEX].blocks[0] = FIRST_DATA_BLOCK_INDEX;
    // Overwrite whole disk with zeros for debugging purpose
    char buffer[this->size];
    this->disk = fopen(this->name.c_str(), "w+");
    for (int i = 0; i < this->size; i++) {
        buffer[i] = '\0';
    }
    fwrite(buffer, 1, this->size, this->disk);
    this->saveSuperBlock();
    this->saveInodeList();
    std::cout << "Successfully created a virtual disk" << std::endl;
}

void VirtualDisk::copyFileFromDisk(string srcPath, string dstPath) {
    readInodeList();
    vector<string> entriesToTraverse = splitPath(srcPath);
    vector<FileEntry> fileEntries;
    for(auto entry : entriesToTraverse) {
        if(entry == "/"){
            for(auto block : this->inode_arr[ROOT_INODE_INDEX].blocks){
                if(block){
                    // Read root file entries
                    fileEntries = readFileEntriesFromBlock(block);
                    // Find inode of next file entry
                    auto next = find_if(fileEntries.begin(),fileEntries.end(), [&](const FileEntry &fileEntry){
                        return string(fileEntry.name) == entriesToTraverse[&entry - &entriesToTraverse[0]+1];
                    });
                    INode nextInode = this->inode_arr[next->inodeIndex];
                    int length = this->inode_arr[next->inodeIndex].size;
                    int i = 0;
                    // Our next inode is our file
                    if(nextInode.type == REGULAR_FILE){
                        // Read it block by block
                        char buffer[this->inode_arr[next->inodeIndex].size];
                        for(auto dataBlock : this->inode_arr[next->inodeIndex].blocks){
                            if(dataBlock){
                                char tempBuffer[BLOCK_SIZE];
                                fseek(this->disk, dataBlock * BLOCK_SIZE, 0);
                                fread(tempBuffer,  min(length, BLOCK_SIZE), 1, this->disk);
                                memcpy(buffer + i * BLOCK_SIZE, tempBuffer, min(length, BLOCK_SIZE));
                                length -= BLOCK_SIZE;
                                i++;
                            }
                        }
                        // Write to output file
                        FILE *dst = fopen(dstPath.c_str(),"wb");
                        fwrite(buffer,sizeof(buffer),1,dst);
                        fclose(dst);
                        return;
                    } else if(nextInode.type == DIRECTORY) {
//                        copyFileFromDisk()
                    }
                }
            }
        }
    }
}

vector<FileEntry> VirtualDisk::readFileEntriesFromBlock(short blockIndex){
    vector<FileEntry> result;
    int offset = 0; // Keep track of how close to block end we are
    FileEntry buffer;
    fseek(this->disk,blockIndex * BLOCK_SIZE, 0);

    while(offset != BLOCK_SIZE){
        fread(&buffer, sizeof(FileEntry),1,this->disk);
        result.push_back(buffer);
        offset += buffer.recLength;
    }

    return result;
}