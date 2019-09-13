#ifndef ceinms_EmgsFromQueue_h
#define ceinms_EmgsFromQueue_h
#include <ceinms/EmgFromX.h>
#include <rtb/concurrency/Concurrency.h>
#include <ceinms/queues/EmgQueue.h>
#include <ceinms/ExcitationGeneratorFromXml.h>
#include <vector>
#include <string>
#include <rtb/Filter/StateSpaceFilter.h>
#include <rtb/Filter/Filter.h>
#include <ceinms/Mapper.h>
#include <iostream>
#include <iomanip>
#include "Utilities.h"
#include <fstream>

namespace ceinms {

	void parseMaxEmgFile(const std::string& maxEmgFilename, std::vector<std::string>& emgNames,
		std::vector<double>& normalisationWeights);

    class EmgFromQueue : public ceinms::EmgFromX{

    public:

		/*If not normalisation file is specified, then weights are calculated
		automatically*/
		template<typename NMSModelT>
		EmgFromQueue(
			ceinms::EmgQueue& inputEmgQueue,
			ceinms::EmgQueue& outputEmgQueue,
			rtb::Concurrency::Latch& doneWithSubscriptions,
			rtb::Concurrency::Latch& doneWithExecution,
			const std::vector<std::string>& inputEmgNames,
			const std::string& excitationsGeneratorFilename,
			const NMSModelT& subject,
			double fc,
			double fs);

        template<typename NMSModelT>
        EmgFromQueue(
            ceinms::EmgQueue& inputEmgQueue,
            ceinms::EmgQueue& outputEmgQueue,
            rtb::Concurrency::Latch& doneWithSubscriptions,
            rtb::Concurrency::Latch& doneWithExecution,
            const std::vector<std::string>& inputEmgNames,
            const std::vector<double>& normalisationWeights,
            const std::string& excitationsGeneratorFilename,
            const NMSModelT& subject,
            double fc,
			double fs);

        template<typename NMSModelT>
        EmgFromQueue(
            ceinms::EmgQueue& inputEmgQueue,
            ceinms::EmgQueue& outputEmgQueue,
            rtb::Concurrency::Latch& doneWithSubscriptions,
            rtb::Concurrency::Latch& doneWithExecution,
            const std::string& maxEmgFilename,
            const std::string& excitationsGeneratorFilename,
            const NMSModelT& subject,
            double fc,
			double fs);
        void operator()() override;
        std::vector<std::string> getOutputLabels() const { return excitationGenerator_.getExcitationsNames(); }

    private:

        ceinms::EmgQueue& inputEmgQueue_;
        std::vector<std::string> inputEmgNames_;
        std::vector<double> normalisationWeights_;
        ceinms::ExcitationGeneratorFromXml excitationGenerator_;
        std::vector<rtb::Filter::StateSpaceFilter<double>> lowPassfilters_;
        std::vector<rtb::Filter::Filter<double>> highPassfilters_;

        double fc_;
        ceinms::Mapper emgMapper_;
        size_t nMuscles_;
		bool autoWeightCalculation_;
    };


	template<typename NMSModelT>
	EmgFromQueue::EmgFromQueue(
		ceinms::EmgQueue& inputEmgQueue,
		ceinms::EmgQueue& outputEmgQueue,
		rtb::Concurrency::Latch& doneWithSubscriptions,
		rtb::Concurrency::Latch& doneWithExecution,
		const std::vector<std::string>& inputEmgNames,
		const std::string& excitationsGeneratorFilename,
		const NMSModelT& subject,
		double fc,
		double fs) :
		EmgFromQueue::EmgFromQueue(
			inputEmgQueue,
			outputEmgQueue,
			doneWithSubscriptions,
			doneWithExecution,
			inputEmgNames,
			std::vector<double>(inputEmgNames.size(), 1.),
			excitationsGeneratorFilename,
			subject,
			fc, fs)
	{
		autoWeightCalculation_ = true;
	}



    template<typename NMSModelT>
    EmgFromQueue::EmgFromQueue(
        ceinms::EmgQueue& inputEmgQueue,
        ceinms::EmgQueue& outputEmgQueue,
        rtb::Concurrency::Latch& doneWithSubscriptions,
        rtb::Concurrency::Latch& doneWithExecution,
        const std::vector<std::string>& inputEmgNames,
        const std::vector<double>& normalisationWeights,
        const std::string& excitationsGeneratorFilename,
        const NMSModelT& subject,
        double fc, double fs) :
        ceinms::EmgFromX(outputEmgQueue, doneWithSubscriptions, doneWithExecution, subject),
        inputEmgQueue_(inputEmgQueue),
        inputEmgNames_(inputEmgNames),
        normalisationWeights_(normalisationWeights),
        excitationGenerator_(excitationsGeneratorFilename),
        fc_(fc),
        lowPassfilters_(inputEmgNames.size()),
        highPassfilters_(inputEmgNames.size()),
		autoWeightCalculation_(false) {

        emgMapper_.setNames(inputEmgNames_, excitationGenerator_.getInputSignalsNames());
        if (!excitationGenerator_.checkInputSignalsNames(emgMapper_.map(inputEmgNames_))) {
            std::cout << "Different names between excitation generator and subject" << std::endl;
            auto inputSignalsNames = excitationGenerator_.getInputSignalsNames();

			auto center([](const string s, const int w) {
				std::stringstream ss, spaces;
				int padding = w - s.size();                 // count excess room to pad
				for (int i = 0; i < padding / 2; ++i)
					spaces << " ";
				ss << spaces.str() << s << spaces.str();    // format with padding
				if (padding > 0 && padding % 2 != 0)               // if odd #, add 1 space
					ss << " ";
				return ss.str();
			});
			std::cout << center("Subject", 20) << " | "
				<< center("Generator", 20) <<  "\n";
			
			for (int i(0); i < std::max(inputEmgNames.size(), inputSignalsNames.size()); ++i) {
				std::string emgName("");
				if(i < inputEmgNames.size())
					emgName = inputEmgNames.at(i);
				std::string genName("");
				if (i < inputSignalsNames.size())
					genName = inputSignalsNames.at(i);

				std::cout << center(emgName, 20) << " | "
					<< center(genName, 20) << "\n";
			}
			std::cout << std::flush;
            
            std::exit(EXIT_FAILURE);
        }

        const std::vector<double> hpB{
            0.87518309243813475,
            -1.7503661848762695,
            0.87518309243813475
        };

        const std::vector<double> hpA{
            1,
            -1.734725768809275,
            0.76600660094326378
        };

        rtb::Filter::TransferFunction<> hpTF(hpB, hpA, fs);
        for (auto& f : highPassfilters_)
            f.setTransferFunction(hpTF);

        nMuscles_ = musclesNames_.size();
    }

    template<typename NMSModelT>
    EmgFromQueue::EmgFromQueue(
        ceinms::EmgQueue& inputEmgQueue,
        ceinms::EmgQueue& outputEmgQueue,
        rtb::Concurrency::Latch& doneWithSubscriptions,
        rtb::Concurrency::Latch& doneWithExecution,
        const std::string& maxEmgFilename,
        const std::string& excitationsGeneratorFilename,
        const NMSModelT& subject,
        double fc,
		double fs) :
        ceinms::EmgFromX(outputEmgQueue, doneWithSubscriptions, doneWithExecution, subject),
        inputEmgQueue_(inputEmgQueue),
        excitationGenerator_(excitationsGeneratorFilename),
        fc_(fc),
		autoWeightCalculation_(false) {

        parseMaxEmgFile(maxEmgFilename, inputEmgNames_, normalisationWeights_);
        lowPassfilters_.resize(inputEmgNames_.size());
        highPassfilters_.resize(inputEmgNames_.size());
        emgMapper_.setNames(inputEmgNames_, excitationGenerator_.getInputSignalsNames());

        if (!excitationGenerator_.checkInputSignalsNames(emgMapper_.map(inputEmgNames_))) {
			std::cout << "Different names between excitation generator and subject" << std::endl;
			auto inputSignalsNames = excitationGenerator_.getInputSignalsNames();

			auto center([](const string s, const int w) {
				std::stringstream ss, spaces;
				int padding = w - s.size();                 // count excess room to pad
				for (int i = 0; i < padding / 2; ++i)
					spaces << " ";
				ss << spaces.str() << s << spaces.str();    // format with padding
				if (padding > 0 && padding % 2 != 0)               // if odd #, add 1 space
					ss << " ";
				return ss.str();
			});
			std::cout << center("Subject", 20) << " | "
				<< center("Generator", 20) << "\n";

			for (int i(0); i < std::max(inputEmgNames_.size(), inputEmgNames_.size()); ++i) {
				std::string emgName("");
				if (i < inputEmgNames_.size())
					emgName = inputEmgNames_.at(i);
				std::string genName("");
				if (i < inputSignalsNames.size())
					genName = inputSignalsNames.at(i);

				std::cout << center(emgName, 20) << " | "
					<< center(genName, 20) << "\n";
			}
			std::cout << std::flush;
            std::exit(EXIT_FAILURE);
        }
        nMuscles_ = musclesNames_.size();

        const std::vector<double> hpB{
            0.87518309243813475,
            -1.7503661848762695,
            0.87518309243813475
        };

        const std::vector<double> hpA{
            1,
            -1.734725768809275,
            0.76600660094326378
        };

        rtb::Filter::TransferFunction<> hpTF(hpB, hpA, fs);
        for (auto& f : highPassfilters_)
            f.setTransferFunction(hpTF);
    }
}

#endif
