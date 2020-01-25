#include <iostream>
#include "VirtualDisk.h"

int main() {
    VirtualDisk vd("disk", 32768);

    vd.readSuperBlock();
    vd.readInodeList();

    vd.writeFileToDisk("../lorem","/");
    vd.copyFileFromDisk("/lorem", "./loremFromDisk");
    return 0;
}
