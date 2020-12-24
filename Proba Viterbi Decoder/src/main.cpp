#include <iostream>
#include <fstream>
#include <complex>
#include <vector>
#include <cstring>
#include <iomanip>
#include "correlator.h"
#include "packetfixer.h"
#include "viterbi27.h"
#include "derandomizer.h"
#include "reedsolomon.h"

#define FRAME_SIZE 1279
#define ENCODED_FRAME_SIZE 1279 * 8 * 2

// Return filesize
size_t getFilesize(std::string filepath)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    std::size_t fileSize = file.tellg();
    file.close();
    return fileSize;
}

void shiftWithConstantSize(uint8_t *arr, int pos, int length)
{
    for (int i = 0; i < length - pos; i++)
    {
        arr[i] = arr[pos + i];
    }
}

// Processing buffer size
#define BUFFER_SIZE (ENCODED_FRAME_SIZE)

int main(int argc, char *argv[])
{
    if (argc != 2 && argc != 3)
    {
        std::cout << "Usage : " << argv[0] << " inputsoftsymbols.bin outputframes.bin [-v]" << std::endl;
        return 1;
    }

    bool probav = false;

    // Terra mode?
    if (argc == 3)
    {
        if (std::string(argv[2]) == "-v")
            probav = true;
    }

    // Output and Input file
    std::ifstream data_in(argv[1], std::ios::binary);
    std::ofstream data_out(argv[2], std::ios::binary);

    // Read buffer
    uint8_t buffer[BUFFER_SIZE];

    // Complete filesize
    size_t filesize = getFilesize(argv[1]);

    // Data we wrote out
    size_t data_out_total = 0;

    // Correlator
    SatHelper::Correlator correlator;

    // All encoded sync words. Though PROBAs are BPSK. So we only have 2! 
    // Weirdly enough... IQ inversed and always 90/270 deg phase shift weirdness? Well it works so whatever I guess!
    correlator.addWord((uint64_t)0xa9f7e368e558c2c1);
    correlator.addWord((uint64_t)0x56081c971aa73d3e);

    // Viterbi, rs, etc
    SatHelper::PacketFixer packetFixer;
    SatHelper::Viterbi27 viterbi(ENCODED_FRAME_SIZE / 2);
    SatHelper::DeRandomizer derand;
    SatHelper::ReedSolomon reedSolomon;

    // Other buffers
    uint8_t frameBuffer[1279];
    uint8_t rsWorkBuffer[255];

    // Lock mechanism stuff
    bool locked = false;
    int goodTimes = 0;
    int errors[5] = {-1, -1, -1, -1, -1};

    // Graphics
    std::cout << "---------------------------" << std::endl;
    std::cout << " Proba Decoder by Aang23" << std::endl;
    std::cout << "    S-Band 1 / 2 / V" << std::endl;
    std::cout << "  Based on libsathelper" << std::endl;
    std::cout << "      by Luigi Cruz" << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << std::endl;

    // Read until EOF
    while (!data_in.eof())
    {
        // Read buffer
        data_in.read((char *)buffer, BUFFER_SIZE);

        // Correlate less if we're locked to go faster
        if (!locked)
            correlator.correlate(buffer, ENCODED_FRAME_SIZE);
        {
            correlator.correlate(buffer, ENCODED_FRAME_SIZE / 64);
            if (correlator.getHighestCorrelationPosition() != 0)
            {
                correlator.correlate(buffer, ENCODED_FRAME_SIZE);
                if (correlator.getHighestCorrelationPosition() > 30)
                    locked = false;
            }
        }

        // Correlator statistics
        uint32_t cor = correlator.getHighestCorrelation();
        uint32_t word = correlator.getCorrelationWordNumber();
        uint32_t pos = correlator.getHighestCorrelationPosition();

        if (cor > 10)
        {
            if (pos != 0)
            {
                shiftWithConstantSize(buffer, pos, ENCODED_FRAME_SIZE);
                uint32_t offset = ENCODED_FRAME_SIZE - pos;
                uint8_t buffer_2[pos];

                data_in.read((char *)buffer_2, pos);

                for (int i = offset; i < ENCODED_FRAME_SIZE; i++)
                {
                    buffer[i] = buffer_2[i - offset];
                }
            }

            // Correct phase ambiguity
            packetFixer.fixPacket(buffer, ENCODED_FRAME_SIZE, word == 0 ? SatHelper::PhaseShift::DEG_90 : SatHelper::PhaseShift::DEG_270, true);

            // Viterbi
            viterbi.decode(buffer, frameBuffer);

            if (probav)
            {
                // Derandomize that frame
                derand.DeRandomize(&frameBuffer[4], FRAME_SIZE - 4);
            }

            // RS Correction
            for (int i = 0; i < 5; i++)
            {
                reedSolomon.deinterleave(&frameBuffer[4], rsWorkBuffer, i, 5);
                errors[i] = reedSolomon.decode_ccsds(rsWorkBuffer);
                reedSolomon.interleave(rsWorkBuffer, &frameBuffer[4], i, 5);
            }

            // Write it out if it's not garbage
            if (cor > 50)
                locked = true;

            if (locked)
            {
                data_out_total += FRAME_SIZE;
                data_out.put(0x1a);
                data_out.put(0xcf);
                data_out.put(0xfc);
                data_out.put(0x1d);
                data_out.write((char *)&frameBuffer[4], FRAME_SIZE - 4);
            }
        }
        else
        {
            locked = false;
            goodTimes = 0;
        }

        // Console stuff
        uint32_t current_asm = frameBuffer[0] << 24 |
                               frameBuffer[1] << 16 |
                               frameBuffer[2] << 8 |
                               frameBuffer[3];
        std::ios oldState(nullptr);
        oldState.copyfmt(std::cout);
        std::cout << '\r' << ("State : " + (std::string)(locked ? "SYNCED" : "NOSYNC")) << ", ASM : " << std::setfill('0') << std::setw(8) << std::hex << current_asm << ", CADUs : " << (float)(data_out_total / 1279) << ", Data out : " << round(data_out_total / 1e5) / 10.0f;
        std::cout.copyfmt(oldState);
        std::cout << " MB, RS : (" << errors[0] << "," << errors[1] << "," << errors[2] << "," << errors[3] << "," << errors[4] << "), Progress : ";
        std::cout << round(((float)data_in.tellg() / (float)filesize) * 1000.0f) / 10.0f << "%     " << std::flush;
    }

    std::cout << std::endl
              << "Done! Enjoy" << std::endl;

    data_in.close();
    data_out.close();
}