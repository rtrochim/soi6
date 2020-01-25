#include <iostream>
#include "VirtualDisk.h"

int main() {
    VirtualDisk vd("disk", 32768);

    vd.readSuperBlock();
    vd.readInodeList();
    vd.createDirectory("/usr/tmp/dupa");
    vd.writeFileToDisk("../lorem1","/usr/tmp/");
    vd.copyFileFromDisk("/usr/tmp/lorem1", "./lorem1FromDisk");
    vd.writeFileToDisk("../lorem2","/usr/tmp/");
    vd.copyFileFromDisk("/usr/tmp/lorem2", "./lorem2FromDisk");
//    vd.writeFileToDisk("../lorem3","/usr/tmp/");
//    vd.copyFileFromDisk("/usr/tmp/lorem3", "./lorem3FromDisk");
    return 0;
}
