#pragma once

#include "ccsds/ccsds.h"

#define cimg_use_png
#define cimg_display 0
#include "CImg.h"

class CHRISReader
{
private:
    unsigned short *tempChannelBuffers[2];
    uint16_t chrisBuffer1[7680], chrisBuffer2[7680], chrisBuffer3[7680];
    std::vector<int> modeMarkers;

public:
    CHRISReader();
    int count;
    int mode;
    int current_width, current_height, max_value;
    int frame_count_ch1, frame_count_ch2;
    void writeChlorophylCompos(cimg_library::CImg<unsigned short> &img);
    void writeWaterCompos(cimg_library::CImg<unsigned short> &img);
    void work(libccsds::CCSDSPacket &packet);
    void workCh1(libccsds::CCSDSPacket &packet, uint16_t &count_marker);
    void workCh2(libccsds::CCSDSPacket &packet, uint16_t &count_marker);
    void save();
};