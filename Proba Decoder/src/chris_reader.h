#pragma once

#include "ccsds/ccsds.h"

#define cimg_use_png
#define cimg_display 0
#include "CImg.h"
#include <string>

class CHRISReader
{
private:
    int count;
    std::string output_folder;

private:
    unsigned short *tempChannelBuffer;
    std::vector<int> modeMarkers;
    int mode;
    int current_width, current_height, max_value;
    int frame_count;

private:
    void writeChlorophylCompos(cimg_library::CImg<unsigned short> &img);
    void writeLandCompos(cimg_library::CImg<unsigned short> &img);
    std::string getModeName(int mode);

public:
    CHRISReader(std::string &outputfolder);
    ~CHRISReader();
    void work(libccsds::CCSDSPacket &packet);
    void save();
};
