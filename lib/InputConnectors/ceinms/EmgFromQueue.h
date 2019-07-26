#ifndef ceinms_EmgsFromQueue_h
#define ceinms_EmgsFromQueue_h
#include <ceinms/EmgFromX.h>
#include <ceinms/concurrency/Latch.h>
#include <ceinms/concurrency/Queue.h>
#include <ceinms/queues/EmgQueue.h>
#include <ceinms/ExcitationGeneratorFromXml.h>
#include <vector>
#include <string>
#include <rtb/Filter/StateSpaceFilter.h>
#include <rtb/Filter/Filter.h>
#include <ceinms/Mapper.h>
#include <iostream>
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
			ceinms::Concurrency::Latch& doneWithSubscriptions,
			ceinms::Concurrency::Latch& doneWithExecution,
			const std::vector<std::string>& inputEmgNames,
			const std::string& excitationsGeneratorFilename,
			const NMSModelT& subject,
			double fc,
			double fs);

        template<typename NMSModelT>
        EmgFromQueue(
            ceinms::EmgQueue& inputEmgQueue,
            ceinms::EmgQueue& outputEmgQueue,
            ceinms::Concurrency::Latch& doneWithSubscriptions,
            ceinms::Concurrency::Latch& doneWithExecution,
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
            ceinms::Concurrency::Latch& doneWithSubscriptions,
            ceinms::Concurrency::Latch& doneWithExecution,
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
		ceinms::Concurrency::Latch& doneWithSubscriptions,
		ceinms::Concurrency::Latch& doneWithExecution,
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
        ceinms::Concurrency::Latch& doneWithSubscriptions,
        ceinms::Concurrency::Latch& doneWithExecution,
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
            for (auto& name : inputEmgNames)
                std::cout << name << std::endl;

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
        ceinms::Concurrency::Latch& doneWithSubscriptions,
        ceinms::Concurrency::Latch& doneWithExecution,
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
            for (auto& name : inputEmgNames_)
                std::cout << name << std::endl;

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
