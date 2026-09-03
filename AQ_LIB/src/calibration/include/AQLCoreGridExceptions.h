#pragma once

#include <stdexcept>

#if defined (_MSC_VER) && (_MSC_VER >= 1000)
# pragma once
# pragma warning(disable : 4290)
#endif

class IRGException
{
    public:

		IRGException() throw();
        IRGException(const char *file, int line, const char *fmt, ...) throw();
        virtual ~IRGException() throw();
        virtual const char* what() const throw();
       
    protected:
        int  m_Line;
        char m_File[512];
        char m_Description[2048];
};


class IRGTimeoutException : public IRGException {
    public:
        IRGTimeoutException() throw ();
		IRGTimeoutException(const char *file, int line, const char *fmt, ...) throw();
		virtual ~IRGTimeoutException() throw ();
};
