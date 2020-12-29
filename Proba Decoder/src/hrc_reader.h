#pragma once

#include "ccsds/ccsds.h"

#define cimg_use_png
#define cimg_display 0
#include "CImg.h"
#include <string>

class HRCReader
{
private:
    unsigned short *tempChannelBuffer;
    int count;
    int frame_count;
    std::string output_folder;

public:
    HRCReader(std::string &outputfolder);
    ~HRCReader();
    void save();
    void work(libccsds::CCSDSPacket &packet);
};
