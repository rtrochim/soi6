#include <iostream>
#include "VirtualDisk.h"

int main() {
    VirtualDisk vd("disk", 32768);
    vd.readSuperBlock();
    vd.readInodeList();
    vd.getDiskStatistics();
    vd.createDirectory("/usr/tmp/local");
    vd.writeFileToDisk("../lorem1","/usr");
    vd.copyFileFromDisk("/usr/lorem1", "./lorem1FromDisk");
    vd.writeFileToDisk("../lorem2","/usr/tmp");
    vd.copyFileFromDisk("/usr/tmp/lorem2", "./lorem2FromDisk");
    vd.writeFileToDisk("../lorem3","/usr/tmp/local");
    vd.copyFileFromDisk("/usr/tmp/local/lorem3", "./lorem3FromDisk");
    vd.removeFile("/usr/tmp/local/lorem3");
    vd.writeFileToDisk("../lorem3","/usr/tmp/local");
    vd.copyFileFromDisk("/usr/tmp/local/lorem3", "./lorem3FromDiskAfterDelete");
    return 0;
}
