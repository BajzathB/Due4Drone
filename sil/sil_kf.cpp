#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <algorithm>

#include "../src/Controller.h"

int parseFiles(std::string& inputFile, std::string& outputFile,
    std::ifstream& input, std::ofstream& output)
{
    if (inputFile.empty() || outputFile.empty())
    {
        std::cerr << "Input and output files are required.\n";
        return 1;
    }

    // Open input file
    input = std::ifstream(inputFile);

    if (!input.is_open())
    {
        std::cerr << "Cannot open input file: "
                  << inputFile << '\n';
        return 1;
    }

    // Create output directory if necessary
    std::filesystem::path outputPath(outputFile);

    if (!outputPath.parent_path().empty())
    {
        std::filesystem::create_directories(
            outputPath.parent_path()
        );
    }

    // Open output file
    output = std::ofstream(outputFile);

    if (!output.is_open())
    {
        std::cerr << "Cannot create output file: "
                  << outputFile << '\n';
        return 1;
    }

    std::cout << "Input:  " << inputFile << '\n';
    std::cout << "Output: " << outputFile << '\n';

    return 0;
}

void parseMeasInitLines(std::ifstream& input, std::ofstream& output, kfAngle2d_st& kf)
{
    std::string line;

    //line1: R
    std::getline(input, line);
    output << line << "\n";
    //line2: header names
    std::getline(input, line);
    output << line << "\n";
    //line3: header values
    std::getline(input, line);
    std::replace(line.begin(), line.end(), ',', ' ');
    uint32_t tickMs, cycleMs;
    uint32_t px, ix, dx, py, iy, dy, pz, iz, dz;
    uint32_t ffrx, ffdrx, ffry, ffdry;
    uint32_t satI, satPID;
    uint32_t KFQAng, KFQbias, KFRmeas;
    uint32_t CPx, CIx, CPy, CIy, CsatI, CsatPID;
    uint32_t CFFdrx,CFFdry, IRelaxX,IRelaxY,DmaxR,DmaxE,DMx,DMy;
    std::istringstream iss3(line);
    iss3 >> tickMs >> cycleMs >> px >> ix >> dx >> py >> iy >> dy >> pz >> iz
         >> ffrx >> ffry >> ffdrx >> ffdry >> satI >> satPID >> KFQAng >> KFQbias
         >> KFRmeas >> CPx >> CIx >> CPy >> CIy >> CsatI >> CsatPID >> CFFdrx
         >> CFFdry >> IRelaxX >> IRelaxY >> DmaxR >> DmaxE >> DMx >> DMy;

    std::cout << "Kalman Filter parameters before -> after:\n"
        << "\tqAngel: " << KFQAng << " -> " << kf.qAngleTick << "\n"
        << "\tqBias: " << KFQbias << " -> " << kf.qBiasTick << "\n"
        << "\trMeas: " << KFRmeas << " -> " << kf.rMeasTick << "\n";

    std::ostringstream line3_updated;
    line3_updated << tickMs << "," << cycleMs << "," << px << "," << ix << "," << dx << ","
        << py << "," << iy << "," << dy << "," << pz << "," << iz << "," << ffrx << "," << ffry << ","
        << ffdrx << "," << ffdry << "," << satI << "," << satPID << "," << kf.qAngleTick << ","
        << kf.qBiasTick << "," << kf.rMeasTick << "," << CPx << "," << CIx << "," << CPy << ","
        << CIy << "," << CsatI << "," << CsatPID << "," << CFFdrx << "," << CFFdry << ","
        << IRelaxX << "," << IRelaxY << "," << DmaxR << "," << DmaxE << "," << DMx << "," << DMy;
    output << line3_updated.str() << "\n";
    //line4: meas names
    std::getline(input, line);
    output << line << "\n";
}


void parseMeasLine(std::string& line, int32_t& gyro, int32_t& accAngle, int32_t& kfAngle)
{
    std::istringstream line_in;
    std::replace(line.begin(), line.end(), ',', ' ');
    line_in = std::istringstream(line);
    line_in >> gyro >> accAngle >> kfAngle;
}

int main(int argc, char* argv[])
{
    // Check arguments
    if (argc != 11)
    {
        std::cerr << "Usage: " << argv[0]
                  << " --input <input.txt>"
                  << " --output <output.csv>"
                  << " --qAng <qAngle>"
                  << " --qBias <qBias>"
                  << " --rMeas <rMeasurement>\n";
        return 1;
    }

    std::string inputFile;
    std::string outputFile;
    std::ifstream input;
    std::ofstream output;
    kfAngle2d_st kf2d;

    // Parse arguments
    for (int i = 1; i < argc; i += 2)
    {
        std::string arg = argv[i];

        if (arg == "--input")
        {
            inputFile = argv[i + 1];
        }
        else if (arg == "--output")
        {
            outputFile = argv[i + 1];
        }
        else if (arg == "--qAng")
        {
            kf2d.qAngleTick = static_cast<uint32_t>(std::stoul(argv[i + 1]));
        }
        else if (arg == "--qBias")
        {
            kf2d.qBiasTick = static_cast<uint32_t>(std::stoul(argv[i + 1]));
        }
        else if (arg == "--rMeas")
        {
            kf2d.rMeasTick = static_cast<uint32_t>(std::stoul(argv[i + 1]));
        }
        else
        {
            std::cerr << "Unknown argument: " << arg << '\n';
            return 1;
        }
    }

    if(0 != parseFiles(inputFile, outputFile, input, output))
    {
        return 1;
    }

    parseMeasInitLines(input, output, kf2d);

    std::string line;
    std::istringstream line_in;
    std::ostringstream line_out;
    int32_t currGyroPT1, currAccAnglePT1, currKfAnglePT11;
    int32_t diffGyroPT1, diffAccAnglePT1, diffKfAnglePT11;
    int32_t lastKfAngle;
    uint32_t ctr;
    
    //line5: parse global values
    std::getline(input, line);
    parseMeasLine(line, currGyroPT1, currAccAnglePT1, currKfAnglePT11);
    output << line;

    while (std::getline(input, line))
    {
        if (line.empty() || line[0] == '#')
        {
            std::cout << "counter value at skip: " << ctr << "\n";
            continue;
        }

        //line: parse diff values
        parseMeasLine(line, diffGyroPT1, diffAccAnglePT1, diffKfAnglePT11);
        //add diff to current global
        currGyroPT1 += diffGyroPT1;
        currAccAnglePT1 += diffAccAnglePT1;
        currKfAnglePT11 += diffKfAnglePT11;
        //KF
        CalcKFAngle(&kf2d.roll, currAccAnglePT1, currGyroPT1, 5250, true);
        //save to output
        output << "\n" << diffGyroPT1 << "," << diffAccAnglePT1 << "," << kf2d.roll.angle-lastKfAngle;

        // std::cout << "Line: " << line << "\n";
        // std::cout << "Parsed values: " << currGyroPT1*2000/32768 << ", " << currAccAnglePT1*30/78747 << ", " << kf2d.roll.angle*30/78747 << "\n";

        lastKfAngle = kf2d.roll.angle;
        ctr++;
    }

    std::cout << "SIL completed, parsed lines: " << ctr << "\n";

    return 0;
}