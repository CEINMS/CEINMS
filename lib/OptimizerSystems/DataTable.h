#ifndef Ceinms_DataTable_h
#define Ceinms_DataTable_h
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
namespace CEINMS {
    template<typename T>
    class DataTable {

    public:
        DataTable();
        DataTable(size_t nRows, size_t nCols); //do not include time column
        void setLabels(const std::vector<std::string>& labels); //do not include time column
        void set(T value, size_t row, size_t col);
        const T& at(size_t row, size_t col) const;
        T& at(size_t row, size_t col);
        void pushRow(T time, const std::vector<T>& values);
        void setRow(size_t row, const std::vector<T>& values);
        
        static DataTable<T> sum(const DataTable<T>& lhs, const DataTable<T>& rhs);
        static DataTable<T> subtract(const DataTable<T>& lhs, const DataTable<T>& rhs);

        std::string getLabel(size_t col) const { return labels_.at(col); }
        T get(size_t row, size_t col) const { return data_.at(row, col); }
        T getTime(size_t row) const { return time_.at(row); }
        T getStartTime() const { return time_.front() }
        T getFinalTIme() const { return time_.back(); }
        std::vector<T> getTimeColumn(size_t col) const { return time_; }
        std::vector<T> getRow(size_t row) const;
        std::vector<T> getColumn(size_t col) const;
        size_t getNColumns() const { return nCols_ };
        size_t getNRows() const { return nRows_ }

    private:
        size_t nRows_, nCols_;
        //internal implementation may be changed to something better.. e.g. nice matrices
        std::vector<std::vector<T>> data_;
        std::vector<T> time_;
        std::vector<std::string> labels_;


    };


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
            throw std::invalid_argument("wrong number of labels. #labels " + labels.size() + " #columns " + nCols_);
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
    static DataTable<T>  DataTable<T>::sum(const DataTable<T>& lhs, const DataTable<T>& rhs) {

        if (lhs.getNColumns() != rhs.getNColumns() || lhs.getNColumns() != rhs.getNColumns())
            throw std::invalid_argument("lhs and rhs differ in size");  //check just on data size.. ignore labels and time column

        DataTable<T> ans(lhs);
        size_t nRows(ans.getNRows());
        for (size_t row(0); row < nRows; ++row)
            std::transform(lhs.data_.at(i).begin(), lhs.data_.at(i).end(), rhs.data_.at(i).begin(), ans.data_.at(i).begin(), [](l, r) {
            return l + r;
        });
        return ans;
    }

    template <typename T>
    static DataTable<T>  DataTable<T>::subtract(const DataTable<T>& lhs, const DataTable<T>& rhs) {

        if (lhs.getNColumns() != rhs.getNColumns() || lhs.getNColumns() != rhs.getNColumns())
            throw std::invalid_argument("lhs and rhs differ in size");  //check just on data size.. ignore labels and time column

        DataTable<T> ans(lhs);
        size_t nRows(ans.getNRows());
        for (size_t row(0); row < nRows; ++row)
            std::transform(lhs.data_.at(i).begin(), lhs.data_.at(i).end(), rhs.data_.at(i).begin(), ans.data_.at(i).begin(), [](l, r) {
            return l - r;
        });
        return ans;
    }

}
#endif
