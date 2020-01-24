//
// Created by radek on 19.01.2020.
//

#ifndef SOI6_CPP_VIRTUALDISK_H
#define SOI6_CPP_VIRTUALDISK_H
#define BLOCK_SIZE 1024
#define FIRST_DATA_BLOCK_INDEX 2
#define INODE_BLOCK_INDEX 1
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
    short freeInodes = INODE_COUNT;
    short firstInode = FIRST_DATA_BLOCK_INDEX - 1;
};

struct INode {
    short type = 0;
    short size = 0;
    short blocks[15] ={0};
    short linksCount = 0;
};

struct File {
    INode inode;
    string name;
};

struct DataBlock {
    char buffer[BLOCK_SIZE];
};

class VirtualDisk {
public:
    VirtualDisk(string name, int size) : name(std::move(name)), size(size) {};
    FILE *createVirtualDisk();
    void readSuperBlock(FILE* disk);
    void readInodeList(FILE* disk);
    void saveInodeList(FILE *disk);
    void saveSuperBlock(FILE *disk);
    void writeFileToDisk(FILE* source, FILE* disk);

    superBlock sb;
    string name;
    int size;
    array<INode, INODE_COUNT> inode_arr;

    vector<short> findFreeBlocks(short requiredBlocksCount);
};

#endif //SOI6_CPP_VIRTUALDISK_H




