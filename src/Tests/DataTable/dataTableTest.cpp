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
 
#include "DataTable.h"
#include <iostream>
using std::cout;
using std::endl;

using ceinms::DataTable;

int main() {

    cout << "Default initializer" << endl;
    DataTable<double> t1;
    std::cout << t1;
    t1.setLabels({ "l1", "l2", "l3"});
    cout << "Set labels" << endl;
    cout << t1;
    t1.pushRow(1, { 1, 2, 3 });
    t1.pushRow(2, { 1, 2, 4 });
    cout << "Pushed row" << endl;
    cout << t1;
    cout << "Element (1,2): " << t1.at(1, 2) << endl;
    cout << "Element (1,2)+1: " << ++t1.at(1, 2) << endl;
    t1.pushRow(3, { 4, 2, 4 });

    auto t2(t1);
    cout << "Subtraction" << endl;
    cout << t2-t1;

    cout << "Sum" << endl;
    cout << t1+t2;

    cout << "Dimension initializer" << endl;
    DataTable<double> table(3, 4);
    std::cout << table;

    cout << endl << "-------------" << endl;
    cout << "Sum with different row numbers" << endl;
    t2.pushRow(4, { 7, 7, 7 });
    t2.setRow(1, { 3, 3, 3 });
    cout << "Table 1:" << endl;
    cout << t1 << endl;

    cout << "Table 2:" << endl;
    cout << t2 << endl;

    cout << "Table 1 minus Table 2:" << endl;
    cout << t1 - t2;

    cout << "Table 2 minus Table 1:" << endl;
    cout << t2 - t1;
    return 0;
}
