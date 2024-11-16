#include <iostream>
/*
1. 单例设计方式，设计一个单例类，确保该类只有一个实例对象
*/
class sigleton
{
public:
    static sigleton& getInstance()
    {
        static sigleton instance;
        return instance;
    }
    void showMessage()
    {
        std::cout<< "This is a instance" << std::endl;
    }
private:
    //禁止默认构造函数和析构函数；
    sigleton() = default;
    ~sigleton() = default;
    sigleton(const sigleton&) = delete;
    sigleton& operator=(const sigleton&) = delete;
};

int main()
{
    sigleton::getInstance().showMessage();
    system("pause");
    return 0;
   
}
