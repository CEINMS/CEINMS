#include <algorithm>
#include <stdexcept>

namespace CEINMS {

    template <typename T>
    DataTable<T>::DataTable() :
        nRows_(0), nCols_(0) {}

    template <typename T>
    DataTable<T>::DataTable(size_t nRows, size_t nCols) :
        nRows_(nRows), nCols_(nCols), data_(std::vector<std::vector<T>>(nRows, std::vector<T>(nCols, 0))) {

    }

    template <typename T>
    void DataTable<T>::setLabels(const std::vector<std::string>& labels) {

        if (!data_.empty() && labels.size() != nCols_)
            throw std::invalid_argument("wrong number of labels.");
        labels_ = labels;
    }


    template <typename T>
    void DataTable<T>::set(T value, size_t row, size_t col) {

        data_.at(row).at(col) = value;
    }


    template <typename T>
    const T& DataTable<T>::at(size_t row, size_t col) const {

        return data_.at(row).at(col);
    }

    template <typename T>
    T& DataTable<T>::at(size_t row, size_t col) {

        return data_.at(row).at(col);
    }


    template <typename T>
    void DataTable<T>::pushRow(T time, const std::vector<T>& values){

        if (values.size() != nCols_ && !data_.empty())
            throw std::invalid_argument("pushRow: Wrong size of values");

        data_.emplace_back(values);
        nRows_++;
        nCols_ = values.size();
        time_.emplace_back(time);
    }


    template <typename T>
    void DataTable<T>::setRow(size_t row, const std::vector<T>& values) {

        if (values.size() != nCols_)
            throw std::invalid_argument("setRow: Wrong size of values");
        data_.at(row) = values;

    }

    template <typename T>
    std::vector<T> DataTable<T>::getRow(size_t row) const {

        return data_.at(row);
    }

    template <typename T>
    std::vector<T> DataTable<T>::getColumn(size_t col) const {

        std::vector<T> column;
        column.reserve(nRows_);
        for (auto& row : data_)
            column.emplace_back(row.at(col));
        return column;
    }

    template <typename T>
    DataTable<T>  DataTable<T>::sum(const DataTable<T>& lhs, const DataTable<T>& rhs) {

        if (lhs.getNColumns() != rhs.getNColumns() || lhs.getNColumns() != rhs.getNColumns())
            throw std::invalid_argument("lhs and rhs differ in size");  //check just on data size.. ignore labels and time column

        DataTable<T> ans(lhs);
        size_t nRows(ans.getNRows());
        for (size_t row(0); row < nRows; ++row)
            std::transform(lhs.data_.at(row).begin(), lhs.data_.at(row).end(), rhs.data_.at(row).begin(), ans.data_.at(row).begin(), [](T l, T r) {
            return l + r;
        });
        return ans;
    }

    template <typename T>
    DataTable<T>  DataTable<T>::subtract(const DataTable<T>& lhs, const DataTable<T>& rhs) {

        if (lhs.getNColumns() != rhs.getNColumns() || lhs.getNColumns() != rhs.getNColumns())
            throw std::invalid_argument("lhs and rhs differ in size");  //check just on data size.. ignore labels and time column

        DataTable<T> ans(lhs);
        size_t nRows(ans.getNRows());
        for (size_t row(0); row < nRows; ++row)
            std::transform(lhs.data_.at(row).begin(), lhs.data_.at(row).end(), rhs.data_.at(row).begin(), ans.data_.at(row).begin(), [](T l, T r) {
            return l - r;
        });
        return ans;
    }

    template<typename T>
    bool DataTable<T>::equals(const DataTable<T>& rhs) const {
        return(data_   == rhs.data_   &&
               labels_ == rhs.labels_ &&
               time_   == rhs.time_   &&
               nCols_  == rhs.nCols_  &&
               nRows_  == rhs.nRows_);


    }

    template<typename T>
    std::ostream& operator<< (std::ostream & out, const DataTable<T> & rhs) {

        out << "DataTable " << rhs.nRows_ << "x" << rhs.nCols_ << std::endl;
        out << rhs.labels_.size() << " labels" << std::endl;
        for (auto &l : rhs.labels_)
            out << l << std::endl;
        out << "Time column" << std::endl;
        for (auto &t : rhs.time_)
            out << t << std::endl;
        out << "Data matrix" << std::endl;
        for (auto i(0); i < rhs.nRows_; ++i) {
            for (auto &e : rhs.data_.at(i))
                out << e << "\t";
            out << std::endl;

        }
        return out;
    }


    template<typename T>
    bool operator==(const DataTable<T>& lhs, const DataTable<T>& rhs) {
        return lhs.equals(rhs);
    }

    template<typename T>
    DataTable<T> operator+ (const DataTable<T>& lhs, const DataTable<T>& rhs) {
        return DataTable<T>::sum(lhs, rhs);
    }


    template<typename T>
    DataTable<T> operator- (const DataTable<T>& lhs, const DataTable<T>& rhs) {
        return DataTable<T>::subtract(lhs, rhs);
    }



}