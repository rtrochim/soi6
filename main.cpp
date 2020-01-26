#include <iostream>
#include "VirtualDisk.h"

void printAvailableOptions() {
    /**
     *  @OPTIONS
     *  mkdir - create directory (mkdir [absolutePath])
     *  stats - get disk statistics (stats)
     *  cpfd - copy from virtual disk to user disk (cpfd [virtualDiskPath] [userPath])
     *  cptd - copy from user disk to virtual disk (cptd [userPath] [virtualDiskPath])
     *  rm - remove file (rm [absolutePath])
     *  ln - link file (ln [newFile] [existingFile])
     *  unlink - removes an existing link (unlink [linkName])
     *  writeBytes - write extra bytes to existing file (writeBytes [path] [newBytes] [startIndex])
     *  removeBytes - removes bytes from existing file (removeBytes [path] [startIndex] [endIndex])
     *  help - print available options
     */

    cout << "Options: " << endl;
    cout << "mkdir - create directory (mkdir [absolutePath])" << endl;
    cout << "stats - get disk statistics (stats)" << endl;
    cout << "cpfd - copy from virtual disk to user disk (cpfd [virtualDiskPath] [userPath] )" << endl;
    cout << "cptd - copy from user disk to virtual disk (cptd [userPath] [virtualDiskPath])" << endl;
    cout << "rm - remove file (rm [absolutePath])" << endl;
    cout << "ln - link file (ln [newFile] [existingFile])" << endl;
    cout << "unlink - removes an existing link (unlink [linkName])" << endl;
    cout << "writeBytes - write extra bytes to existing file (writeBytes [path] [newBytes] [startIndex])" << endl;
    cout << "removeBytes - removes bytes from existing file (removeBytes [path] [startIndex] [endIndex])" << endl;
    cout << "help - print available options" << endl << endl;
}

vector<string> splitCommand(string command) {
    vector<string> result;

    while(command.length()) {
        short index = command.find_first_of(' ');
        if (index == -1) {
            result.push_back(command);
            break;
        }

        result.push_back(command.substr(0, index+1));
        command = command.substr(index + 1,command.length());
    }

    return result;
}

vector<string> splitCommand2(string str)
{
    string word = "";
    vector<string> result;
    int it = 0;

    for (auto x : str) {
        if (x == ' ' || it == str.size() - 1) {
            if (it == str.size() - 1) {
                word = word + x;
            }

            result.push_back(word);
            word = "";
        } else {
            word = word + x;
        }
        it++;
    }

    return result;
}

int main() {
    string option;
    string name;
    string size = "0";
    cout << "Virtual disk name: ";
    getline(cin, name);
    FILE *file = fopen(name.c_str(), "r");
    if (file) {
        fclose(file);
    } else {
        cout << "Disk size: ";
        getline(cin, size);
    }

    VirtualDisk vd(name, stoi(size));
    vd.readSuperBlock();
    vd.readInodeList();
    printAvailableOptions();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {
        cout << "# ";
        getline(cin, option);
        vector<string> commandSet = splitCommand2(option);
        string command = commandSet.at(0);
        vd.saveSuperBlock();
        vd.saveInodeList();
        vd.readSuperBlock();
        vd.readInodeList();

        if (command == "mkdir") {
            vd.createDirectory(commandSet.at(1));
            continue;
        }

        if (command == "stats") {
            vd.getDiskStatistics();
            continue;
        }

        if (command == "cpfd") {
            vd.copyFileFromDisk(commandSet.at(1), commandSet.at(2));
            continue;
        }

        if (command == "cptd") {
            vd.writeFileToDisk(commandSet.at(1), commandSet.at(2));
            continue;
        }

        if (command == "rm") {
            vd.removeFile(commandSet.at(1));
            continue;
        }

        if (command == "ln") {
            vd.link(commandSet.at(1), commandSet.at(2));
            continue;
        }

        if (command == "unlink") {
            vd.removeLink(commandSet.at(1));
            continue;
        }

        if (command == "writeBytes") {
            vd.addBytesToFile(commandSet.at(1), commandSet.at(2), stoi(commandSet.at(3)));
            continue;
        }

        if (command == "removeBytes") {
            vd.removeBytesFromFile(commandSet.at(1), stoi(commandSet.at(2)), stoi(commandSet.at(3)));
            continue;
        }

        if (command == "help") {
            printAvailableOptions();
            continue;
        }
    }
#pragma clang diagnostic pop

//    vd.createDirectory("/usr/tmp/local");
//
//    vd.writeFileToDisk("../lorem1","/usr");
//    vd.getDiskStatistics();
//    vd.link("/usr/lorem1Link","/usr/lorem1");
//    vd.getDiskStatistics();
//    vd.removeLink("/usr/lorem1Link");
//    vd.getDiskStatistics();
//    vd.copyFileFromDisk("/usr/lorem1", "./lorem1Before");
//    vd.removeBytesFromFile("/usr/lorem1",50,1000);
//    vd.copyFileFromDisk("/usr/lorem1", "./lorem1AfterRemove");
//    vd.addBytesToFile("/usr/lorem1", "randombytesfortestingpurpose", 10);
//    vd.copyFileFromDisk("/usr/lorem1", "./lorem1AfterAdd");
//    vd.writeFileToDisk("../lorem2","/usr/tmp");
//
//    vd.copyFileFromDisk("/usr/tmp/lorem2", "./lorem2FromDisk");
//    vd.writeFileToDisk("../lorem3","/usr/tmp/local");
//
//    vd.copyFileFromDisk("/usr/tmp/local/lorem3", "./lorem3FromDisk");
//    vd.link("/usr/lorem3Link","usr/tmp/local/lorem3");
//    vd.copyFileFromDisk("/usr/lorem3Link", "./lorem3LinkFromDisk");
//
//    vd.getDiskStatistics();
//
//    vd.removeFile("/usr/tmp/local/lorem3");
//    vd.writeFileToDisk("../lorem3","/usr/tmp/local");
//    vd.copyFileFromDisk("/usr/tmp/local/lorem3", "./lorem3FromDiskAfterDelete");

    return 0;
}
