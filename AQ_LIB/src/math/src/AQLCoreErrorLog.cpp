/*! @file
    @brief Implementation to print error message.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLCoreErrorLog.h"
#include <cstdio>

#define  LINES  5   // the number of rows to display an error message to the screen (show by scrolling if more than this number of lines) 

// initialize STATIC VARIABLE
F_LOGOBJ*
AQLCoreErrorLog::mEditBox = NULL;

///////////////////////// IMPLEMENTATION //////////////////////////
// LIFECYCLE
/*!
    @brief constructor
*/
AQLCoreErrorLog::AQLCoreErrorLog()
{
}

/*!
    @brief destructor
*/
AQLCoreErrorLog::~AQLCoreErrorLog()
{
}

/*!
    @brief add an error mesage to the object of the screen
    
    @param[in] msg additional error message
*/
void
AQLCoreErrorLog::setMsg(const char* msg)
{
    (void)msg;
	if (mEditBox == NULL) return;

#ifdef DUMP_WIN
// perform DUMP
#ifdef _MSC_VER
    // by Windows
    CString str;
    try {
        mEditBox->GetWindowText(str);
        str += msg;
        str += "\r\n";
        mEditBox->SetWindowText(str);
        mEditBox->LineScroll(mEditBox->GetLineCount()> LINES ?
            mEditBox->GetLineCount() - LINES : 0);
    } catch(...)
    {
        ;
    }
#else
// not Windows
    return;
#endif

#else
// not perform DUMP
    return;
#endif
}

/*!
    @brief select object printing error message
    
    @param[in] editbox object printing error message
*/
void
AQLCoreErrorLog::setLog( F_LOGOBJ* editbox)
{
    mEditBox = editbox;
}
    
