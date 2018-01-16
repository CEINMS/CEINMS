#include "ceinms/QueueToDataTable.h"
#include "ceinms/EndOfData.h"

namespace ceinms {

QueueToDataTable::QueueToDataTable(
    ScalarVectorQueue& inputQueue,
    Concurrency::Latch& doneWithSubscription,
    Concurrency::Latch& doneWithExecution,
    const std::vector<std::string>& columnLabels) :
    inputQueue_(inputQueue),
    doneWithSubscription_(doneWithSubscription),
    doneWithExecution_(doneWithExecution) {

    dataTable_.setLabels(columnLabels);
    
}

void QueueToDataTable::operator()() {

    inputQueue_.subscribe();
    doneWithSubscription_.wait();
    bool runCondition(true);
    while(runCondition) {
        auto frame(inputQueue_.pop());
        runCondition = !EndOfData::isEod(frame);
        if (runCondition) {
            dataTable_.pushRow(frame.time, frame.data);
        }
    }
    doneWithExecution_.wait();
}

const DataTable<double>& QueueToDataTable::getDataTable() const {

    return dataTable_;
}

}