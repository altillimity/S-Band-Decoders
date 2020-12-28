#include "chris_reader.h"
#include <fstream>
#include <iostream>
#include <map>

#define ALL_MODE 1
#define WATER_MODE 2
#define LAND_MODE 3
#define CHLOROPHYL_MODE 4
#define LAND_ALL_MODE 5

std::string getModeName(int mode)
{
    if (mode == ALL_MODE)
        return "ALL";
    else if (mode == WATER_MODE)
        return "WATER";
    else if (mode == LAND_MODE)
        return "LAND";
    else if (mode == CHLOROPHYL_MODE)
        return "CHLOROPHYL";
    else if (mode == LAND_ALL_MODE)
        return "ALL-LAND";

    return "";
}

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

CHRISReader::CHRISReader()
{
    for (int i = 0; i < 2; i++)
        tempChannelBuffers[i] = new unsigned short[748 * 12096];
    count = 0;
    mode = 0;
    current_width = 12096;
    current_height = 748;
    max_value = 710;
}

void CHRISReader::workCh1(libccsds::CCSDSPacket &packet, uint16_t &count_marker)
{
    int posb = 16;

    // Convert into 12-bits values
    for (int i = 0; i < 7680; i += 2)
    {
        uint16_t px1 = packet.payload[posb + 0] | ((packet.payload[posb + 1] & 0xF) << 8);
        uint16_t px2 = (packet.payload[posb + 1] >> 4) | (packet.payload[posb + 2] << 4);
        tempChannelBuffers[0][count_marker * 7680 + (i + 0)] = px1 << 4;
        tempChannelBuffers[0][count_marker * 7680 + (i + 1)] = px2 << 4;
        posb += 3;
    }

    frame_count_ch1++;

    // Frame counter
    if (count_marker == max_value)
    {
        std::cout << "Finished CHRIS image! Saving as CHRIS-" + std::to_string(count) + ".png. Mode " << getModeName(mode) << std::endl;
        cimg_library::CImg<unsigned short> img = cimg_library::CImg<unsigned short>(tempChannelBuffers[0], current_width, current_height);
        img.normalize(0, 65535);
        img.save_png(std::string("CHRIS-" + std::to_string(count) + ".png").c_str());

        if (mode == CHLOROPHYL_MODE)
            writeChlorophylCompos(img);
        else if (mode == WATER_MODE)
            writeWaterCompos(img);

        std::fill(&tempChannelBuffers[0][0], &tempChannelBuffers[0][748 * 12096], 0);
        count++;
        frame_count_ch1 = 0;
    }
}

void CHRISReader::workCh2(libccsds::CCSDSPacket &packet, uint16_t &count_marker)
{
    int posb = 16;

    // Convert into 12-bits values
    for (int i = 0; i < 7680; i += 2)
    {
        uint16_t px1 = packet.payload[posb + 0] | ((packet.payload[posb + 1] & 0xF) << 8);
        uint16_t px2 = (packet.payload[posb + 1] >> 4) | (packet.payload[posb + 2] << 4);
        tempChannelBuffers[1][count_marker * 7680 + (i + 0)] = px1 << 4;
        tempChannelBuffers[1][count_marker * 7680 + (i + 1)] = px2 << 4;
        posb += 3;
    }

    frame_count_ch2++;

    // Frame counter
    if (count_marker == max_value)
    {
        std::cout << "Finished CHRIS image! Saving as CHRIS-" + std::to_string(count) + ".png. Mode " << getModeName(mode) << std::endl;
        cimg_library::CImg<unsigned short> img = cimg_library::CImg<unsigned short>(tempChannelBuffers[1], current_width, current_height);
        img.normalize(0, 65535);
        img.save_png(std::string("CHRIS-" + std::to_string(count) + ".png").c_str());

        if (mode == CHLOROPHYL_MODE)
            writeChlorophylCompos(img);
        else if (mode == WATER_MODE)
            writeWaterCompos(img);

        std::fill(&tempChannelBuffers[1][0], &tempChannelBuffers[1][748 * 12096], 0);
        count++;
        frame_count_ch2 = 0;
    }
}

void CHRISReader::work(libccsds::CCSDSPacket &packet)
{
    if (packet.payload.size() < 11538)
        return;

    int channel_marker = packet.payload[9 - 6];
    uint16_t count_marker = packet.payload[16 - 6] << 8 | packet.payload[17 - 6];
    int mode_marker = (packet.payload[7 - 6] >> 6) + 1;

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
    }

    modeMarkers.push_back(mode_marker);

    if (channel_marker == 128)
        workCh1(packet, count_marker);
    else if (channel_marker == 0)
        workCh2(packet, count_marker);
}

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
    image16169.save_png(std::string("CHRIS-" + std::to_string(count) + "-RGB16-16-9.png").c_str());

    cimg_library::CImg<unsigned short> image13169(375, 748, 1, 3);
    image13169.draw_image(0, 0, 0, 0, img13);
    image13169.draw_image(0, 0, 0, 1, img16);
    image13169.draw_image(0, 0, 0, 2, img9);
    image13169.save_png(std::string("CHRIS-" + std::to_string(count) + "-RGB13-16-9.png").c_str());
}

void CHRISReader::writeWaterCompos(cimg_library::CImg<unsigned short> &img)
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
    image5170.save_png(std::string("CHRIS-" + std::to_string(count) + "-RGB5-17-0.png").c_str());

    cimg_library::CImg<unsigned short> image81410(375, 748, 1, 3);
    image81410.draw_image(0, 0, 0, 0, imgs[8]);
    image81410.draw_image(0, 0, 0, 1, imgs[14]);
    image81410.draw_image(0, 0, 0, 2, imgs[10]);
    image81410.save_png(std::string("CHRIS-" + std::to_string(count) + "-RGB8-14-10.png").c_str());

    cimg_library::CImg<unsigned short> image8130(375, 748, 1, 3);
    image8130.draw_image(0, 0, 0, 0, imgs[8]);
    image8130.draw_image(0, 0, 0, 1, imgs[13]);
    image8130.draw_image(0, 0, 0, 2, imgs[0]);
    image8130.save_png(std::string("CHRIS-" + std::to_string(count) + "-RGB8-13-0.png").c_str());

    cimg_library::CImg<unsigned short> image13169(375, 748, 1, 3);
    image13169.draw_image(0, 0, 0, 0, imgs[13]);
    image13169.draw_image(0, 0, 0, 1, imgs[16]);
    image13169.draw_image(0, 0, 0, 2, imgs[9]);
    image13169.save_png(std::string("CHRIS-" + std::to_string(count) + "-RGB13-16-9.png").c_str());
}

void CHRISReader::save()
{
    std::cout << "Saving in-progress CHRIS data! (if any)" << std::endl;

    if (frame_count_ch1 != 0)
    {
        std::cout << "Finished CHRIS image! Saving as CHRIS-" + std::to_string(count) + ".png. Mode " << getModeName(mode) << std::endl;
        cimg_library::CImg<unsigned short> img = cimg_library::CImg<unsigned short>(tempChannelBuffers[1], current_width, current_height);
        img.normalize(0, 65535);
        img.save_png(std::string("CHRIS-" + std::to_string(count) + ".png").c_str());

        if (mode == CHLOROPHYL_MODE)
            writeChlorophylCompos(img);
        else if (mode == WATER_MODE)
            writeWaterCompos(img);

        std::fill(&tempChannelBuffers[1][0], &tempChannelBuffers[1][748 * 12096], 0);
        count++;
    }

    if (frame_count_ch2 != 0)
    {
        std::cout << "Finished CHRIS image! Saving as CHRIS-" + std::to_string(count) + ".png. Mode " << getModeName(mode) << std::endl;
        cimg_library::CImg<unsigned short> img = cimg_library::CImg<unsigned short>(tempChannelBuffers[1], current_width, current_height);
        img.normalize(0, 65535);
        img.save_png(std::string("CHRIS-" + std::to_string(count) + ".png").c_str());

        if (mode == CHLOROPHYL_MODE)
            writeChlorophylCompos(img);
        else if (mode == WATER_MODE)
            writeWaterCompos(img);

        std::fill(&tempChannelBuffers[1][0], &tempChannelBuffers[1][748 * 12096], 0);
        count++;
    }
}