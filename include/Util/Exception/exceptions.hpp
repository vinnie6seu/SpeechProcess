#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include <string>
#include <sstream>
#include <exception>

// 在抛出异常时加上该宏，指出异常出处 
#define E_INFOS __FUNCTION__,__FILE__,__LINE__

// 从异常类Super中派生出异常类Current
// 在宏定义中，#表示：对应变量字符串化；##表示：把宏参数名与宏定义代码序列中的标识符连接在一起，形成一个新的标识符。
#define ADD_EXCEPTION_CLASS(Super, Current) class Current : public Super {public: Current ( const std::string & desc, const std::string & func="?", const std::string & f="?", const int l=-1 ) : Super (desc,func,f,l) {name = #Current;} }

//! This is the base class for all exceptions in oMetah
class Exception : public std::exception {
protected:
    // 异常类名字
    std::string name;

    // 异常信息
    std::string description;

    //! Function where the exception has been raised
    std::string function;

    //! File where the exception has been raised
    std::string file;

    //! Line where the exception has been raised
    int line;

public:
    //! Constructor of the exception
    /*!
        This constructor is not supposed to be used with hand-made location arguments
        but with metadata provided by the compiler.

        Use the E_INFOS macro to raise the exception, for example :
            throw( Exception( "Shit evolves", E_INFOS );
    */
    Exception( const std::string & desc, const std::string & func, const std::string & f, const int l ) 
        : description(desc), function(func), file(f), line(l) {}

    //! The destructor is not allowed to throw exceptions
    virtual ~Exception() throw () {}

    //! The method to use for printing the complete description of the exception
    // 写成虚函数，方便用基类接受所有派生类的异常信息
    virtual std::string what() throw () {
        std::ostringstream msg;

        // msg << description << " (<" << name << "> in " << function << " at " << file << ":" << line << ")";
        msg << description << " (ExceptionName=<" << name << "> in FunctionName=<" << function << "> at " << file << ":" << line << ")";

        return msg.str();
    }
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 此处增加各个新的异常类
ADD_EXCEPTION_CLASS(Exception, RUN_EXCEPTION);  

ADD_EXCEPTION_CLASS(Exception, SHM_EXCEPTION);

ADD_EXCEPTION_CLASS(Exception, TEXT_TO_SPEECH_EXCEPTION);


#endif 

