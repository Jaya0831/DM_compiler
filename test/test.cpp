using namespace std;
#include <list>
#include <iostream>
#include "test.h"

void myFunc(int num){
    list<int> myList;
    for (int i = 0; i < num; i++)
    {
        myList.push_back(i);
    }
    for (int i : myList)
    {
        cout<<i<<" ";
    }
}
