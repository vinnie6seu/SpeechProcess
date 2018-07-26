#include "Util/Exception/throw.hpp"
#include "Util/Exception/exceptions.hpp" 

#include <iostream>

ThrowableException::ThrowableException() {
	exceptions_.clear();
}

ThrowableException::ThrowableException(const ThrowableException* other) {
	CopyException(other->exceptions_); 
}

ThrowableException::ThrowableException(ThrowableException& other) {
	CopyException(other.exceptions_);
}

ThrowableException::ThrowableException(vector<tr1::shared_ptr<Exception> >& in) {
    CopyException(in);
}

ThrowableException::ThrowableException(tr1::shared_ptr<Exception> in) {
    exceptions_.push_back(in);
}

std::string ThrowableException::what() const {
    stringstream ss;

    ss << "�����쳣,������Ϣ����(��" << exceptions_.size() << "���쳣��" << endl;
    for (int i = 0; i < exceptions_.size(); i++) {
        ss << "�쳣[" << i + 1 << "]���ݣ�" << exceptions_[i]->what() << endl;   
    }

    return ss.str();
}

vector<tr1::shared_ptr<Exception> >& ThrowableException::GetInnerExceptions() {
    return exceptions_;
}

bool ThrowableException::isEmpty() const {
	return exceptions_.size() == 0;
}

void ThrowableException::addException(tr1::shared_ptr<Exception> in) {
	exceptions_.push_back(in);
}

void ThrowableException::CopyException(const vector<tr1::shared_ptr<Exception> >& exps) {
    exceptions_.clear();

    for (int i = 0; i < exps.size(); i++) {
        exceptions_.push_back(exps[i]);
    }
}

void ThrowableException::addExceptionVec(vector< tr1::shared_ptr<Exception> >& exceptions) {
    for (int i = 0; i < exceptions.size(); i++) {
        exceptions_.push_back(exceptions[i]);
    }	
}
