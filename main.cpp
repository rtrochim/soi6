#include <iostream>
#include "VirtualDisk.h"

int main() {
    VirtualDisk vd("disk", 32768);
    vd.readSuperBlock();
    vd.readInodeList();
    vd.getDiskStatistics();
    vd.createDirectory("/usr/tmp/local");
    vd.getDiskStatistics();

    vd.writeFileToDisk("../lorem1","/usr");
    vd.getDiskStatistics();

    vd.copyFileFromDisk("/usr/lorem1", "./lorem1FromDisk");
    vd.writeFileToDisk("../lorem2","/usr/tmp");
    vd.getDiskStatistics();

    vd.copyFileFromDisk("/usr/tmp/lorem2", "./lorem2FromDisk");
    vd.writeFileToDisk("../lorem3","/usr/tmp/local");
    vd.getDiskStatistics();

    vd.copyFileFromDisk("/usr/tmp/local/lorem3", "./lorem3FromDisk");
    vd.removeFile("/usr/tmp/local/lorem3");
    vd.getDiskStatistics();

    vd.writeFileToDisk("../lorem3","/usr/tmp/local");
    vd.copyFileFromDisk("/usr/tmp/local/lorem3", "./lorem3FromDiskAfterDelete");
    vd.getDiskStatistics();

    return 0;
}
