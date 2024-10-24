#include <iostream>
#include <string>
#include <memory>

using std::shared_ptr;
using std::make_shared;

class TryTestClassA
{

};

class TryTestClassB {
public:
    TryTestClassB()
    {
        std::cout << "TryTestClass(string msg)" << std::endl;
    }

    ~TryTestClassB()
    {
        std::cout << "~TryTestClass()" << std::endl;
    }

    //设置对象
    void setObj(std::shared_ptr<TryTestClassA> &obj)
    {
        m_aPtr = obj;
    }
    int getObjCount()
    {
        return m_aPtr.use_count();
    }
private:
    std::shared_ptr<TryTestClassA> m_aPtr;
};

 //场景2：使用智能指针来解决,思考下列user_cout使出的值是什么？l
void tryUseSmartPoint()
{
std::shared_ptr<TryTestClassA> ptrA = std::make_shared<TryTestClassA>();
    std::shared_ptr<TryTestClassB> ptrB = std::make_shared<TryTestClassB>();
    ptrB->setObj(ptrA);

    std::cout << "ptrA count : " << ptrA.use_count() << std::endl;
    std::cout << "ptrB count : " << ptrB.use_count() << std::endl;

    ptrA.reset(); //ptrA释放对象所有权

    std::cout << "ptrA count : " << ptrA.use_count() << std::endl;
    std::cout << "ptrB count : " << ptrB.use_count() << std::endl;
    //ptrB中的m_aPtr依旧存在所有权,shared_ptr内部对->进行了重载，本质是返回ptrB.get()->
    //getCount()
    std::cout << "ptrB count : " << ptrB->getObjCount() << std::endl; 
}
int main(int argc, char const *argv[])
{
    tryUseSmartPoint();
    return 0;
}
