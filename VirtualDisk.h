//
// Created by radek on 19.01.2020.
//

#ifndef SOI6_CPP_VIRTUALDISK_H
#define SOI6_CPP_VIRTUALDISK_H
#define BLOCK_SIZE 1024
#define FIRST_DATA_BLOCK_INDEX 2
#define INODE_COUNT 10

#include <string>
#include <utility>
#include <fstream>
#include <vector>
#include <array>

using namespace std;

struct superBlock {
    short blockSize = BLOCK_SIZE;
    short freeBlocks = 0;
    short freeInodes = 0;
    short firstInode = FIRST_DATA_BLOCK_INDEX;
};

struct INode {
    short type;
    short size = 0;
    short blocks[15] ={0};
    short linksCount = 0;
};

struct File {
    INode inode;
    string name;
};

class VirtualDisk {
public:
    VirtualDisk(string name, int size) : name(std::move(name)), size(size) {};
    FILE *createVirtualDisk();
    void readSuperBlock(FILE* file);
    void readInodeList(FILE* file);
    void writeFileToDisk(FILE* source, FILE* disk);

    const string &getName() const;
    void setName(const string &name);
    int getSize() const;
    void setSize(int size);
    const vector<INode> &getInodes() const;
    const superBlock &getSb() const;
    void setSb(const superBlock &sb);

private:
    superBlock sb;
    string name;
    int size;
    array<INode, INODE_COUNT> inode_arr;
};

#endif //SOI6_CPP_VIRTUALDISK_H




