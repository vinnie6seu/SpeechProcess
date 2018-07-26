#ifndef _THROW_H
#define _THROW_H

#include <tr1/memory>
#include <string>
#include <sstream>
#include <vector>

#include <stdarg.h>

#include "Util/Exception/exceptions.hpp" 

using namespace std;

/// @brief ���׳��쳣����
class ThrowableException {
protected:
    vector< tr1::shared_ptr<Exception> > exceptions_;

public:
    /// @brief Ĭ�Ϲ��캯��
    ThrowableException();

    /// @brief �������캯��
    /// @param other ��Ҫ�����Ķ���
    ThrowableException(const ThrowableException* other);

    /// @brief �������캯��
    /// @param other ��Ҫ�����Ķ���
    ThrowableException(ThrowableException& other);

    /// @brief ���캯��
    /// @param in �����˵��쳣�б�
    ThrowableException(vector< tr1::shared_ptr<Exception> >& in);

    /// @brief �������캯��
    /// @param in ��Ҫ���п����Ķ���
    ThrowableException(tr1::shared_ptr<Exception> in);

    /// @brief �������졣���������չ
    virtual ~ThrowableException() {}

    /// @brief ��ȡ���ݿ��쳣�ľ�������
    /// @param needDetail �Ƿ���Ҫ��ӡ��ϸ��
    /// @return �������ݿ��쳣�ľ�������
    virtual std::string what() const;

    /// @brief ��ȡ�ڲ��쳣�б�
    /// @return �����ڲ��쳣�б�
    virtual vector< tr1::shared_ptr<Exception> >& GetInnerExceptions();
    
    /// @brief ����ڲ��쳣�б��Ƿ�Ϊ��
    /// @return ����boolֵ   
    bool isEmpty() const;
    
    void addExceptionVec(vector< tr1::shared_ptr<Exception> >& exceptions);
    
    void addException(tr1::shared_ptr<Exception> in);

private:
    /// @brief �����ڲ��쳣�б�
    /// @param exps ��Ҫ���п������쳣�б�
    void CopyException(const vector<tr1::shared_ptr<Exception> >& exps);
};  

// 1.�쳣�����ɱ���������,��Ϊ�׳��쳣���ʽ�ĸ���,��Ҫ�Զ��忽�����캯��
// 2.�׳���Ӧ���Ƕ���,��Ҫ��ָ��ֲ������ָ��.
// 3.�����һ��ָ�����������Ļ���ָ��,��ô�׳���ָ��Ľ�����,ֻ���׳����ಿ��.
// 4.����Ҳ���ƥ���catch,��������terminate,������abort����.
// 5.�������������׳��쳣,��Ϊĳ���쳣����ջչ��������,�����������׳�δ���������һ���쳣,��������terminate.
// 6.ʹ��throw���������׳�,�׳����Ǳ�����������쳣���󲢲���catch���β�.����catch��ͨ������,�ı������������쳣����.
// 7.Ϊ�˴������Թ��캯����ʼ��ʽ���쳣,�����캯��д�ɺ������Կ�.    


// ��ʼ���쳣��Err�����׳��쳣ThrowableException
// #define RAISE_EXCEPTION(Err, msg) {std::ostringstream oss; oss << msg; tr1::shared_ptr<Exception> inner_e(new Err(oss.str(), E_INFOS)); ThrowableException e(inner_e); throw e;}    


// throw e��ȥ��ʱ��͵�����һ�ο������캯��,�������������õĲ��������ڲ���쳣����
#define RAISE_EXCEPTION(Err, preThrowableException, msg) { \
	std::ostringstream oss; \
	oss << msg; \
	tr1::shared_ptr<Exception> inner_e(new Err(oss.str(), E_INFOS)); \
	preThrowableException.addException(inner_e); \
	throw e; \
}

#endif


                
				
				







