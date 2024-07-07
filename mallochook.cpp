#include <iostream>
#include <string>
using namespace std;
//参考：https://www.bilibili.com/video/BV1if421B7Jo/?spm_id_from=333.880.my_history.page.click&vd_source=7a6c8709cb6e18891827cd3ee51f69ee

#include <unordered_map> //哈希结构 - 每插入一个元素将调用两次operator new
#include <map>

#include <execinfo.h> //backtrace
#include <cxxabi.h> //解析符号


struct Meminfo
{
    void *p_;
    size_t size_;
    std::string caller_;
};

struct Global
{
    map<void*, Meminfo> allocated;

    bool enable; //允许调用operator new
    Global()
    {
        enable = true;
    }
    ~Global()
    {
        enable = false;
        for(auto &[p, info]:allocated)
        {
            printf("内 存 泄 漏 了 : p=%p size=%zd caller=%s\n", info.p_, info.size_, info.caller_.c_str());
        }
    }
} g; //全局变量，在main函数之前构造，main函数之后析构

struct EnableGuard
{
    bool was_enable;

    EnableGuard()
    {
        was_enable = g.enable;
        g.enable = false;
    }

    explicit operator bool()
    {
        return was_enable;
    }

    ~EnableGuard()
    {
        g.enable = was_enable;
    }
};

std::string backtrace_symbols(void *const *__array)
{
    char **strings = ::backtrace_symbols(__array, 1); //backtraace 头文件包含#include <execinfo.h> 需要在编译选项中加入-rdynamic以支持显示函数名功能
    std::string res = strings[0];
    free(strings);
    
    //解析出所在函数名
    auto pos = res.find('(');
    auto pos2 = res.find('+');
    if(pos != std::string::npos && pos2 != std::string::npos)
    {
        res = res.substr(pos+1,pos2-pos-1);
        char *demangled = abi::__cxa_demangle(res.data(), nullptr,nullptr,nullptr); //解析符号 头文件包含#include <cxxabi.h> 
        if(demangled)
        {
            res = demangled; 
            free(demangled);
        }
    }
    return res;
}

void *operator new(size_t size)
{
    EnableGuard guard; //使用RAII思想

    void *p = malloc(size);
    if(p == nullptr) throw std::bad_alloc();

    if(guard) //强制转换EnableGuard为bool类型
    {    
        void *pcaller = __builtin_return_address(0); //该函数得到当前函数返回地址，该函数由编译器提供，参数默认为0
        g.allocated[p] = Meminfo{p,size,backtrace_symbols(&pcaller)}; //往里面插入东西时本身会调用operator new，因此需要防止递归调用
        printf("operator new p=%p, size=%zd caller=%s\n", g.allocated[p].p_, g.allocated[p].size_, g.allocated[p].caller_.c_str());
    }

    return p;   
}

void operator delete(void *p)
{
    EnableGuard guard;
    
    if(p != nullptr)
    {
        if(g.allocated.count(p))
        {        
            void *pcaller = __builtin_return_address(0); //该函数得到当前函数返回地址，该函数由编译器提供，参数默认为0
            printf("operator delete p=%p caller=%s\n", g.allocated[p].p_, backtrace_symbols(&pcaller).c_str());
            g.allocated.erase(p); 
        }
    }

    free(p);
}

char* test(int x = 4, double w = 2.0)
{
    char *p = new char; 
    char *p1 = new char;

    return p;
}


int main(int argc, char const *argv[])
{
    char * p = test();
    delete p;

    return 0;
}