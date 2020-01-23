//
// Created by radek on 19.01.2020.
//

#include "VirtualDisk.h"
#include <iostream>

FILE * VirtualDisk::createVirtualDisk() {
    ofstream wf(this->name,ios::out | ios::binary);
    char buffer[this->size];
    for (int i = 0; i < this->size; i++) {
        buffer[i] = '\0';
    }

    std::cout << "Successfully created a virtual disk" << std::endl;
    return fopen("disk", "w+");
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
    return inodes;
}

void VirtualDisk::setInodes(const vector<INode> &inodes) {
    VirtualDisk::inodes = inodes;
}

const superBlock &VirtualDisk::getSb() const {
    return sb;
}

void VirtualDisk::setSb(const superBlock &sb) {
    VirtualDisk::sb = sb;
}