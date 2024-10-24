#include <iostream>

template <typename T>
class MySharedPtr {
private:
    T* ptr; // 指向管理对象的指针
    int* count; // 指向引用计数的指针

public:
    // 构造函数
    MySharedPtr(T* p = nullptr) : ptr(p), count(new int(1)) {
        if (ptr) {
            std::cout << "MySharedPtr constructed with ptr: " << ptr << std::endl;
        }
    }

    // 拷贝构造函数
    MySharedPtr(const MySharedPtr<T>& other) : ptr(other.ptr), count(other.count) {
        if (ptr) {
            (*count)++;
            std::cout << "MySharedPtr copied. New count: " << *count << std::endl;
        }
    }

    // 赋值操作符
    MySharedPtr<T>& operator=(const MySharedPtr<T>& other) {
        if (this != &other) {
            // 减少原有对象的引用计数，并在需要时释放资源
            if (ptr) {
                if (--(*count) == 0) {
                    delete ptr;
                    delete count;
                    std::cout << "MySharedPtr object destroyed." << std::endl;
                }
            }
            // 接管新对象的资源和引用计数
            ptr = other.ptr;
            count = other.count;
            if (ptr) {
                (*count)++;
                std::cout << "MySharedPtr assigned. New count: " << *count << std::endl;
            }
        }
        return *this;
    }

    // 析构函数
    ~MySharedPtr() {
        if (ptr) {
            if (--(*count) == 0) {
                delete ptr;
                delete count;
                std::cout << "MySharedPtr destroyed." << std::endl;
            }
        }
    }

    // 重载*和->操作符
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }

    // 获取原始指针（危险，应谨慎使用）
    T* get() const { return ptr; }

    // 获取引用计数（主要用于调试）
    int use_count() const { return *count; }
};

// 示例类
class A {
public:
    A() { std::cout << "A constructed" << std::endl; }
    ~A() { std::cout << "A destructed" << std::endl; }
};

int main()
{
    return 0;
}