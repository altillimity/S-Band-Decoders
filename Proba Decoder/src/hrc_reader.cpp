#include "hrc_reader.h"
#include <fstream>
#include <iostream>
#include <map>

HRCReader::HRCReader(std::string &outputfolder)
{
    tempChannelBuffer = new unsigned short[74800 * 12096];
    frame_count = 0;
    count = 0;
    output_folder = outputfolder;
}

HRCReader::~HRCReader()
{
    delete[] tempChannelBuffer;
}

uint8_t reverseBits(uint8_t byte)
{
    byte = (byte & 0xF0) >> 4 | (byte & 0x0F) << 4;
    byte = (byte & 0xCC) >> 2 | (byte & 0x33) << 2;
    byte = (byte & 0xAA) >> 1 | (byte & 0x55) << 1;
    return byte;
}

void HRCReader::work(libccsds::CCSDSPacket &packet)
{
    if (packet.payload.size() < 21458)
        return;

    int count_marker = packet.payload[18 - 6];

    int pos = 21;

    for (int i = 0; i < 21458; i++)
        packet.payload[i] = reverseBits(packet.payload[i]);

    // Convert into 10-bits values
    for (int i = 0; i < 17152; i += 4)
    {
        if (count_marker <= 65)
        {
            tempChannelBuffer[count_marker * 17152 + i + 0] = reverseBits((packet.payload[pos + 0] << 2) | (packet.payload[pos + 1] >> 6));
            tempChannelBuffer[count_marker * 17152 + i + 1] = reverseBits(((packet.payload[pos + 1] % 64) << 4) | (packet.payload[pos + 2] >> 4));
            tempChannelBuffer[count_marker * 17152 + i + 2] = reverseBits(((packet.payload[pos + 2] % 16) << 6) | (packet.payload[pos + 3] >> 2));
            tempChannelBuffer[count_marker * 17152 + i + 3] = reverseBits(((packet.payload[pos + 3] % 4) << 8) | packet.payload[pos + 4]);
            pos += 5;
        }
    }

    frame_count++;

    if (count_marker == 65)
    {
        save();
        frame_count = 0;
    }
}

void HRCReader::save()
{
    if (frame_count != 0)
    {
        std::cout << "Finished HRC image! Saving as HRC-" + std::to_string(count) + ".png" << std::endl;
        cimg_library::CImg<unsigned short> img = cimg_library::CImg<unsigned short>(tempChannelBuffer, 1072, 1072);
        img.normalize(0, 65535);
        img.save_png(std::string(output_folder + "/HRC-" + std::to_string(count) + ".png").c_str());
        img.equalize(1000);
        img.save_png(std::string(output_folder + "/HRC-" + std::to_string(count) + "-EQU.png").c_str());

        std::fill(&tempChannelBuffer[0], &tempChannelBuffer[748 * 12096], 0);
        count++;
    }
}