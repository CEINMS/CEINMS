//__________________________________________________________________________
// Author: Claudio Pizzolato - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include <ctime>
#include <chrono>

#include "CalibrationManager.h"


void printHeader() {

    cout << endl;
    cout << "+-+-+-+-+-+-+\n"                            
         << "|C|E|I|N|M|S|\n"
         << "+-+-+-+-+-+-+-+-+-+-+\n"                    
         << "|C|a|l|i|b|r|a|t|e|d|\n"                    
         << "+-+-+-+-+-+-+-+-+-+-+-+-+\n"                
         << "|E|M|G|-|I|n|f|o|r|m|e|d|\n"                
         << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n"
         << "|N|e|u|r|o|m|u|s|c|u|l|o|s|k|e|l|e|t|a|l|\n"
         << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n"
         << "|T|o|o|l|b|o|x|\n"                          
         << "+-+-+-+-+-+-+-+\n\n"
         << "Calibration\n\n";                   
        
    }
    
void printAuthors() {
    
    time_t now = time(0);
    tm *gmtm = gmtime(&now);
    cout << "Copyright (C) " << gmtm->tm_year+1900 << endl;
    cout << "Claudio Pizzolato, Monica Reggiani, Massimo Sartori, David Lloyd\n\n";
    
    cout << "Software developers: Claudio Pizzolato, Monica Reggiani\n";
}

void PrintUsage()
{
    string progName = "CEINMScalibrate";
    cout << "\n\n" << progName << ":\n";// << GetVersionAndDate() << "\n\n";
    cout << "Option            Argument          Action / Notes\n";
    cout << "------            --------          --------------\n";
    cout << "-Help, -H                           Print the command-line options for " << progName << ".\n";
    cout << "-PrintSetup, -PS                    Generates a template Setup file\n";
    cout << "-Setup, -S        SetupFileName     Specify an xml setup file.\n";

}


int main(int argc, char** argv){
    
    string option = "";
    string setupFileName;
    if (argc < 2) {
        PrintUsage();
        return 0;
    }
    else{
        int i;
        for (i = 1; i <= (argc - 1); i++) {
            option = argv[i];

            // PRINT THE USAGE OPTIONS
            if ((option == "-help") || (option == "-h") || (option == "-Help") || (option == "-H") ||
                (option == "-usage") || (option == "-u") || (option == "-Usage") || (option == "-U")) {
                PrintUsage();
                return 0;
            }
            else if ((option == "-S") || (option == "-Setup")) {
                if (argv[i + 1] == 0){
                    cout << "No setup file specified!" << endl;
                    PrintUsage();
                    return -1;
                }
                setupFileName = argv[i + 1];
                break;

                // Print a default setup file
            }
            else if ((option == "-PrintSetup") || (option == "-PS")) {
                if (CeinmsCalibrationSetupXmlReader::writeTemplateCeinmsCalibrationSetupFile("defaultCeinmsCalibrationSetupFile.xml"))
                {
                    std::cout << "Wrote template setup file to defaultCeinmsCalibrationSetupFile.xml" << std::endl;
                    return 0;
                }
                else
                {
                    std::cout << "An error occurred while writing template setup file to defaultCeinmsSetupFIle.xml" << std::endl;
                    return -1;
                }
            }
            else {
                cout << "Unrecognized option " << option << " on command line... Ignored" << endl;
                PrintUsage();
                return -1;
            }
        }
    }

    CEINMS::CalibrationManager calibrationManager(setupFileName);
    auto timeBegin = std::chrono::high_resolution_clock::now();
    bool exitflag = calibrationManager.run();
    auto timeEnd = std::chrono::high_resolution_clock::now();
    std::cout << "Calibration time: " << std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeBegin).count() << "ms" << std::endl;

    std::exit(exitflag);
}

























