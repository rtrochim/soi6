//
// Created by radek on 19.01.2020.
//

#ifndef SOI6_CPP_VIRTUALDISK_H
#define SOI6_CPP_VIRTUALDISK_H
#define BLOCK_SIZE 1024
#define FIRST_DATA_BLOCK_INDEX 2
#define INODE_BLOCK_INDEX 1
#define INODE_COUNT 10

#define ROOT_INODE_INDEX 0

#define REGULAR_FILE 1
#define DIRECTORY 2
#define SYMBOLIC_LINK 3

#include <string>
#include <utility>
#include <fstream>
#include <vector>
#include <array>

using namespace std;

struct superBlock {
    short blockSize = BLOCK_SIZE;
    short freeBlocks;
    short freeInodes = INODE_COUNT - 1;
    short firstInode = FIRST_DATA_BLOCK_INDEX - 1;
};

struct INode {
    short type = 0;
    short size = 0;
    short blocks[15] ={0};
    short linksCount = 0;
};

struct File {
    short inodeIndex;
    short nameLength;
    char name[60]; // Name of this file
};

class VirtualDisk {
public:
    VirtualDisk(string name, int size);
    void readSuperBlock();
    void readInodeList();
    void saveInodeList();
    void saveSuperBlock();
    void writeFileToDisk(FILE* source, string dstPath);

    superBlock sb;
    string name;
    int size;
    array<INode, INODE_COUNT> inode_arr;
    FILE *disk;

    vector<short> findFreeBlocks(short requiredBlocksCount);
};

#endif //SOI6_CPP_VIRTUALDISK_H




