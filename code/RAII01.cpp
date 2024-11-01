#include <iostream>
bool operationA()
{
    //do something
    return false;
}


bool operationB()
{
    //do something
    return true;
}

int main()
{
    int *testArray = new int[10];
    //Here , you can use the array
    if(!operationA())
    {
        //if the operator A failed, we should delete the memory
        delete[] testArray;
        testArray = nullptr;
    }

    //Here , you can use the array
    if(!operationB())
    {
        //if the operator B failed, we should delete the memory
        delete[] testArray;
        testArray = nullptr;
    }

    delete [] testArray;
    testArray = nullptr;
    std::cout << "delete" <<std::endl;
    system("pause");
    return 0;
}