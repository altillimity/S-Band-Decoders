#include "chris_reader.h"
#include <fstream>
#include <iostream>
#include <map>

#define ALL_MODE 2
#define WATER_MODE 3
#define LAND_MODE 3
#define CHLOROPHYL_MODE 3
#define LAND_ALL_MODE 100 // Never seen yet...

template <class InputIt, class T = typename std::iterator_traits<InputIt>::value_type>
T most_common(InputIt begin, InputIt end)
{
    std::map<T, int> counts;
    for (InputIt it = begin; it != end; ++it)
    {
        if (counts.find(*it) != counts.end())
            ++counts[*it];
        else
            counts[*it] = 1;
    }
    return std::max_element(counts.begin(), counts.end(), [](const std::pair<T, int> &pair1, const std::pair<T, int> &pair2) { return pair1.second < pair2.second; })->first;
}

CHRISReader::CHRISReader(std::string &outputfolder)
{
    count = 0;
    output_folder = outputfolder;
    tempChannelBuffer = new unsigned short[748 * 12096];
    mode = 0;
    current_width = 12096;
    current_height = 748;
    max_value = 710;
    frame_count = 0;
    output_folder = outputfolder;
}

CHRISReader::~CHRISReader()
{
    delete[] tempChannelBuffer;
}

void CHRISReader::work(libccsds::CCSDSPacket &packet)
{
    if (packet.payload.size() < 11538)
        return;

    uint16_t count_marker = packet.payload[10] << 8 | packet.payload[11];
    int mode_marker = packet.payload[9] & 0x03;

    int tx_mode = (packet.payload[2] & 0b00000011) << 2 | packet.payload[3] >> 6;

    //std::cout << "CH " << channel_marker << std::endl;
    //std::cout << "CNT " << count_marker << std::endl;
    //std::cout << "MODE " << mode_marker << std::endl;
    //std::cout << "TMD " << tx_mode << std::endl;

    int posb = 16;

    if (mode_marker == ALL_MODE)
    {
        if (tx_mode == 8)
            posb = 16;
        if (tx_mode == 0)
            posb = 15;
        if (tx_mode == 200)
            posb = 16;
        if (tx_mode == 72)
            posb = 16;
        if (tx_mode == 200)
            posb = 16;
        if (tx_mode == 136)
            posb = 16;
        if (tx_mode == 192)
            posb = 16;

        if (tx_mode == 128)
            posb = 15;
        if (tx_mode == 64)
            posb = 15;
    }
    else if (mode_marker == LAND_MODE)
    {
        if (tx_mode == 72)
            posb = 16;
        if (tx_mode == 64)
            posb = 16;
        if (tx_mode == 8)
            posb = 16;
        if (tx_mode == 0)
            posb = 16;
    }

    // Convert into 12-bits values
    for (int i = 0; i < 7680; i += 2)
    {
        if (count_marker <= max_value)
        {
            uint16_t px1 = packet.payload[posb + 0] | ((packet.payload[posb + 1] & 0xF) << 8);
            uint16_t px2 = (packet.payload[posb + 1] >> 4) | (packet.payload[posb + 2] << 4);
            tempChannelBuffer[count_marker * 7680 + (i + 0)] = px1 << 4;
            tempChannelBuffer[count_marker * 7680 + (i + 1)] = px2 << 4;
            posb += 3;
        }
    }

    frame_count++;

    // Frame counter
    if (count_marker == max_value)
    {
        save();
        frame_count = 0;
        modeMarkers.clear();
    }

    if ((count_marker > 50 && count_marker < 70) || (count_marker > 500 && count_marker < 520) || (count_marker > 700 && count_marker < 720))
    {

        mode = most_common(modeMarkers.begin(), modeMarkers.end());

        if (mode == WATER_MODE || mode == CHLOROPHYL_MODE || mode == LAND_MODE)
        {
            current_width = 7296;
            current_height = 748;
            max_value = 710;
        }
        else if (mode == ALL_MODE)
        {
            current_width = 12096;
            current_height = 374;
            max_value = 589;
        }
        else if (mode == LAND_ALL_MODE)
        {
            current_width = 7680;
            current_height = 374;
            max_value = 589;
        }
    }

    modeMarkers.push_back(mode_marker);
}

void CHRISReader::save()
{
    if (frame_count != 0)
    {
        std::cout << "Finished CHRIS image! Saving as CHRIS-" + std::to_string(count) + ".png. Mode " << getModeName(mode) << std::endl;
        cimg_library::CImg<unsigned short> img = cimg_library::CImg<unsigned short>(tempChannelBuffer, current_width, current_height);
        img.normalize(0, 65535);
        img.save_png(std::string(output_folder + "/CHRIS-" + std::to_string(count) + ".png").c_str());

        if (mode == CHLOROPHYL_MODE)
            writeChlorophylCompos(img);
        if (mode == LAND_MODE)
            writeLandCompos(img);

        std::fill(&tempChannelBuffer[0], &tempChannelBuffer[748 * 12096], 0);
        count++;
    }
};

void CHRISReader::writeChlorophylCompos(cimg_library::CImg<unsigned short> &img)
{
    std::cout << "Writing chlorophyl mode RGB compositions..." << std::endl;
    cimg_library::CImg<unsigned short> img9 = img;
    cimg_library::CImg<unsigned short> img13 = img;
    cimg_library::CImg<unsigned short> img16 = img;
    img9.crop(3077, 3077 + 375);
    img13.crop(4613, 4613 + 375);
    img16.crop(5765, 5765 + 375);

    cimg_library::CImg<unsigned short> image16169(375, 748, 1, 3);
    image16169.draw_image(0, 0, 0, 0, img16);
    image16169.draw_image(0, 0, 0, 1, img16);
    image16169.draw_image(0, 0, 0, 2, img9);
    image16169.save_png(std::string(output_folder + "/CHRIS-" + std::to_string(count) + "-RGB16-16-9.png").c_str());

    cimg_library::CImg<unsigned short> image13169(375, 748, 1, 3);
    image13169.draw_image(0, 0, 0, 0, img13);
    image13169.draw_image(0, 0, 0, 1, img16);
    image13169.draw_image(0, 0, 0, 2, img9);
    image13169.save_png(std::string(output_folder + "/CHRIS-" + std::to_string(count) + "-RGB13-16-9.png").c_str());
}

void CHRISReader::writeLandCompos(cimg_library::CImg<unsigned short> &img)
{
    std::cout << "Writing water mode RGB compositions..." << std::endl;
    cimg_library::CImg<unsigned short> imgs[19];
    for (int i = 0; i < 19; i++)
    {
        imgs[i] = img;
        imgs[i].crop(5 + i * 384, 5 + i * 384 + 375);
    }

    cimg_library::CImg<unsigned short> image5170(375, 748, 1, 3);
    image5170.draw_image(0, 0, 0, 0, imgs[5]);
    image5170.draw_image(0, 0, 0, 1, imgs[17]);
    image5170.draw_image(0, 0, 0, 2, imgs[0]);
    image5170.save_png(std::string(output_folder + "/CHRIS-" + std::to_string(count) + "-RGB5-17-0.png").c_str());

    cimg_library::CImg<unsigned short> image81410(375, 748, 1, 3);
    image81410.draw_image(0, 0, 0, 0, imgs[8]);
    image81410.draw_image(0, 0, 0, 1, imgs[14]);
    image81410.draw_image(0, 0, 0, 2, imgs[10]);
    image81410.save_png(std::string(output_folder + "/CHRIS-" + std::to_string(count) + "-RGB8-14-10.png").c_str());

    cimg_library::CImg<unsigned short> image8130(375, 748, 1, 3);
    image8130.draw_image(0, 0, 0, 0, imgs[8]);
    image8130.draw_image(0, 0, 0, 1, imgs[13]);
    image8130.draw_image(0, 0, 0, 2, imgs[0]);
    image8130.save_png(std::string(output_folder + "/CHRIS-" + std::to_string(count) + "-RGB8-13-0.png").c_str());

    cimg_library::CImg<unsigned short> image13169(375, 748, 1, 3);
    image13169.draw_image(0, 0, 0, 0, imgs[13]);
    image13169.draw_image(0, 0, 0, 1, imgs[16]);
    image13169.draw_image(0, 0, 0, 2, imgs[9]);
    image13169.save_png(std::string(output_folder + "/CHRIS-" + std::to_string(count) + "-RGB13-16-9.png").c_str());
}

std::string CHRISReader::getModeName(int mode)
{
    if (mode == ALL_MODE)
        return "ALL";
    else if (mode == WATER_MODE)
        return "LAND/WATER/CHLOROPHYL";
    else if (mode == LAND_MODE)
        return "LAND/WATER/CHLOROPHYL";
    else if (mode == CHLOROPHYL_MODE)
        return "LAND/WATER/CHLOROPHYL";
    else if (mode == LAND_ALL_MODE)
        return "ALL-LAND";

    return "";
}