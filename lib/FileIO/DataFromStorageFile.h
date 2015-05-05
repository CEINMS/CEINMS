//__________________________________________________________________________
// Author(s): Elena Ceseracciu - June 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_DataFromStorageFile_h
#define ceinms_DataFromStorageFile_h

#include <fstream>
#include <iostream>
#include <vector>
#include <memory>

/**
 * \brief This class read from a file either lmt or ma data
 * You should have a file that includes 
 * write here the structure of the file
 */
namespace ceinms {
    class DataFromStorageFile {
    public:
        DataFromStorageFile() = delete;
        DataFromStorageFile(const std::string& inputFile);
        const std::vector<std::string>& getColumnNames() const { return columnNames_; }
        void readNextData();
        int getNoTimeSteps() const { return noTimeSteps_; }
        bool areStillData() const { return currentTimeStep_ < noTimeSteps_; }
        inline double getCurrentTime() const { return currentDataTime_; }
        const std::vector<double>& getCurrentData() const;
        ~DataFromStorageFile();
        DataFromStorageFile(const DataFromStorageFile& orig) = delete;
        DataFromStorageFile& operator=(const DataFromStorageFile& orig) = delete;
    private:
        std::string dataFileName_;
        std::ifstream dataFile_;
        unsigned int noColumns_;
        std::vector<std::string> columnNames_;
        int currentTimeStep_;
        int noTimeSteps_;
        double currentDataTime_;
        std::vector<double> currentData_;
    };
}
#endif
