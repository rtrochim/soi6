//
// Created by Radke on 19.01.2020.
//

#include "VirtualDisk.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <utility>
#include <memory.h>
#include <numeric>

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

void VirtualDisk::writeFileToDisk(const string& srcPath, string dstPath) {
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
        memcpy(tempBuffer,buffer + i * BLOCK_SIZE, min(static_cast<long>(BLOCK_SIZE),filelen));
        fseek(this->disk, blocksToWrite[i] * BLOCK_SIZE ,0);
        fwrite(tempBuffer,1,min(static_cast<long>(BLOCK_SIZE),filelen),this->disk);
        filelen -= BLOCK_SIZE;
        freeInode.blocks[i] = blocksToWrite[i];
    }

    // Split path by '/'
    vector<string> entriesToTraverse = splitPath(dstPath);
    int parentInodeIndex = getInodeIndexForFile(dstPath);
    // Read existing file entries
    vector<FileEntry> fileEntries = readFileEntriesForInode(parentInodeIndex);

    // Add new for our file
    FileEntry f{};
    string filename = splitPath(srcPath).back();
    filename.copy(f.name,filename.length());
    f.inodeIndex = static_cast<short>(freeInodeIndex);
    if(fileEntries.empty()){
        f.recLength = BLOCK_SIZE;
    } else {
        fileEntries.back().recLength = 64;
        short sum = accumulate(begin(fileEntries), end(fileEntries), 0, [&](short acc, const FileEntry &fe){
            return acc + fe.recLength;
        });
        sum %= BLOCK_SIZE;
        f.recLength = BLOCK_SIZE - sum;
    }
    fileEntries.push_back(f);

    this->inode_arr[parentInodeIndex].size += 64;

    // Update inode array
    freeInode.linksCount += 1;
    this->inode_arr[freeInodeIndex] = freeInode;
    saveInodeList();
    this->sb.freeInodes -= 1;
    this->sb.freeBlocks -= blocksToWrite.size();
    saveSuperBlock();
    readInodeList();
    writeFileEntriesForInode(this->inode_arr[parentInodeIndex], fileEntries);
    saveInodeList();
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
    saveInodeList();
    return blocksToWrite;
}

vector<string> VirtualDisk::splitPath(string path, bool frontSlash) {
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

    if(frontSlash){
        return result;
    } else {
        if(result[0][0] == '/'){
            result.erase(result.begin());
        }
        return result;
    }
}

void VirtualDisk::copyFileFromDisk(const string& srcPath, const string& dstPath) {
    readInodeList();
    int inodeIndex = getInodeIndexForFile(srcPath);
    char buffer[this->inode_arr[inodeIndex].size];
    int length = this->inode_arr[inodeIndex].size;
    int i = 0;

    // Load all file contents to memory
    for(auto dataBlock : this->inode_arr[inodeIndex].blocks){
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
}

vector<FileEntry> VirtualDisk::readFileEntriesFromBlock(short blockIndex){
    vector<FileEntry> result;
    int offset = 0; // Keep track of how close to block end we are
    FileEntry buffer;
    fseek(this->disk,blockIndex * BLOCK_SIZE, 0);
    int alarmCounter = 0;

    // TODO: infinite loop if no entries found
    while(offset < BLOCK_SIZE){
        fread(&buffer, sizeof(FileEntry),1,this->disk);
        result.push_back(buffer);
        offset += buffer.recLength;
        alarmCounter++;
        if (alarmCounter > 20) {
            result.clear();
            break;
        }
    }
    return result;
}

vector<FileEntry> VirtualDisk::readFileEntriesForInode(short inodeIndex){
    vector<FileEntry> result;
    for(auto block:this->inode_arr[inodeIndex].blocks){
        if(block){
            auto temp = readFileEntriesFromBlock(block);
            result.insert(result.end(), temp.begin(), temp.end());
        }
    }
    return result;
}

void VirtualDisk::writeFileEntriesForInode(INode &inode, vector<FileEntry> &fileEntries) {
    int entriesCounter = 0;
    for (auto block : inode.blocks){
        if (block) {
            auto offset = block*BLOCK_SIZE;
            if(fileEntries.empty()){
                break;
            }
            for (auto entry : fileEntries){
                fseek(this->disk,offset,0);
                fwrite(&entry,sizeof(entry),1,this->disk);
                offset += sizeof(entry);
                entriesCounter++;
                if (entriesCounter == 15) {
                    entriesCounter = 0;
                    break;
                }
            }
            // Write at most 16 entries per block
            int index = min(static_cast<int>(fileEntries.size()), 15);
            fileEntries.erase(fileEntries.begin(), fileEntries.begin() + index);
        }
    }
    // If there are more entries to be written, allocate additional blocks
    if(fileEntries.size() != 0){
        short blockIndex = findFreeBlocks(1)[0];
        for(auto &block : inode.blocks){
            if(!block){
                block = blockIndex;
                writeFileEntriesForInode(inode,fileEntries);
                return;
            }
        }
    }
}
// This resolves path name to inode number in inode array
int VirtualDisk::getInodeIndexForFile(string path, short inodeIndex) {
    // Root inode is always 0
    if(path == "/"){
        return ROOT_INODE_INDEX;
    }
    vector<string> entriesToTraverse = splitPath(path, false);
    // Remove trailing slash in entry name
    for(auto &entry : entriesToTraverse){
        if(entry.size() != 1 && entry[entry.size() - 1] == '/'){
            entry = entry.substr(0,entry.size() -1);
        }
    }
    vector<FileEntry> fileEntries;
    for (auto block : this->inode_arr[inodeIndex].blocks){
        if(block && this->inode_arr[inodeIndex].size) {
            // Read all existing file entries
            fileEntries = readFileEntriesFromBlock(block);
            // Find inode of next file entry
            auto next = find_if(fileEntries.begin(),fileEntries.end(), [entriesToTraverse](const FileEntry fileEntry){
                return string(fileEntry.name) == string(entriesToTraverse[0]);
            });
            // Found
            if(next != fileEntries.end()){
                INode nextInode = this->inode_arr[next->inodeIndex];
                // Our next inode is our file
                if(nextInode.type == REGULAR_FILE){
                    return next->inodeIndex;
                // Our next inode is a directory
                } else if(nextInode.type == DIRECTORY) {
                    // If this directory is empty, return its inode
                    if(readFileEntriesForInode(next->inodeIndex).empty()){
                        return next->inodeIndex;
                    }
                    entriesToTraverse.erase(entriesToTraverse.begin());
                    // If there are no more entries to traverse, also return
                    if(entriesToTraverse.empty()){
                        return next->inodeIndex;
                    }
                    // Otherwise, call yourself with remaining path to traverse
                    return getInodeIndexForFile(
                            // Concatenate remaining path
                            std::accumulate(entriesToTraverse.begin(), entriesToTraverse.end(), std::string(""), [&](string acc, const string &entr) {
                                string tmp = string(acc + entr);
                                vector<string>::iterator it = find(entriesToTraverse.begin(), entriesToTraverse.end(), entr);
                                if (distance(entriesToTraverse.begin(), it) != entriesToTraverse.size() - 1) {
                                    tmp.append("/");
                                }

                                return tmp;
                            }), next->inodeIndex);
                }
            }
        }
    }
    return -1;
}

void VirtualDisk::createDirectory(string path) {
    readInodeList();
    vector<string> entriesToTraverse = splitPath(path);
    string acc = "";
    short previousInodeIndex = 0;

    for (auto &entry : entriesToTraverse) {
        readInodeList();
        if (entry == "/") {
            acc = "/";
            continue;
        }

        string tempPath = entry.substr(0,entry.find_last_of('/'));
        short inodeIndex = getInodeIndexForFile(tempPath, previousInodeIndex);
        if (inodeIndex == -1) {
            // create directory with path 'acc + entry'
            string parentPath = acc;
            short parentInodeIndex = previousInodeIndex;
            vector<FileEntry> entries;
            if(this->inode_arr[parentInodeIndex].size){
               entries = readFileEntriesForInode(parentInodeIndex);
            }

            FileEntry newEntry{};
            string newName = entry.substr(0,entry.find_last_of('/'));
            strcpy(newEntry.name, newName.c_str()); // Copy directory name
            // Assign a new INode
            // Find first free inode

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
                cout << "Not enough space to create a new directory" << endl;
                return;
            }

            freeInode.size = 0;
            freeInode.type = DIRECTORY;
            freeInode.linksCount = 1;

            // Calculate newEntry recLength
            if (entries.empty()) {
                newEntry.recLength = BLOCK_SIZE;
            } else {
                entries.back().recLength = 64;
                short sum = accumulate(begin(entries), end(entries), 0, [&](short acc, const FileEntry &fe){
                    return acc + fe.recLength;
                });
                sum %= BLOCK_SIZE;
                newEntry.recLength = BLOCK_SIZE - sum;
            }
            newEntry.inodeIndex = freeInodeIndex;
            entries.push_back(newEntry);

            // Save to virtual disk
            writeFileEntriesForInode(this->inode_arr[parentInodeIndex], entries); // May break stuff
            this->inode_arr[parentInodeIndex].size += 64;
            this->inode_arr[freeInodeIndex] = freeInode;
            saveInodeList();
            previousInodeIndex = freeInodeIndex;
        } else {
            previousInodeIndex = inodeIndex;
        }
        acc = entry == "/" ? "/" : entry.substr(0,entry.find_last_of('/'));
    }
    saveInodeList();
}

void VirtualDisk::getDiskStatistics() {
    readInodeList();
    int size = 0;
    for (int i = 0; i<INODE_COUNT; i++) {
        size += this->inode_arr[i].size;
    }

    size += 2 * BLOCK_SIZE; // Add superBlock + inodes table
    cout << "Disk usage: " << size << " / " << this->size << " (" << static_cast<float>(size)/static_cast<float>(this->size) * 100 << "%)" << endl;
    vector<FileEntry> rootEntries = readFileEntriesForInode(ROOT_INODE_INDEX);
    printDirectories(rootEntries);
}

void VirtualDisk::printDirectories(vector<FileEntry> entries, const string& pathAcc) {
    for (auto &entry : entries) {
        short inodeIndex = getInodeIndexForFile(string(pathAcc + entry.name));
        if (this->inode_arr[inodeIndex].type == REGULAR_FILE) {
            cout << "File: " << string(pathAcc + entry.name) << ", Size: " << this->inode_arr[inodeIndex].size << " B" << endl;
        } else {
            cout << "Directory: " << string(pathAcc + entry.name) << endl;
            vector <FileEntry> nestedEntries = readFileEntriesForInode(inodeIndex);
            printDirectories(nestedEntries, string(pathAcc + entry.name + "/"));
        }
    }
}

void VirtualDisk::removeFile(string path){
    readInodeList();
    // Get file inode inde
    int index = getInodeIndexForFile(path);
    string parentPath = path.substr(0,path.find_last_of('/'));
    // Get parent inode index
    int parentIndex = getInodeIndexForFile(parentPath);
    this->inode_arr[index].linksCount -=1;
    // If this was the last link, this inode is now free
    if(!this->inode_arr[index].linksCount){
        this->inode_arr[index].size = 0;
        this->inode_arr[index].type = 0;
        memset(this->inode_arr[index].blocks, 0, sizeof (this->inode_arr[index].blocks));
    }
    // Modify parent file entries
    vector<FileEntry> entries = readFileEntriesForInode(parentIndex);
    string filename = splitPath(path).back();
    // Remove file entry for deleted file
    auto it = remove_if(entries.begin(), entries.end(), [&](const FileEntry entry) {
        return strcmp(entry.name, filename.c_str()) == 0;
    });
    entries.erase(it);
    if(entries.empty()){
        // This directory is now empty
        this->inode_arr[parentIndex].size = 0;
        memset(this->inode_arr[parentIndex].blocks, 0, sizeof (this->inode_arr[index].blocks));
    } else {
        // Modify existing file entries
        int counter = 0;
        int sum = 0;
        for (auto &entry : entries) {
            if (counter < entries.size() - 1) {
                entry.recLength = 64;
                sum += entry.recLength;
                counter++;
            } else {
                entry.recLength = BLOCK_SIZE - (sum%BLOCK_SIZE);
            }
        }
        writeFileEntriesForInode(this->inode_arr[parentIndex],entries);
    }
    saveInodeList();
}
void VirtualDisk::removeLink(const string& linkPath){
    readInodeList();
    string parentPath = linkPath.substr(0,linkPath.find_last_of('/'));
    string linkName = splitPath(linkPath).back();
    int parentInodeIndex = getInodeIndexForFile(parentPath);
    vector<FileEntry> entries = readFileEntriesForInode(parentInodeIndex);
    auto entryToDelete = find_if(entries.begin(),entries.end(), [linkName](const FileEntry fileEntry){
        return string(fileEntry.name) == linkName;
    });
    this->inode_arr[entryToDelete->inodeIndex].linksCount -= 1;
    this->inode_arr[parentInodeIndex].size -= 64;
    entries.erase(entryToDelete);
    if(!entries.empty()){
        int sum = 0;
        for (auto &entry : entries){
            sum+= entry.recLength;
        }
        entries.back().recLength = BLOCK_SIZE - sum%BLOCK_SIZE + 64;
    }
    writeFileEntriesForInode(this->inode_arr[parentInodeIndex],entries);
    saveInodeList();
}

void VirtualDisk::link(string newFile, string existingFile){
    int inodeBeingLinkedTo = getInodeIndexForFile(existingFile);
    readInodeList();
    vector<string> entriesToTraverse = splitPath(newFile);
    string acc = "";
    short previousInodeIndex = ROOT_INODE_INDEX;

    for (auto &entry : entriesToTraverse) {
        readInodeList();
        if (entry == "/") {
            acc = "/";
            continue;
        }

        string tempPath = entry.substr(0,entry.find_last_of('/'));
        short inodeIndex = getInodeIndexForFile(tempPath, previousInodeIndex);
        if (inodeIndex == -1) {
            // create file with path 'acc + entry'
            string parentPath = acc;
            short parentInodeIndex = previousInodeIndex;
            vector<FileEntry> entries;
            // Read existing entries
            if(this->inode_arr[parentInodeIndex].size){
                entries = readFileEntriesForInode(parentInodeIndex);
            }

            FileEntry newEntry{};
            string newName = entry.substr(0,entry.find_last_of('/'));
            strcpy(newEntry.name, newName.c_str()); // Copy file name

            // Calculate newEntry recLength
            if (entries.empty()) {
                newEntry.recLength = BLOCK_SIZE;
            } else {
                entries.back().recLength = 64;
                short sum = accumulate(begin(entries), end(entries), 0, [&](short acc, const FileEntry &fe){
                    return acc + fe.recLength;
                });
                sum %= BLOCK_SIZE;
                newEntry.recLength = BLOCK_SIZE - sum;
            }
            newEntry.inodeIndex = inodeBeingLinkedTo;
            entries.push_back(newEntry);

            // Save to virtual disk
            writeFileEntriesForInode(this->inode_arr[parentInodeIndex], entries); // May break stuff
            this->inode_arr[parentInodeIndex].size += 64;
            this->inode_arr[inodeBeingLinkedTo].linksCount += 1;
            saveInodeList();
        } else {
            previousInodeIndex = inodeIndex;
        }
        acc = entry == "/" ? "/" : entry.substr(0,entry.find_last_of('/'));
    }
    saveInodeList();

}

void VirtualDisk::removeBytesFromFile(string path, short startIndex, short endIndex) {
    short inodeIndex = getInodeIndexForFile(path);
    short size = this->inode_arr[inodeIndex].size;
    char buffer[size];
    int counter = 0;
    for (auto &block : this->inode_arr[inodeIndex].blocks) {
        fseek(this->disk, block * BLOCK_SIZE, 0);
        fread(buffer+ counter*BLOCK_SIZE, min(BLOCK_SIZE, static_cast<int>(size)), 1, this->disk);
        size -= BLOCK_SIZE;
        counter++;

        if (size <= 0) {
            break;
        }
    }

    string str = string(buffer);
    str.erase(str.begin() + startIndex, str.begin() + endIndex);

    // Write this as array of chars to file
    char newBuffer[this->inode_arr[inodeIndex].size - (endIndex - startIndex)];
    strcpy(newBuffer, str.c_str());
    this->inode_arr[inodeIndex].size = static_cast<short>(this->inode_arr[inodeIndex].size - (endIndex - startIndex));
    memset(&this->inode_arr[inodeIndex].blocks, 0, sizeof (this->inode_arr[inodeIndex].blocks));
    saveInodeList();
    // Write to disk
    short requiredBlocksCount = ceil(static_cast<float>(this->inode_arr[inodeIndex].size) / BLOCK_SIZE);
    vector<short> blocksToWrite = findFreeBlocks(requiredBlocksCount);
    long filelen = this->inode_arr[inodeIndex].size;
    // Write file to disk block by block
    for(int i = 0; i < blocksToWrite.size(); i++) {
        char tempBuffer[min(static_cast<long>(BLOCK_SIZE),filelen)];
        memcpy(tempBuffer,newBuffer + i * BLOCK_SIZE, min(static_cast<long>(BLOCK_SIZE),filelen));
        fseek(this->disk, blocksToWrite[i] * BLOCK_SIZE ,0);
        fwrite(tempBuffer,1,min(static_cast<long>(BLOCK_SIZE),filelen),this->disk);
        filelen -= BLOCK_SIZE;
        this->inode_arr[inodeIndex].blocks[i] = blocksToWrite[i];
    }
    saveInodeList();
}

void VirtualDisk::addBytesToFile(string path, string newBytes, short startIndex) {
    short inodeIndex = getInodeIndexForFile(path);
    short size = this->inode_arr[inodeIndex].size;
    char buffer[size + newBytes.size()];
    int counter = 0;
    for (auto &block : this->inode_arr[inodeIndex].blocks) {
        fseek(this->disk, block * BLOCK_SIZE, 0);
        fread(buffer+ counter*BLOCK_SIZE, min(BLOCK_SIZE, static_cast<int>(size)), 1, this->disk);
        size -= BLOCK_SIZE;
        counter++;

        if (size <= 0) {
            break;
        }
    }

    string str = string(buffer);
    str.insert(startIndex, newBytes);

    // Write this as array of chars to file
    char newBuffer[str.size()];
    strcpy(newBuffer, str.c_str());
    this->inode_arr[inodeIndex].size = static_cast<short>(str.size());
    memset(this->inode_arr[inodeIndex].blocks, 0, sizeof (this->inode_arr[inodeIndex].blocks));
    saveInodeList();
    readInodeList();
    // Write to disk
    short requiredBlocksCount = ceil(static_cast<float>(this->inode_arr[inodeIndex].size) / BLOCK_SIZE);
    vector<short> blocksToWrite = findFreeBlocks(requiredBlocksCount);
    long filelen = this->inode_arr[inodeIndex].size;
    // Write file to disk block by block
    for(int i = 0; i < blocksToWrite.size(); i++) {
        char tempBuffer[min(static_cast<long>(BLOCK_SIZE),filelen)];
        memcpy(tempBuffer,newBuffer + i * BLOCK_SIZE, min(static_cast<long>(BLOCK_SIZE),filelen));
        fseek(this->disk, blocksToWrite[i] * BLOCK_SIZE ,0);
        fwrite(tempBuffer,1,min(static_cast<long>(BLOCK_SIZE),filelen),this->disk);
        filelen -= BLOCK_SIZE;
        this->inode_arr[inodeIndex].blocks[i] = blocksToWrite[i];
    }
    saveInodeList();
}

VirtualDisk::VirtualDisk(string name, int size) : name(name), size(size){
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        this->disk = fopen(name.c_str(), "a+");
        this->readInodeList();
        this->readSuperBlock();

        fseek(file,0,SEEK_END);
        int filesize = static_cast<int>(ftell(file));
        this->size = filesize;
        rewind(file);
        std::cout << "Opened existing Virtualdisk " << name << std::endl;
    } else {
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
        std::cout << "Created Virtualdisk " << name << std::endl;
    }

}

VirtualDisk::~VirtualDisk() {
    saveInodeList();
    saveSuperBlock();
    fclose(this->disk);
}