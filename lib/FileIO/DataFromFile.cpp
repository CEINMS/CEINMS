/* -------------------------------------------------------------------------- *
 * CEINMS is a standalone toolbox for neuromusculoskeletal modelling and      *
 * simulation. CEINMS can also be used as a plugin for OpenSim either         *
 * through the OpenSim GUI or API. See https://simtk.org/home/ceinms and the  *
 * NOTICE file for more information. CEINMS development was coordinated       *
 * through Griffith University and supported by the Australian National       *
 * Health and Medical Research Council (NHMRC), the US National Institutes of *
 * Health (NIH), and the European Union Framework Programme 7 (EU FP7). Also  *
 * see the PROJECTS file for more information about the funding projects.     *
 *                                                                            *
 * Copyright (c) 2010-2015 Griffith University and the Contributors           *
 *                                                                            *
 * CEINMS Contributors: C. Pizzolato, M. Reggiani, M. Sartori,                *
 *                      E. Ceseracciu, and D.G. Lloyd                         *
 *                                                                            *
 * Author(s): C. Pizzolato, M. Reggiani                                       *
 *                                                                            *
 * CEINMS is licensed under the Apache License, Version 2.0 (the "License").  *
 * You may not use this file except in compliance with the License. You may   *
 * obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.*
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

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

#include "ceinms/DataFromFile.h"

//DataFromFile::DataFromFile(const DataFromFile& orig)
//:dataFile_(orig.dataFilename_.c_str()) {

//  if (!dataFile_.is_open()) {
//    cout << "ERROR: " << dataFilename << " could not be open\n";
//    exit(EXIT_FAILURE);
//  }
//  // and then you open it because ifstream cannot be used with copy constructor and operator =
//  dataFileName_ = orig.dataFileName_;
//  noMuscles_ = orig.noMuscles_;
//  muscleNames_ = orig.muscleNames_;
//  noTimeSteps_ = orig.noTimeSteps_;
//  currentDataTime_ = orig.currentDataTime_;
//  currentData_ = orig.currentData_;
//  currentTimeStep_ = orig.currentTimeStep_;
//}
namespace ceinms {
    DataFromFile::DataFromFile(const string& dataFilename)
        :dataFile_(dataFilename.c_str()) {
        if (!dataFile_.is_open()) {
            cout << "ERROR: " << dataFilename << " could not be open\n";
            exit(EXIT_FAILURE);
        }

        dataFileName_ = dataFilename;

        // reading number of columns/rows
        string trash;
        dataFile_ >> trash;
        int noColumns;
        dataFile_ >> noColumns;
        noMuscles_ = noColumns - 1;
        dataFile_ >> trash;
        dataFile_ >> noTimeSteps_;

        // reading muscles
        string line;
        getline(dataFile_, line, '\n'); getline(dataFile_, line, '\n');
        stringstream myStream(line);
        string nextMuscleName;
        // the first is the "Time"
        string timeName;
        myStream >> timeName;
        // then we have their names

        int noReadMuscles = 0;
        do {
            myStream >> nextMuscleName;
            muscleNames_.push_back(nextMuscleName);
        } while (!myStream.eof() && ++noReadMuscles < noMuscles_);


        if (noMuscles_ != muscleNames_.size()) {
            cout << "\nSomething is wrong in " << dataFileName_ << endl << noMuscles_ << " muscles should be in the file "
                << "and we have : " << muscleNames_.size() << endl;
            for (vector<string>::iterator it = muscleNames_.begin(); it != muscleNames_.end(); ++it)
                cout << *it << " -\n";
            exit(EXIT_FAILURE);
        }

        currentData_.resize(noMuscles_);
        currentDataTime_ = 0.;
        currentTimeStep_ = 0;

    }

    void DataFromFile::readNextData()  {

        // read time for the data currently stored in DataFromFile
        string line;
        getline(dataFile_, line, '\n');
        stringstream myStream(line);
        double value;
        currentData_.clear();
        //  cout << "\ndatafromfile check 1\n";
        myStream >> currentDataTime_;
        //    cout << "\ndatafromfile check 1b\n";
        int noReadMuscles = 0;
        do {
            myStream >> value;
            //  cout << "datafromfile in " << dataFileName_  << ": value "<< value << endl;
            currentData_.push_back(value);
        } while (!myStream.eof() && ++noReadMuscles < noMuscles_);
        //    cout << "\ndatafromfile check 2\n";
        if (currentData_.size() != muscleNames_.size())
        {
            cout << "\nERROR: in" << dataFileName_ << " at time step " << currentTimeStep_ << " you have " << currentData_.size() << " input.\nYou need " << muscleNames_.size() << endl;
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

    const vector<double>& DataFromFile::getCurrentData() const
    {
        //  if(currentData_.size() == muscleNames_.size())
        return currentData_;
        /*  else
          {
          cout << "\nERROR: in" << dataFileName_ << " at time step " << currentTimeStep_ << " you have " << currentData_.size() << " input.\nYou need " << muscleNames_.size() << endl;
          exit(EXIT_FAILURE);
          }*/
    }


    DataFromFile::~DataFromFile() {
        dataFile_.close();
    }
}
