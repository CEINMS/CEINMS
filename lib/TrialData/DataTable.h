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
 * Author(s): C. Pizzolato                                                    *
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

#ifndef ceinms_DataTable_h
#define ceinms_DataTable_h
#include <vector>
#include <string>

#include <ostream>

//TODO: insert further checks, operators + and -
namespace ceinms {
    template<typename T> class DataTable;

    template<typename T>
    std::ostream& operator<< (std::ostream & out, const DataTable<T> & rhs);

    template<typename T>
    class DataTable {

    public:
        DataTable();
        DataTable(size_t nRows, size_t nCols); //do not include time column
        void setLabels(const std::vector<std::string>& labels); //do not include time column
        std::vector<std::string> getLabels() { return labels_; }
        void set(T value, size_t row, size_t col);
        const T& at(size_t row, size_t col) const;
        T& at(size_t row, size_t col);
        void pushRow(T time, const std::vector<T>& values);
        void setRow(size_t row, const std::vector<T>& values);

        static DataTable sum(const DataTable& lhs, const DataTable& rhs);
        static DataTable subtract(const DataTable& lhs, const DataTable& rhs);
        static DataTable multiplyByElement(const DataTable& lhs, const DataTable& rhs);
        static DataTable multiplyByScalar(const DataTable& lhs, T scalar);

        std::string getLabel(size_t col) const { return labels_.at(col); }
        T get(size_t row, size_t col) const { return data_.at(row, col); }
        T getTime(size_t row) const { return time_.at(row); }
        T getStartTime() const { return time_.front(); }
        T getFinalTIme() const { return time_.back(); }
        std::vector<T> getTimeColumn(size_t col) const { return time_; }
        const std::vector<T>& getRow(size_t row) const; //return const ref is bad.. checking for performance

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
