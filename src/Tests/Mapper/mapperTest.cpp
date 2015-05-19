#include "ceinms/Mapper.h"
using ceinms::Mapper;
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;


int main() {

    //Map from a bigger group to a smaller group
    vector<string> namesIn = { "Apple", "Melon", "Orange", "Banana", "PineApple" };
    vector<string> namesOut = { "Melon", "Banana", "Orange", };

    Mapper bigToSmall(namesIn, namesOut);
    auto ans1 = bigToSmall.map<string>(namesIn);
    for (auto s : ans1) cout << s << " ";
    cout << endl;

    Mapper smallToBig(namesOut, namesIn);
    auto ans2 = smallToBig.map<string>(namesOut);
    for (auto s : ans2) cout << s << " ";
    cout << endl;


}