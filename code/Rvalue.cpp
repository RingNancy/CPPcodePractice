#include <iostream>
#include <string>

class MyObject {
public:
    MyObject(const std::string& name) : name(name) {
        std::cout << "Constructing MyObject: " << name << std::endl;
    }

    ~MyObject() {
        std::cout << "Destructing MyObject: " << name << std::endl;
    }

private:
    std::string name;
};
//正常是创建临时变量接受，然后返回临时变量

//temp 空间销毁
MyObject createObject(const std::string& name) {
    return MyObject(name); //将亡值
}



int main() {
    // 将亡值绑定到右值引用
    MyObject&& obj = createObject("temp"); //函数调用后，值将死亡，移动到MyObject.
    //Myobject obj = createObject("temp");
    // 使用 obj
    std::cout << "Using MyObject: " << std::endl;

    return 0;
}
