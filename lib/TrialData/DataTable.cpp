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
 * Author(s):                                                                 *
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
 
#include <algorithm>
#include <stdexcept>

namespace ceinms {

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
    const std::vector<T>& DataTable<T>::getRow(size_t row) const {

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

        DataTable<T> ans(lhs.getNRows() < rhs.getNRows() ? lhs.getNRows() : rhs.getNRows(), lhs.getNColumns());
        size_t nRows(std::min(lhs.getNRows(), rhs.getNRows()));
        for (size_t row(0); row < nRows; ++row)
            std::transform(lhs.data_.at(row).begin(), lhs.data_.at(row).end(), rhs.data_.at(row).begin(), ans.data_.at(row).begin(), [](T l, T r) {
            return l + r;
        });
        ans.labels_ = lhs.labels_;
        ans.time_.assign(lhs.time_.begin(), lhs.time_.begin() + ans.nRows_);
        return ans;
    }

    template <typename T>
    DataTable<T> DataTable<T>::subtract(const DataTable<T>& lhs, const DataTable<T>& rhs) {

        if (lhs.getNColumns() != rhs.getNColumns() || lhs.getNColumns() != rhs.getNColumns())
            throw std::invalid_argument("lhs and rhs differ in size");  //check just on data size.. ignore labels and time column

        //initialise with nRows equals to the minmum of the two arguments
        DataTable<T> ans(lhs.getNRows() < rhs.getNRows() ? lhs.getNRows() : rhs.getNRows(), lhs.getNColumns());
        size_t nRows(ans.getNRows());
        for (size_t row(0); row < nRows; ++row)
            std::transform(lhs.data_.at(row).begin(), lhs.data_.at(row).end(), rhs.data_.at(row).begin(), ans.data_.at(row).begin(), [](T l, T r) {
            return l - r;
        });
        ans.labels_ = lhs.labels_;
        ans.time_.assign(lhs.time_.begin(), lhs.time_.begin() + ans.nRows_);
        return ans;
    }


    template <typename T>
    DataTable<T>  DataTable<T>::multiplyByElement(const DataTable<T>& lhs, const DataTable<T>& rhs) {

        if (lhs.getNColumns() != rhs.getNColumns() || lhs.getNColumns() != rhs.getNColumns())
            throw std::invalid_argument("lhs and rhs differ in size");  //check just on data size.. ignore labels and time column

        DataTable<T> ans(lhs.getNRows() < rhs.getNRows() ? lhs.getNRows() : rhs.getNRows(), lhs.getNColumns());
        for (size_t r(0); r < lhs.nRows_; ++r)
            for (size_t c(0); c < lhs.nCols_; ++c)
                ans.at(r, c) = lhs.at(r, c)*rhs.at(r, c);
        ans.labels_ = lhs.labels_;
        ans.time_.assign(lhs.time_.begin(), lhs.time_.begin() + ans.nRows_);
        return ans;
    }


    template <typename T>
    DataTable<T>  DataTable<T>::multiplyByScalar(const DataTable<T>& lhs, T scalar) {

        DataTable<T> ans(lhs.getNRows());
        for (size_t r(0); r < lhs.nRows_; ++r)
            for (size_t c(0); c < lhs.nCols_; ++c)
                ans.at(r, c) = lhs.at(r, c)*scalar;
        ans.labels_ = lhs.labels_;
        ans.time_.assign(lhs.time_.begin(), lhs.time_.begin() + ans.nRows_);
        return ans;
    }

    template<typename T>
    bool DataTable<T>::equals(const DataTable<T>& rhs) const {
        return(data_ == rhs.data_   &&
            labels_ == rhs.labels_ &&
            time_ == rhs.time_   &&
            nCols_ == rhs.nCols_  &&
            nRows_ == rhs.nRows_);


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

    template<typename T>
    DataTable<T> operator* (const DataTable<T>& lhs, const DataTable<T>& rhs) {
        return DataTable<T>::multiplyByElement(lhs, rhs);
    }

    template<typename T>
    DataTable<T> operator* (const DataTable<T>& lhs, T scalar) {
        return DataTable<T>::multiplyByScalar(lhs, scalar);
    }

    template<typename T>
    DataTable<T> operator* (T scalar, const DataTable<T>& rhs) {
        return DataTable<T>::multiplyByScalar(rhs, scalar);
    }

}
