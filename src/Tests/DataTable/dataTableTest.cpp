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

    return 0;
}