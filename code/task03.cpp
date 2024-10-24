// 练习:
// 写一个简单的KString类。要求实现main函数中的功能。
#include <iostream>
#include <memory>
#include <cstring>

class KString{
public:
   KString():m_data(nullptr), m_size(0){}; //变量都初始化为0
   
   //堆区申请len个空间,把str字符串内容给m_data,并设置长度
   KString(const char *str,int len)
   {
        m_size = len;
        m_data = new char[len + 1];
        std::memcpy(m_data, str, len);
        m_data[len] = '\0';
   }
   KString(KString&& str){
        m_size = str.m_size;
        m_data = str.m_data;
        str.m_data = nullptr;
        str.m_size = 0;
   }
   char* getString()
   {
    return m_data;
   }
   ~KString()
   {
        if (m_data)
        {
            delete[] m_data;
        }
        
   }
private:                                               
   char *m_data;                                       
   int m_size;                                              
                                                            
};

 int main()
{  
    KString str1("hello", 6);
    KString str2 = std::move(str1);
    std::cout << str2.getString()<< std:: endl;
}    