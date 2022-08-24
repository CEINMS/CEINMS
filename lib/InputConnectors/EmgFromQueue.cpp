#include "ceinms/EmgFromQueue.h"

namespace ceinms {

	void parseMaxEmgFile(const std::string& maxEmgFilename, std::vector<std::string>& emgNames, std::vector<double>& normalisationWeights) {
		std::ifstream iF(maxEmgFilename);
		std::string line;
		std::getline(iF, line); //garbage
		while (std::getline(iF, line)) {
			std::istringstream iss(line);
			std::string emgName, trial;
			double emgMax, time;

			if (!(iss >> emgName >> emgMax >> trial >> time)) { break; } // error

			emgNames.push_back(emgName);
			normalisationWeights.push_back(emgMax);
		}

		for (unsigned i(0); i < emgNames.size(); ++i)
			std::cout << emgNames.at(i) << " -> " << normalisationWeights.at(i) << std::endl;
	}

    void EmgFromQueue::operator()() {
        inputEmgQueue_.subscribe();
        EmgFromX::doneWithSubscriptions();

        while (auto emgFrame{inputEmgQueue_.pop()}) {
            double currentTime(emgFrame.value().time);
            for (unsigned i(0); i < emgFrame.value().data.size(); ++i) {
                emgFrame.value().data.at(i) = lowPassfilters_.at(i).filter(fabs(highPassfilters_.at(i).filter(emgFrame.value().data.at(i))), currentTime, fc_);
                if (autoWeightCalculation_) {
                    for (int i(0); i < emgFrame.value().data.size(); ++i) {
                        if (emgFrame.value().data[i] > normalisationWeights_[i]) {
                            normalisationWeights_[i] = emgFrame.value().data[i];
                        }
                    }
                }
                emgFrame.value().data.at(i) /= normalisationWeights_.at(i);
            }
            auto mappedEmg(emgMapper_.map(emgFrame.value().data));
            std::vector<double> excitations(nMuscles_);
            excitationGenerator_.convert(mappedEmg, excitations);
            EmgFromX::updateEmg(excitations, currentTime);
        }

        EmgFromX::sendEndOfData();
        EmgFromX::doneWithExecution();
    }
}