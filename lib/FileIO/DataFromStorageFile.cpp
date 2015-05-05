//__________________________________________________________________________
// Author(s): Elena Ceseracciu - June 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <sstream>
using std::stringstream;
#include <stdlib.h>

#include "DataFromStorageFile.h"

namespace ceinms {
    DataFromStorageFile::DataFromStorageFile(const string& dataFilename)
        :dataFile_(dataFilename.c_str())
    {
        if (!dataFile_.is_open()) {
            cout << "ERROR: " << dataFilename << " could not be open\n";
            exit(EXIT_FAILURE);
        }

        dataFileName_ = dataFilename;

        string line;
        getline(dataFile_, line, '\n');
        while (line != "endheader" && line != "endheader\r" && !dataFile_.eof())
        {
            if (size_t foundInfo = line.find("nRows=") != string::npos)
            {
                noTimeSteps_ = atoi(line.substr(6).c_str());
            }
            if (size_t foundInfo = line.find("datarows ") != string::npos)
            {
                noTimeSteps_ = atoi(line.substr(9).c_str());
            }
            if (size_t foundInfo = line.find("nColumns=") != string::npos)
            {
                noColumns_ = atoi(line.substr(9).c_str()) - 1; //nColumns includes the "time" column
            }
            if (size_t foundInfo = line.find("datacolumns ") != string::npos)
            {
                noColumns_ = atoi(line.substr(12).c_str()) - 1; //nColumns includes the "time" column
            }
            getline((dataFile_), line, '\n');
        }
        // reading muscles
        getline((dataFile_), line, '\n');
        stringstream myStream(line);
        string nextColumnName;
        // the first is the "Time"
        string timeName;
        myStream >> timeName;
        // then we have their names 

        int noReadColumns = 0;
        do {
            myStream >> nextColumnName;
            columnNames_.push_back(nextColumnName);
        } while (!myStream.eof() && ++noReadColumns < noColumns_);


        if (noColumns_ != columnNames_.size()) {
            cout << "\nSomething is wrong in " << dataFileName_ << endl << noColumns_ << " muscles should be in the file "
                << "and we have : " << columnNames_.size() << endl;
            for (vector<string>::iterator it = columnNames_.begin(); it != columnNames_.end(); ++it)
                cout << *it << " -\n";
            exit(EXIT_FAILURE);
        }

        currentData_.resize(noColumns_);
        currentDataTime_ = 0.;
        currentTimeStep_ = 0;

    }

    void DataFromStorageFile::readNextData()  {

        // read time for the data currently stored in DataFromFile
        string line;
        getline(dataFile_, line, '\n');

        stringstream myStream(line);
        double value;
        currentData_.clear();
        //  cout << "\ndatafromfile check 1\n";
        myStream >> currentDataTime_;
        //    cout << "\ndatafromfile check 1b\n";
        int noReadColumns = 0;
        do {
            myStream >> value;
            //  cout << "datafromfile in " << dataFileName_  << ": value "<< value << endl;
            currentData_.push_back(value);
        } while (!myStream.eof() && ++noReadColumns < noColumns_);
        //    cout << "\ndatafromfile check 2\n";
        if (currentData_.size() != columnNames_.size())
        {
            cout << "\nERROR: in" << dataFileName_ << " at time step " << currentTimeStep_ << " you have " << currentData_.size() << " input.\nYou need " << columnNames_.size() << endl;
            exit(EXIT_FAILURE);
        }
        /*
          dataFile_ >>  currentDataTime_;
          // and then store their value
          for (int i = 0; i < noMuscles_; ++i) {
          double value;
          dataFile_ >> value;
          currentData_.at(i) = value;
          }
          */
        ++currentTimeStep_;
    }

    const vector<double>& DataFromStorageFile::getCurrentData() const
    {
        //  if(currentData_.size() == muscleNames_.size())
        return currentData_;
        /*  else
          {
          cout << "\nERROR: in" << dataFileName_ << " at time step " << currentTimeStep_ << " you have " << currentData_.size() << " input.\nYou need " << muscleNames_.size() << endl;
          exit(EXIT_FAILURE);
          }*/
    }


    DataFromStorageFile::~DataFromStorageFile() {
        (dataFile_).close();
    }
}