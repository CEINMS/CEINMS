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

#include <iostream>
using std::cout;
using std::endl;

#include "ceinms/DataTable.h"
using ceinms::DataTable;

int main() {
    // Test the default data table initializer
    cout << "1 - Default initializer" << endl
         << "    (should create a data table with zero columns and rows)" << endl;
    DataTable<double> t1;
    cout << t1 << endl;

    // Test setting the column labels
    cout << "2 - Set labels" << endl
         << "    (the columns should be labeled: 'Time   l1  l2  l3')" << endl;
    t1.setLabels({ "l1", "l2", "l3"});
    cout << t1 << endl;

    // Test pushing data into rows
    cout << "3 - Pushed row" << endl
         << "    (the table should now contain 3 rows of data)" << endl;
    t1.pushRow(1, { 1, 2, 3 });
    t1.pushRow(2, { 1, 2, 4 });
    t1.pushRow(3, { 4, 2, 4 });
    cout << t1;
    cout << "Element (1,2): " << t1.at(1, 2) << endl;
    cout << "Element (1,2)+1: " << ++t1.at(1, 2) << endl << endl;

    // Test subtracting 2 data tables containing the same row values
    cout << "4 - Subtraction" << endl
         << "    (data table should be 3 rows of zeros (both tables contain the same values))" << endl;
    auto t2(t1);
    cout << t2-t1 << endl;

    // Test adding 2 data tables containing the same row values
    cout << "5 - Sum" << endl
         << "    (the elements of the two tables should be summed (both tables contain the same values))" << endl;
    cout << "Sum" << endl;
    cout << t1+t2 << endl;

    // Test initializing a new table to provided dimensions
    cout << "6 - Dimension initializer" << endl
         << "    (a new table should be created with 3 row and 4 columns of zeros)" << endl;
    DataTable<double> table(3, 4);
    cout << table << endl;

    // Test adding 2 data tables containing different row values
    cout << "7 - Sum with different row values" << endl;
    t2.pushRow(4, { 7, 7, 7 });
    t2.setRow(1, { 3, 3, 3 });
    cout << "Table 1:" << endl;
    cout << t1 << endl;

    cout << "Table 2:" << endl;
    cout << t2 << endl;

    cout << "7a - Table 1 minus Table 2" << endl;
    cout << t1 - t2 << endl;

    cout << "7b - Table 2 minus Table 1" << endl;
    cout << t2 - t1 << endl;

	// Test cropping data table
	cout << "8 - Testing table cropping" << endl
         << "    (the table should be cropped to display from between Time = 10.01 and 13.07)" << endl;
	DataTable<double> t3;
	for (int i(0); i < 1000; ++i) {
		t3.pushRow(i*0.1, { i * 10., i * 20. });
	}
	t3.crop(10.01, 13.07);
	cout << t3;

    return 0;
}
