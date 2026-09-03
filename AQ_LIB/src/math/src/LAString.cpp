#include "LAString.h"

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#endif

#include "LABasic.h"
#include "ExceptionMacros.h"    // incl AQ_REQUIRE, AQ_THROW, AQ_THROW_IF
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <iostream>
#include <string.h>             // required for strlen and string length methods
#include <string>               // required for std::stod string to double conversion methods
#include <sstream>              // required for string to double via string stream (more precise)
#include <iomanip>              // for string stream precision

#if !defined(WIN32) && !defined(WIN64)
#include <values.h>
#endif

using namespace std;

//
//  ---------------------------- Internal StringData Class -----------------------------------------   
//

LAString::StringData::StringData( const unsigned int allocSize)
    : allocationSize_(allocSize), stringSize_(0)
{
    try
    {
        string_ = new char_t[allocationSize_];
        *string_ = '\0';
    }
    catch (bad_alloc & e)
    {
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
LAString::StringData::StringData( const char_t* pString)
{
    allocationSize_ = STRLEN(pString) + 1;
    try
    {
        string_ = new char_t[allocationSize_];
        STRCPY(string_, pString);
        stringSize_ = allocationSize_ - 1;
    }
    catch (bad_alloc & e)
    {
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

void
LAString::StringData::toUpper(void)
{
    char_t* str = string_;
    for(;(*str) != '\0'; ++str) 
    {
        *str = (char)toupper(*str);
    }
}

void
LAString::StringData::toLower(void)
{
    char_t* str = string_;
    for(;(*str) != '\0'; ++str) 
    {
        *str = (char)tolower(*str);
    }
}

void
LAString::StringData::exchange( char_t from, char_t to)
{
     char_t* str = string_;
     for(;(*str) != '\0'; ++str) 
     {
        if (*str == from)
        {
            *str = to;
        }
    }
}

void 
LAString::StringData::exchange(const char_t* pFromString, const char_t* pToString)
{
    // in case From is nullptr
    if (pFromString[0] == '\0' ) 
    {
       return;
    }
    
    // in case the same string
    unsigned int *pos = nullptr;
    unsigned int num = 0 ;
    unsigned int fromLen = STRLEN(pFromString);
    unsigned int toLen = STRLEN(pToString);
    int diff  = toLen - fromLen;

        
    char_t* pPos = string_;
	try
	{
        for(;(pPos = strstr(pPos, pFromString)) != nullptr;)
        {
            ++num;
            pos = (unsigned int*)realloc(pos, sizeof(unsigned int) * num);
            pos[num-1] = static_cast<unsigned int>(pPos - string_);
            pPos += fromLen;
        }
        if (diff > 0) 
        {
            for (unsigned int i = 0; i < num; ++i)
            {
                pos[i] += diff * i;
                extend(pos[i], diff);
                replace(pos[i], pToString);
            }
        }
        else
        {
			diff *= -1;
			for (unsigned int i = 0; i < num; ++i)
			{
				pos[i] -= diff * i;
				remove(pos[i], diff);
				replace(pos[i], pToString);
			}
        }
        free(pos);
    }
    catch (LACoreSystemError&)
    {
        free(pos);
        throw;
    }
    catch (...)
    {
        free(pos);
        throw LACoreSystemError("Unexpected Error at LAString::StringData::exchange", __FILE__, __LINE__);
    }

}

void 
LAString::StringData::replace( const unsigned int from, const char_t* pString)
{
    unsigned int len = STRLEN(pString);
    unsigned int newSize = len + from;

    if (newSize + 1 > allocationSize_) 
    {
        extend(from, newSize);
    }

    if (newSize > stringSize_) 
    {
        stringSize_ = newSize;
        STRCPY(string_ + from, pString);
    }
    else
    {
        char_t* ptwk = string_ + from;
        char_t* pfwk = const_cast<char_t*>(pString);
        for(;*pfwk != '\0';)
        {
            *(ptwk++) = *(pfwk++);
        }
    }
}

void 
LAString::StringData::insert(const unsigned int from, const char_t* pString)
{
    extend(from, STRLEN(pString));

    char_t* ptwk = string_ + from;
    char_t* pfwk = const_cast<char_t*>(pString);
    for(;(*pfwk) != '\0';)
    {
        *(ptwk++) = *(pfwk++);
    }
}

void
LAString::StringData::remove(const unsigned int from, const unsigned int num)
{
    if (from + num > stringSize_) 
    {
        string_[from] = '\0';
        stringSize_ = from;
    }
    else
    {
        char_t* toWk = string_ + from;
        char_t* frWk = string_ + from + num;
        for(;*frWk != '\0';)
        {
            *(toWk++) = *(frWk++);
        }
        (*toWk) = (*frWk);
        stringSize_ -= num;
    }
    if (4 * stringSize_ < allocationSize_) 
    {
        allocationSize_ = 2 * stringSize_+1;
        try
        {
            char_t* wk = new char_t[allocationSize_];
            char_t* toWk = wk;
            char_t* frWk = string_;
            for(;*frWk != '\0';)
            {
                *(toWk++) = *(frWk++);
            }
            (*toWk) = (*frWk);
             delete[] string_;
             string_ = wk;
        }
        catch (bad_alloc & e){
            throw LACoreSystemError(e.what(), __FILE__, __LINE__);
        }
    }
}

void
LAString::StringData::extend( const unsigned int fromSize, const unsigned int to)
{
    unsigned int    i;
    char_t* ptWk = nullptr;
    char_t* pfWk = nullptr;

    if(allocationSize_ < stringSize_ + to + 1) 
    {
        allocationSize_ = 2 * (stringSize_ + to + 1);
        try
        {
            char_t* pWk = new char_t[allocationSize_];
            memset(pWk, 0x00, allocationSize_ * sizeof(char_t));
            ptWk = pWk;
            pfWk = string_;
            for (i = 0; i < fromSize; ++i) 
            {
                *(ptWk++) = *(pfWk++);
            }
            ptWk += to;
            for(;*pfWk != '\0';)
            {
                *(ptWk++) = *(pfWk++);
            }
            *ptWk = *pfWk;
            delete[] string_;
            string_ = pWk;
        }
        catch (bad_alloc & e){
            throw LACoreSystemError(e.what(), __FILE__, __LINE__);
        }
    } 
    else 
    {
        ptWk = string_ + stringSize_ + to;
        pfWk = string_ + stringSize_;
        for(i = 0; i <= stringSize_ - fromSize + 1; ++i)
        {
            *(ptWk--) = *(pfWk--);
        }
    }
    stringSize_ = stringSize_ + to;
}


//
//  ---------------------------- LAString Class -----------------------------------------   
//

// Constructor
LAString::LAString(void) 
{
    init();
}

// Copy Constructor
LAString::LAString(const LAString& rString) 
{
    init(); 
    copy(rString);
}

// Char Constructor
LAString::LAString(const char_t inputChar)
{
    init(); 
    char_t str[2];
    str[0] = inputChar;
    str[1] = '\0';
    copy(str);
}

// Standard String Constructor
LAString::LAString(const std::string & standardString )
{ 
    init();
    copy(standardString.c_str());
}

// Char Pointer Constructor
LAString::LAString(const char_t* str)
{
    init();
    copy(str);
}

// Convert Int to String
LAString::LAString( const int a)
{
	// OLD - Slow
	// ---

    // init();
    // char_t str[INT_LEN];
    // unsigned int length;
    // length = SPRINTF(str, "%d", a);
    // copy(str);


	// NEW - Faster
	// ---

	init();
	std::string str = std::to_string(a);
	copy(str);


	// Alternative for Higher Precision
	// --------------------------------
	
	// init();
	// std::ostringstream streamObj;
	// streamObj.precision(15);
	// streamObj << std::fixed << a;
	// copy(streamObj.str());

}


// Convert Double to String
LAString::LAString(const double a, unsigned int stringDoubleSize)
{
	// OLD - Slow
	// ---

	// init();
	//
    // char_t str[DOUBLE_LEN];
    // char_t form[7];
	// 
    // if (stringDoubleSize > DOUBLE_LEN / 2) 
    // {
    //     stringDoubleSize = DOUBLE_LEN / 2;
    // }
	// 
	// int digit = DOUBLE_LEN - stringDoubleSize - 2;    // '2' means decimal point and minus sign 
    //             
    // if(LAMath::abs(a) < LAMath::pow(10, digit)) 
    // {
	// 	SPRINTF(form, "%%.%df", stringDoubleSize);
    // }
	// else
    // {
	// 	SPRINTF(form, "%%1.6e");
    // }
    //             
	// SPRINTF(str, form, a);
	// copy(str);


	// NEW - Faster
	// ---

	init();
	copy(std::to_string(a));


	// Alternative for Higher Precision
	// --------------------------------
	
	// init();
	// std::ostringstream streamObj;
	// streamObj.precision(15);
	// streamObj << std::fixed << a;
	// copy(streamObj.str());

}

// Destructor
LAString::~LAString(void) 
{
    clear();
}


const char_t* LAString::getCString(void) const
{
    return (stringData_ == nullptr) ? "": stringData_->getCString();
}

const char_t* LAString::c_str(void) const
{
    return getCString();
}


double LAString::getDoubleValue(void) const
{
    if (stringData_ == nullptr || stringData_->size() == 0) return 0.0;
    // Behaviour: Same as legacy code, where "abc" = 0, "2D" = 2, "2" = 2
    return stringToDouble(getCString());
}

int LAString::getIntValue(void) const
{
    if (stringData_ == nullptr || stringData_->size() == 0) return 0.0;
    // Behaviour: Same as legacy code, where "abc" = 0, "2D" = 2, "2" = 2
    return stringToInteger(getCString());
}


LAString LAString::subString(unsigned int start, unsigned int end) const 
{
    LAString ret(*this);

    if (end < start)
    {
        return ret;
    }
    if (++end != ret.size())
    {
        ret.remove(end, ret.size() - end);
    }
    if (start != 0)
    {
        ret.remove(0, start);
    }
    return ret;
}


unsigned int LAString::size(void) const
{
    return (stringData_ == nullptr) ? 0 : stringData_->size();
}

int LAString::findString(
    const LAString& rStr) const
{
    return findString(rStr.getCString());
}

int LAString::findString(
    const char_t* pPattern) const
{
    unsigned int        pattern_len;
    unsigned int        i, j;
    unsigned int        skip[256];
    
    // BM method
    if (pPattern == nullptr)
    {
        return -1;
    }
    pattern_len = STRLEN(pPattern);
    if (!pattern_len)
    {
        return -1;
    }

    for (i = 0;i < 256;i++)
    {
        skip[i] = pattern_len;
    }
    for (i = 0;i < pattern_len - 1;i++)
    {
        skip[static_cast<int>(pPattern[i])] = pattern_len - i - 1;
    }
    while(i < size())
    {
        j = pattern_len - 1;
        while (stringData_->getChar(i) == pPattern[j])
        {
            if (j == 0)
            {
                return i;
            }
            i--;
            j--;
        }
        if (skip[static_cast<int>(stringData_->getChar(i))] > pattern_len - j)
        {
            i = i + skip[static_cast<int>(stringData_->getChar(i))];
        }
        else
        {
            i = i + pattern_len - j;
       }
    }
    return -1;
}


int LAString::findString(const char_t c) const
{
    unsigned int        i;
    bool    found = false;
    for (i = 0; i < size();i++)
    {
        if (stringData_->getChar(i) == c)
        {
            found = true;
            break;
        }
    }
	// We are about to cast from (unsigned int) to (int).
	// Guard against the very unlikely case that the int might overflow.
    if (i > INT_MAX)
    {
        throw LACoreSystemError("String found at the point over INT_MAX.", __FILE__, __LINE__);
    }
    
    return found ? static_cast<int>(i) : -1;
}


LAString& 
LAString::toUpper(void)
{
    if ( stringData_ == nullptr ) return *this;
    makeUnShared();
    stringData_->toUpper();
    return *this;
}

LAString&
LAString::toLower(void)
{
    if ( stringData_ == nullptr ) return *this;
    makeUnShared();
    stringData_->toLower();
    return *this;
}

LAString& 
LAString::exchange( const char_t from, const char_t to)
{
    makeUnShared();
    stringData_->exchange(from, to);
    return *this;
}
   
LAString& 
LAString::exchange(const LAString& from, const LAString& to)
{
    if (! from.isDefined()) 
    {
        return *this;
    }
    makeUnShared();
    if (to.isDefined())
    {
        stringData_->exchange(from.getCString(), to.getCString());
    } 
    else
    {
        stringData_->exchange(from.getCString(), "");
    }
    return *this;
} 

LAString&
LAString::exchange(const  char_t* from, const LAString& to)
{
    if (from == nullptr) 
    {
        return *this;
    }
    makeUnShared();
    if (to.isDefined())
    {
        stringData_->exchange(from, to.getCString());
    } 
    else
    {
        stringData_->exchange(from, "");
    }
    return *this;
}

LAString&
LAString::exchange(const LAString& from, const char_t* to)
{
    if (! from.isDefined()) 
    {
        return *this;
    }
    makeUnShared();
    const char_t* to2 = (to == nullptr ? "": to);
    stringData_->exchange(from.getCString(), to2);
    return *this;
}

LAString&
LAString::exchange(const char_t* from, const char_t* to)
{
    if (from == nullptr) 
    {
        return *this;
    }
    makeUnShared();
    const char_t* to2 = (to == nullptr ? "": to);
    stringData_->exchange(from, to2);
    return *this;
}

LAString&
LAString::replace(const unsigned int from, const char_t* pStr)
{
    if (from > size() - 1)
    {
        return *this;
    }
    if (pStr == nullptr)
    {
        return *this;
    }
    makeUnShared();
    stringData_->replace(from, pStr); 
    return *this;
}

LAString&
LAString::replace(const unsigned int from, const LAString& pStr)
{
    if (from > size() - 1)
    {
        return *this;
    }
    if (!pStr.isDefined())
    {
        return *this;
    }
    makeUnShared();
    stringData_->replace(from, pStr.getCString());
    return *this;
}

LAString&
LAString::insert(unsigned int from, const LAString& rStr)
{
    if (from > size())
    {
        return *this;
    }
    if (!rStr.isDefined())
    {
        return *this;
    }
    makeUnShared();
    stringData_->insert(from, rStr.getCString());
    return *this;
}

LAString&
LAString::insert(unsigned int from, const char_t* pStr)
{
    if (pStr == nullptr)
    {
        return *this;
    }
    makeUnShared();
    stringData_->insert(from, pStr);
    return *this;
}

LAString&
LAString::remove(const unsigned int from, const unsigned int num)
{
    if (from > size() - 1)
    {
        return *this;
    }
    makeUnShared();
    stringData_->remove(from, num);
    return *this;
}

LAString&
LAString::padLeft(const unsigned int n, char_t c)
{
    char_t* str = nullptr;
    if (n == 0)
    {
        return *this;
    }
    try
    {
        str = new char_t[n+1];
        for (unsigned int i =0; i < n; ++i)
        {
            str[i] = c;
        }
        str[n] = '\0';
        insert(0, str);
        delete[] str;
        str = nullptr;
        return *this;
    }
    catch (bad_alloc & e)
    {
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief replaced with the specified character from the end of the string

    Replace with (c) the specified character n characters from the end of the string.
    If the string is not defined, the behavior is undetermined.

    @param[in] n number of characters to replace
    @param[in] c characters to replace

    @return string replaced
*/
LAString&
LAString::padRight(
    const unsigned int n, 
    char_t c)
{
    char_t* str = nullptr;
    if (n == 0)
    {
        return *this;
    }
    try
    {
        str = new char_t[n+1];
        for (unsigned int i =0; i < n; ++i)
        {
            str[i] = c;
        }
        str[n] = '\0';
        insert(size(), str);
        delete[] str;
        str = nullptr;
        return *this;
    }
    catch (bad_alloc & e)
    {
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief returns true if the entire string is made of whitespace characters
*/
bool
LAString::isWhiteSpace(void)
{
    const char_t* str = getCString();
    bool allSpace = true;
    for(unsigned int i = 0; i < size(); ++i)
                {
                                if ( ! isspace( (int) str[i]) )
                                {
                                                allSpace = false;
                                                break;
                                }
                }
                return allSpace;
}
        

/*!
    @brief delete a blank character at the beginning of the string

    @return string deleted
*/
LAString&
LAString::trimLeft(void)
{
    const char_t* str = getCString();
    unsigned int i;
    for(i = 0; i < size() && isspace((int)str[i]); ++i)
        ;  // nullptr
    if(i != 0) 
    {
        remove(0, i);
    }
    return *this;
}

/*!
    @brief delete a blank character at the end of the string

    @return string deleted
*/
LAString&
LAString::trimRight(void)
{
    const char_t* str = getCString();
    int i  = size()-1;
    for(; i >= 0 && isspace((int)str[i]); --i);  // nullptr
    if ((unsigned int)i != size() - 1) 
    {
        remove(i+1, size()-i-1);
    }
    return *this;
}

/*!
    @brief change the character with another character at the specified position

                If "index" is larger than the length of string -1 , then do nothing.
    If the string is not defined, the behavior is undetermined.

    @param[in] index position to change(0start)
    @param[in] c changed characters

    @return changed characters
*/
LAString& 
LAString::charUpdate(
    const unsigned int index, 
    const char_t c)
{
    if (index > size() - 1)
    {
        return *this;
    }
    makeUnShared();
    stringData_->getChar(index) = c;
    return *this;
}

/*!
    @brief decompose into multiple strings based on the specified delimiter character
    If the string is not defined, the behavior is undetermined.
    @param[in] del delimiter
    @return strings after decomposition(store multi strings into vector)
*/
vector<LAString>
LAString::toToken(
    const char_t del) const
{
    const char_t* str = getCString();
    vector<LAString> ret;
    for(;; ++str)
    {
        LAString tmp("");
        for(;*str != '\0' && *str != del; ++str)
        {
            tmp+= *str;
        }
        ret.push_back(tmp);
        if (*str == '\0') 
        {
            break;
        }
    }
    return ret;
}

/*!
    @brief assignment operator for immediate / debug window in Visual Studio 
*/
LAString&
LAString::assign( const char_t* inputString )
{
copy(inputString);
return *this;
}

LAString&
LAString::operator=(const std::string& inputString)
{
    copy(inputString.c_str());
    return *this;
}

LAString&
LAString::operator=(const LAString& inputString)
{
    copy(inputString);
    return *this;
}

LAString&
LAString::operator=(const char_t* inputString)
{
    copy(inputString);
    return *this;
}

LAString&
LAString::operator=(const char_t inputChar)
{
    char_t str[2];
    str[0] = inputChar;
    str[1] = '\0';
    copy(str);
    return *this;
}

LAString
LAString::operator+(const char_t* inputString) const 
{
    LAString retString(*this);
    if (inputString == nullptr)
    {
        return retString;
    }
    if (!isDefined())
    {
        return LAString(inputString);
    }
    retString += inputString;
    return retString;
}

LAString
LAString::operator+(const char_t inputChar) const 
{
    LAString retString(*this);
    LAString plusString(inputChar);
    if (!isDefined())
    {
        return plusString;
    }
    retString += plusString;
    return retString;
}

LAString
LAString::operator+(const std::string& inputString) const 
{
    LAString retString(*this);
    if (!isDefined())
    {
        return inputString.c_str();
    }
    retString += inputString.c_str();
    return retString;
}

LAString
LAString::operator+(const LAString& inputString) const 
{
    LAString retString(*this);
    if (!isDefined())
    {
        return inputString;
    }
    retString += inputString;
    return retString;
}

LAString&
LAString::operator+=(const std::string& inputString) 
{
    if (!isDefined())
    {
        return *this = inputString.c_str();
    }
    insert(size(), inputString.c_str());
    return *this;
}

LAString&
LAString::operator+=(const LAString& inputString) 
{
    if (!isDefined())
    {
        return *this = inputString;
    }
    insert(size(), inputString);
    return *this;
}

LAString&
LAString::operator+=(const char_t* inputString) 
{
    if (!isDefined())
    {
        return *this = inputString;
    }
    insert(size(), inputString);
    return *this;
}

LAString&
LAString::operator+=(const char_t c) 
{
    char_t a[2];
    a[0] = c;
    a[1] = '\0';
    if (!isDefined())
    {
        return *this = a;
    }
    insert(size(), a);
    return *this;
}

//============= Friend Methods =====================================

LAString operator+(const char_t* charString, const LAString& inputString) 
{
    LAString retString(charString);
    if (charString == nullptr)
    {   
        return inputString;
    }
    if (!inputString.isDefined())
    {
        return retString;
    }
    retString += inputString;
    return retString;
}

bool
operator==(const char_t* charString, const LAString& inputString) 
{
    return (inputString.cmp(charString)==0);
}

bool
operator!=(const char_t* charString, const LAString& inputString) 
{
    return !(inputString == charString);
}

bool
operator<=(const char_t* charString, const LAString& inputString) 
{
    return (inputString.cmp(charString)>=0);
}

/*!
    @brief relational operator
*/
bool
operator>=(
    const char_t* charString, 
    const LAString& inputString) 
{
    return (inputString.cmp(charString)<=0);
}

bool
operator<(const char_t* charString, const LAString& inputString) 
{
    return (inputString.cmp(charString)>0);
}

bool
operator>(const char_t* charString, const LAString& inputString) 
{
    return (inputString.cmp(charString)<0);
}

ostream& operator<< ( ostream& os, const LAString& st )
{
    os << st.getCString();
    return os;
}

istream& operator>> (istream& is,LAString& st )
{
    char_t  ischar;
    is.get(ischar);
    while (is.gcount() != 0)
    {
        st += ischar;
        is.get(ischar);
    }
    return is;
}

// String Comparison: LAString
int LAString::cmp(const LAString& rString) const
{
    if (stringData_ == rString.stringData_) return 0;
    else if (!isDefined()) return -1;
    else if (!rString.isDefined()) return 1;
    return stringData_->cmp(rString.getCString());
}

// String Comparison: Standard String std::string
int LAString::cmp(const std::string& rString) const
{
    LAString myLAString(rString);
    return cmp( myLAString );
}

// String Comparison: char_t*
int LAString::cmp(const char_t* pString) const
{
    if (!isDefined() && pString == nullptr) return 0;
    else if (!isDefined()) return -1;
    else if (pString == nullptr) return 1;
    return stringData_->cmp(pString);
}

// ------------------------- Private Methods ----------------------------------------

// Convert a string to double
double LAString::stringToDouble(const std::string & str) const
{
    // Pointer to first non-numeric input, can be used for error handling
    // We don't hande errors here to mimic legacy code e.g. if (*pEnd != 0) throw "Error";
    char* pEnd;
    
    // String to Double
    // Behaviour: Same as legacy code, where "abc" = 0, "2D" = 2, "2" = 2
    return strtod( str.c_str(), &pEnd );
}

// Convert a string to integer
int LAString::stringToInteger(const std::string & str) const
{
    // Pointer to first non-numeric input, can be used for error handling
    // We don't hande errors here to mimic legacy code e.g. if (*pEnd != 0) throw "Error";
    char* pEnd;
    
    // String to Long Int
    // Behaviour: Same as legacy code, where "abc" = 0, "2D" = 2, "2" = 2
    return strtol( str.c_str(), &pEnd, 10 ); // base 10 number format
}

void 
LAString::init(void)
{
    stringData_ = nullptr;
    refCount_ = nullptr;
}

void
LAString::clear(void)
{
    // Atomic Refernce Counter
    if (isDefined() && (*refCount_)-- == 1)
    {
        delete stringData_;
        delete refCount_;
    }
    init();
}

// Shallow Copy
void
LAString::copy(const LAString& rString)
{
    // no copy itself
    if (this == &rString) 
    {
        return;
    }

    clear();
    if (rString.isDefined()) 
    {
        // increment reference counter since shallow copy
        refCount_ = rString.refCount_;
        
        // Atomic Refernce Counter
        ++(*refCount_);
        
        stringData_ = rString.stringData_;  // shallow copy
    }
}

// Deep Copy
void
LAString::copy(const char_t* pString)
{
    if (isDefined() && stringData_->getCString() == pString) 
    {
        return;
    }

    clear();
    if (pString == nullptr) 
    {
        return;
    }
    try
	{
        stringData_ = new StringData(pString);
    }
    catch (bad_alloc & e)
    {
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
    refCount_ = new std::atomic<int>(1);
}

/*!
    @brief reset the reference count of the string pointer that this object holds

                If you want to (shallow)copy LAString object by copy method, then you must share data string between the original and referenced objects.
                By using this method, the shared data string is released, the reference counter is reset to 1. 
*/
void 
LAString::makeUnShared(void)
{
    // Atomic Refernce Counter
   if ((*refCount_) == 1) 
    {
        return;
    }
    --(*refCount_);
    refCount_ = new std::atomic<int>(1);

    try
    {
        stringData_ = new StringData(stringData_->getCString());
    }
    catch (bad_alloc & e)
    {
        delete refCount_;
        refCount_ = nullptr;
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
