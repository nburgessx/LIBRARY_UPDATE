#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#ifdef DUMP_WIN

#ifdef _MSC_VER // when VC, with CEdit of MFC
#include <afxwin.h>
#define  F_LOGOBJ       CEdit  // GUI object printed error message
#else
// not VC++
#define  F_LOGOBJ       void   // GUI object printed error message
#endif

//set void when DUMP_WIN is not selected
#else
#define  F_LOGOBJ       void   // GUI object printed error message
#endif

/*! 
    @brief Class to print error message.
*/
class LACoreErrorLog
{
public:
// LIFECYCLE
    // constructor
    LACoreErrorLog();
    // destructor
    ~LACoreErrorLog();

// OPERATION
    // add an error mesage to the object of the screen
    static void         setMsg(const char* msg);
    // select object printing error message
    static void         setLog(F_LOGOBJ* editBox);

private:
    static F_LOGOBJ* mEditBox;  // object printing error message

};
