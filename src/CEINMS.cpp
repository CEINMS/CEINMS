//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "Utilities.h"
#include "FileUtils.h"
#include "CeinmsSetupXmlReader.h"
#include "SimulationManager.h"
#include <ctime>

#include <iomanip>

#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <map>
using std::map;
#include <stdlib.h>


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
         << "+-+-+-+-+-+-+-+\n\n";                   
        
    }
    
void printAuthors() {
    
    time_t now = std::time(0);
    tm *gmtm = std::gmtime(&now);
    cout << "Copyright (C) " << gmtm->tm_year+1900 << endl;
    cout << "Claudio Pizzolato, Monica Reggiani, David Lloyd\n\n";
    
    cout << "Software developers: Claudio Pizzolato, Elena Ceseracciu, Monica Reggiani\n";
}

void PrintUsage()
{
    string progName= "CEINMS";
    cout << "\n\n" << progName << ":\n";// << GetVersionAndDate() << "\n\n";
    cout << "Option            Argument          Action / Notes\n";
    cout << "------            --------          --------------\n";
    cout << "-Help, -H                           Print the command-line options for " << progName << ".\n";
    cout << "-PrintSetup, -PS                    Generates a template Setup file\n";
    cout << "-Setup, -S        SetupFileName     Specify an xml setup file.\n";

}


int main(int argc, char** argv) {
 
    printHeader();
    printAuthors();
    
#ifdef LOG  
  cout << "Check configuration data...\n";
#endif

    string option="";
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
                if (ceinms::CeinmsSetupXmlReader::writeTemplateCeinmsSetupFile("defaultCeinmsSetupFile.xml"))
                {
                    std::cout << "Wrote template setup file to defaultCeinmsSetupFile.xml" << std::endl;
                    return 0;
                }
                else
                {
                    std::cout << "An error occurred while writing template setup file to defaultCeinmsSetupFile.xml" << std::endl;
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
    
    ceinms::SimulationManager simulation(setupFileName);
    bool flag = simulation.run();

    return flag;
}
