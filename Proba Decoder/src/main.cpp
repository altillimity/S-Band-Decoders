#include <iostream>
#include <fstream>
#include <complex>
#include <vector>
#include <cstring>
#include <iomanip>
#include <filesystem>
#include "ccsds/vcdu.h"
#include "ccsds/demuxer.h"
#include "ccsds/mpdu.h"
#include "chris_reader.h"
#include "hrc_reader.h"
#include "swap_reader.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage : " << argv[0] << "(1/2/V) inputframes.bin [output_dir]" << std::endl;
        return 1;
    }

    bool mode_proba1 = false;
    bool mode_proba2 = false;
    bool mode_probaV = false;

    if (std::string(argv[1]) == "1")
        mode_proba1 = true;
    else if (std::string(argv[1]) == "2")
        mode_proba2 = true;
    else if (std::string(argv[1]) == "V")
        mode_probaV = true;
    else
    {
        std::cout << "This mode does not exist!" << std::endl;
        return 1;
    }

    // Output and Input file
    std::ifstream data_in(argv[2], std::ios::binary);
    //std::ofstream data_out("chris.bin", std::ios::binary);
    
    std::string output_folder = ".";
    if (argc == 4)
    {
        output_folder = argv[3];

        // Mkdir our output folder if it doesn't exist
        if (!std::filesystem::exists(output_folder))
            std::filesystem::create_directories(output_folder);
    }

    // Read buffer
    uint8_t buffer[1279];

    // CCSDS Demuxer
    libccsds::Demuxer ccsdsDemuxer(1103, false);

    // Graphics
    std::cout << "---------------------------" << std::endl;
    std::cout << " Proba Decoder by Aang23" << std::endl;
    std::cout << "     S-Band 1 / 2 / V    " << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << std::endl;

    // Proba-1 Routine
    if (mode_proba1)
    {
        std::cout << "Starting in Proba-1 mode...\n"
                  << std::endl;

        CHRISReader chris_reader(output_folder);
        HRCReader hrc_reader(output_folder);

        // Read until EOF
        while (!data_in.eof())
        {
            // Read buffer
            data_in.read((char *)buffer, 1279);

            int vcid = libccsds::parseVCDU(buffer).vcid;

            if (vcid == 1)
            {
                std::vector<libccsds::CCSDSPacket> pkts = ccsdsDemuxer.work(buffer);

                if (pkts.size() > 0)
                {
                    for (libccsds::CCSDSPacket pkt : pkts)
                    {
                        if (pkt.header.apid == 2047)
                            continue;

                        if (pkt.header.apid == 0)
                        {
                            int mode_marker = pkt.payload[7 - 6];
                            if (mode_marker == 169)
                                hrc_reader.work(pkt);
                            else
                                chris_reader.work(pkt);
                        }
                    }
                }
            }
        }
        chris_reader.save();
        hrc_reader.save();
    }

    // Proba-2
    if (mode_proba2)
    {
        std::cout << "Starting in Proba-2 mode...\n"
                  << std::endl;

        SWAPReader swap_reader(output_folder);

        int cnt = 0;

        // Read until EOF
        while (!data_in.eof())
        {
            // Read buffer
            data_in.read((char *)buffer, 1279);

            int vcid = libccsds::parseVCDU(buffer).vcid;

            if (vcid == 3)
            {
                std::vector<libccsds::CCSDSPacket> pkts = ccsdsDemuxer.work(buffer);

                if (pkts.size() > 0)
                {
                    for (libccsds::CCSDSPacket pkt : pkts)
                    {
                        if (pkt.header.apid == 2047)
                            continue;

                        if (pkt.header.apid == 20)
                        {
                            swap_reader.work(pkt);
                        }
                    }
                }
            }
        }

        swap_reader.save();
    }

    // Proba-V
    if (mode_probaV)
    {
        std::cout << "Starting in Proba-V mode..." << std::endl;
        std::cout << "Proba-V is not yet supported!" << std::endl;
    }

    std::cout << std::endl
              << "Done! Enjoy" << std::endl;

    data_in.close();
    //data_out.close();
}