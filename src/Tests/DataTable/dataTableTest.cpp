#include "DataTable.h"
#include <iostream>
using std::cout;
using std::endl;

using CEINMS::DataTable;

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