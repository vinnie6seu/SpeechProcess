#ifndef _THROW_H
#define _THROW_H

#include <tr1/memory>
#include <string>
#include <sstream>
#include <vector>

#include <stdarg.h>

#include "Util/Exception/exceptions.hpp" 

using namespace std;

/// @brief 可抛出异常类型
class ThrowableException {
protected:
    vector< tr1::shared_ptr<Exception> > exceptions_;

public:
    /// @brief 默认构造函数
    ThrowableException();

    /// @brief 拷贝构造函数
    /// @param other 需要拷贝的对象
    ThrowableException(const ThrowableException* other);

    /// @brief 拷贝构造函数
    /// @param other 需要拷贝的对象
    ThrowableException(ThrowableException& other);

    /// @brief 构造函数
    /// @param in 发生了的异常列表
    ThrowableException(vector< tr1::shared_ptr<Exception> >& in);

    /// @brief 拷贝构造函数
    /// @param in 需要进行拷贝的对象
    ThrowableException(tr1::shared_ptr<Exception> in);

    /// @brief 析构构造。方便后续拓展
    virtual ~ThrowableException() {}

    /// @brief 获取数据库异常的具体描述
    /// @param needDetail 是否需要打印出细节
    /// @return 返回数据库异常的具体描述
    virtual std::string what() const;

    /// @brief 获取内部异常列表
    /// @return 返回内部异常列表
    virtual vector< tr1::shared_ptr<Exception> >& GetInnerExceptions();
    
    /// @brief 检查内部异常列表是否为空
    /// @return 返回bool值   
    bool isEmpty() const;
    
    void addExceptionVec(vector< tr1::shared_ptr<Exception> >& exceptions);
    
    void addException(tr1::shared_ptr<Exception> in);

private:
    /// @brief 拷贝内部异常列表
    /// @param exps 需要进行拷贝的异常列表
    void CopyException(const vector<tr1::shared_ptr<Exception> >& exps);
};  

// 1.异常对象由编译器管理,是为抛出异常表达式的副本,需要自定义拷贝构造函数
// 2.抛出的应该是对象,不要是指向局部对象的指针.
// 3.如果有一个指向派生类对象的基类指针,那么抛出该指针的解引用,只会抛出基类部分.
// 4.如果找不到匹配的catch,程序会调用terminate,其中有abort函数.
// 5.析构函数不能抛出异常,在为某个异常进行栈展开过程中,析构函数又抛出未经处理的另一个异常,程序会调用terminate.
// 6.使用throw进行重新抛出,抛出的是编译器管理的异常对象并不是catch的形参.可在catch中通过引用,改变编译器管理的异常对象.
// 7.为了处理来自构造函数初始化式的异常,将构造函数写成函数测试块.    


// 初始化异常类Err，并抛出异常ThrowableException
// #define RAISE_EXCEPTION(Err, msg) {std::ostringstream oss; oss << msg; tr1::shared_ptr<Exception> inner_e(new Err(oss.str(), E_INFOS)); ThrowableException e(inner_e); throw e;}    


// throw e出去的时候就调用了一次拷贝构造函数,因此在最外层引用的并不是最内层的异常对象
#define RAISE_EXCEPTION(Err, preThrowableException, msg) { \
	std::ostringstream oss; \
	oss << msg; \
	tr1::shared_ptr<Exception> inner_e(new Err(oss.str(), E_INFOS)); \
	preThrowableException.addException(inner_e); \
	throw e; \
}

#endif


                
				
				







