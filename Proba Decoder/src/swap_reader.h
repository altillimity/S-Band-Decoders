#pragma once

#include "ccsds/ccsds.h"
#include <fstream>
#include <map>

class SWAPReader
{
private:
    int count;
    std::map<uint16_t, std::pair<int, std::pair<std::string, std::ofstream>>> currentOutFiles;
    std::string output_folder;

public:
    SWAPReader(std::string &outputfolder);
    void work(libccsds::CCSDSPacket &packet);
    void save();
};