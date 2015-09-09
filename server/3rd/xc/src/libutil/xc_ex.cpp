#include "util/xc_ex.h"
#include <execinfo.h>
#include <string.h>
#include <stdlib.h>
#include <cerrno>

namespace xutil
{

XC_Exception::XC_Exception(const string &buffer)
:_buffer(buffer), _code(0)
{
//    getBacktrace();
}

XC_Exception::XC_Exception(const string &buffer, int err)
{
	_buffer = buffer + " :" + strerror(err);
    _code   = err;
//    getBacktrace();
}

XC_Exception::~XC_Exception() throw()
{
}

const char* XC_Exception::what() const throw()
{
    return _buffer.c_str();
}

void XC_Exception::getBacktrace()
{
    void * array[64];
    int nSize = backtrace(array, 64);
    char ** symbols = backtrace_symbols(array, nSize);

    for (int i = 0; i < nSize; i++)
    {
        _buffer += symbols[i];
        _buffer += "\n";
    }
	free(symbols);
}

}
