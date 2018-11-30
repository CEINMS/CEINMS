#include "ceinms/EmgFromQueue.h"
#include "ceinms/EndOfData.h"

namespace ceinms {

    void EmgFromQueue::operator()() {
        inputEmgQueue_.subscribe();
        EmgFromX::doneWithSubscriptions();

        bool runCondition  = true;
        while (runCondition) {

            auto emgFrame(inputEmgQueue_.pop());
            if (ceinms::EndOfData::isEod(emgFrame))
                runCondition = false;
            else {
                double currentTime(emgFrame.time);

                for (unsigned i(0); i < emgFrame.data.size(); ++i) {
                    emgFrame.data.at(i) = lowPassfilters_.at(i).filter(fabs(highPassfilters_.at(i).filter(emgFrame.data.at(i))), currentTime, fc_);
					if (autoWeightCalculation_) {
						for (int i(0); i < emgFrame.data.size(); ++i) {
							if (emgFrame.data[i] > normalisationWeights_[i]) {
								normalisationWeights_[i] = emgFrame.data[i];
							}
						}
					}
                    emgFrame.data.at(i) /= normalisationWeights_.at(i);
                }
                auto mappedEmg(emgMapper_.map(emgFrame.data));
                std::vector<double> excitations(nMuscles_);
                excitationGenerator_.convert(mappedEmg, excitations);
                EmgFromX::updateEmg(excitations, currentTime);
            }
        }

        EmgFromX::sendEndOfData();
        EmgFromX::doneWithExecution();
    }
}