#include "ceinms/EndOfData.h"

namespace ceinms {

template<typename DataType>
QueueToDataTable<DataType>::QueueToDataTable(
    QueueType& inputQueue,
    rtb::Concurrency::Latch& doneWithSubscription,
    rtb::Concurrency::Latch& doneWithExecution,
    const std::vector<std::string>& columnLabels) :
    inputQueue_(inputQueue),
    doneWithSubscription_(doneWithSubscription),
    doneWithExecution_(doneWithExecution) {

    dataTable_.setLabels(columnLabels);
    
}

template<typename DataType>
void QueueToDataTable<DataType>::operator()() {

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


template<typename DataType>
const DataTable<typename QueueToDataTable<DataType>::ValueType>& 
QueueToDataTable<DataType>::getDataTable() const {

    return dataTable_;
}

}