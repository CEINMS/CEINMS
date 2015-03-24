#ifndef Ceinms_DataTable_h
#define Ceinms_DataTable_h
#include <vector>
#include <string>

#include <ostream>

//TODO: insert further checks, operators + and -
namespace CEINMS {
    template<typename T> class DataTable;

    template<typename T>
    std::ostream& operator<< (std::ostream & out, const DataTable<T> & rhs);

    template<typename T>
    bool operator==(const DataTable<T>& lhs, const DataTable<T>& rhs) {
        return lhs.equals(rhs);
    }

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
        
        static DataTable sum(const DataTable& lhs, const DataTable& rhs);
        static DataTable subtract(const DataTable& lhs, const DataTable& rhs);

        std::string getLabel(size_t col) const { return labels_.at(col); }
        T get(size_t row, size_t col) const { return data_.at(row, col); }
        T getTime(size_t row) const { return time_.at(row); }
        T getStartTime() const { return time_.front(); }
        T getFinalTIme() const { return time_.back(); }
        std::vector<T> getTimeColumn(size_t col) const { return time_; }
        std::vector<T> getRow(size_t row) const;
        std::vector<T> getColumn(size_t col) const;
        size_t getNColumns() const { return nCols_; }
        size_t getNRows() const { return nRows_; }

        bool equals(const DataTable& rhs) const;
        friend std::ostream& operator<< <> (std::ostream& output, const DataTable& ths);
    private:
        size_t nRows_, nCols_;
        //internal implementation may be changed to something better.. e.g. nice matrices
        std::vector<std::vector<T>> data_;
        std::vector<T> time_;
        std::vector<std::string> labels_;


    };

}
#include "DataTable.cpp"
#endif
