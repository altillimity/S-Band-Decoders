#include "swap_reader.h"
#include <fstream>
#include <iostream>
#include <map>
#include <filesystem>

#define cimg_use_png
#define cimg_use_jpeg
#define cimg_display 0
#include "CImg.h"

SWAPReader::SWAPReader(std::string &outputfolder)
{
    count = 0;
    output_folder = outputfolder;
}

void SWAPReader::work(libccsds::CCSDSPacket &packet)
{
    if (packet.payload.size() < 65530)
        return;

    uint16_t marker = packet.payload[10 - 6] << 8 | packet.payload[11 - 6];

    // Start new image
    if (currentOutFiles.find(marker) == currentOutFiles.end())
    {
        std::cout << "Found new SWAP image! Saving as SWAP-" + std::to_string(count) + ".jpeg. Marker " << marker << std::endl;
        currentOutFiles.insert(std::pair<uint16_t, std::pair<int, std::pair<std::string, std::ofstream>>>(marker, std::pair<int, std::pair<std::string, std::ofstream>>(0, std::pair<std::string, std::ofstream>("SWAP-" + std::to_string(count), std::ofstream("SWAP-" + std::to_string(count) + ".jpeg", std::ios::binary)))));
        count++;
    }

    int &currentFrameCount = currentOutFiles[marker].first;
    std::ofstream &currentOutFile = currentOutFiles[marker].second.second;

    if (currentFrameCount == 0)
        currentOutFile.write((char *)&packet.payload[14 + 78], 65530 - 14);
    else
        currentOutFile.write((char *)&packet.payload[14], 65530 - 14);

    currentFrameCount++;
}

void SWAPReader::save()
{
    for (std::pair<const uint16_t, std::pair<int, std::pair<std::string, std::ofstream>>> &currentPair : currentOutFiles)
    {
        currentPair.second.second.second.close();

        std::string filename = currentPair.second.second.first;

        std::cout << "Decompressing " << filename << "...";

        cimg_library::CImg<unsigned short> img;
        try
        {
            img.load_jpeg(std::string(filename + ".jpeg").c_str());
        }
        catch (std::exception &e)
        {
            std::cout << " Error! Deleting " << std::endl;
            std::filesystem::remove(filename + ".jpeg");
            continue;
        }

        std::cout << " Good! Saving as png... ";
        img.save_png(std::string(output_folder + "/" + filename + ".png").c_str());

        std::cout << "Done! Deleting jpeg..." << std::endl;

        std::filesystem::remove(filename + ".jpeg");
    }
}