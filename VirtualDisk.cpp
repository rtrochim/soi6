//
// Created by radek on 19.01.2020.
//

#include "VirtualDisk.h"
#include <iostream>

FILE *VirtualDisk::createVirtualDisk() {
    ofstream wf(this->name,ios::out | ios::binary);
    char buffer[this->size];
    for (int i = 0; i < this->size; i++) {
        buffer[i] = '\0';
    }

    std::cout << "Successfully created a virtual disk" << std::endl;
    return fopen("disk", "w+");
}

void VirtualDisk::readSuperBlock(FILE *file) {
    superBlock sb;
    fseek(file,0,0);
    fread(&sb,sizeof(superBlock),1,file);
    this->sb = sb;
}

void VirtualDisk::writeFileToDisk(FILE *source, FILE *disk) {
    long filelen;
    fseek(source, 0, SEEK_END);
    filelen = ftell(source);
    rewind(source);
    char *buffer = (char *)malloc(filelen * sizeof(char));
    fread(buffer, filelen, 1, source);
    fclose(source);
    auto *freeInode = std::find(this->inode_arr.begin(), this->inode_arr.end(), [&](const auto& inode) {
        return inode.linksCount == 0;
    });

    cout << "freeInode: " << freeInode->linksCount << endl;
    fseek(disk,2*BLOCK_SIZE,0);
    fwrite(buffer,filelen,1,disk);
    free(buffer);
}

void VirtualDisk::readInodeList(FILE *file) {
    fseek(file, BLOCK_SIZE, 0);
    INode *buffer;
    std::copy(buffer, this->inode_arr.begin(), this->inode_arr.end());
    fread(buffer, sizeof(INode), INODE_COUNT, file);
}

const string &VirtualDisk::getName() const {
    return name;
}

void VirtualDisk::setName(const string &name) {
    VirtualDisk::name = name;
}

int VirtualDisk::getSize() const {
    return size;
}

void VirtualDisk::setSize(int size) {
    VirtualDisk::size = size;
}

const vector<INode> &VirtualDisk::getInodes() const {
    return std::vector<INode>();
}

const superBlock &VirtualDisk::getSb() const {
    return sb;
}

void VirtualDisk::setSb(const superBlock &sb) {
    VirtualDisk::sb = sb;
}