#include "AQLString.h"

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#endif

#include "AQLBasic.h"
#include "ExceptionMacros.h"    // incl AQ_REQUIRE, AQ_THROW, AQ_THROW_IF
#include <algorithm>            // std::replace, used by the single-char exchange()
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

namespace
{
    // Replaces every occurrence of `from` in `s` with `to`. Mirrors the old hand-rolled
    // StringData::exchange(const char_t*, const char_t*)'s find-and-replace-all semantics, but via
    // std::basic_string's own find()/replace() instead of a manual strstr() loop over a position
    // array grown with realloc().
    void replaceAllOccurrences(std::basic_string<char_t>& s, const char_t* from, const char_t* to)
    {
        if (from == nullptr || from[0] == '\0')
        {
            return;
        }
        const std::basic_string<char_t> fromStr(from);
        const std::basic_string<char_t> toStr(to == nullptr ? "" : to);
        std::basic_string<char_t>::size_type pos = 0;
        while ((pos = s.find(fromStr, pos)) != std::basic_string<char_t>::npos)
        {
            s.replace(pos, fromStr.size(), toStr);
            pos += toStr.size();
        }
    }
}

//
//  ---------------------------- AQLString Class -----------------------------------------
//

// Constructor
AQLString::AQLString(void)
{
    init();
}

// Copy Constructor
AQLString::AQLString(const AQLString& rString)
{
    init();
    copy(rString);
}

// Move Constructor - steals rString's buffer outright: no allocation, no copy of any characters.
AQLString::AQLString(AQLString&& rString) noexcept
    : stringData_(std::move(rString.stringData_))
{
}

// Char Constructor
AQLString::AQLString(const char_t inputChar)
{
    init();
    char_t str[2];
    str[0] = inputChar;
    str[1] = '\0';
    copy(str);
}

// Standard String Constructor
AQLString::AQLString(const std::string & standardString )
{
    init();
    copy(standardString.c_str());
}

// Char Pointer Constructor
AQLString::AQLString(const char_t* str)
{
    init();
    copy(str);
}

// Convert Int to String
AQLString::AQLString( const int a)
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
AQLString::AQLString(const double a, unsigned int stringDoubleSize)
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
    // if(AQLMath::abs(a) < AQLMath::pow(10, digit))
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
AQLString::~AQLString(void)
{
    clear();
}


const char_t* AQLString::getCString(void) const noexcept
{
    return stringData_ ? stringData_->c_str() : "";
}

const char_t* AQLString::c_str(void) const noexcept
{
    return getCString();
}


double AQLString::getDoubleValue(void) const
{
    if (!stringData_ || stringData_->empty()) return 0.0;
    // Behaviour: Same as legacy code, where "abc" = 0, "2D" = 2, "2" = 2
    return stringToDouble(getCString());
}

int AQLString::getIntValue(void) const
{
    if (!stringData_ || stringData_->empty()) return 0.0;
    // Behaviour: Same as legacy code, where "abc" = 0, "2D" = 2, "2" = 2
    return stringToInteger(getCString());
}


AQLString AQLString::subString(unsigned int start, unsigned int end) const
{
    AQLString ret(*this);

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


unsigned int AQLString::size(void) const noexcept
{
    return stringData_ ? static_cast<unsigned int>(stringData_->size()) : 0;
}

int AQLString::findString(
    const AQLString& rStr) const
{
    return findString(rStr.getCString());
}

int AQLString::findString(
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
    // Indices into skip[] must go through unsigned char first: char_t is a plain (signed, under MSVC)
    // char, so any byte with the high bit set would otherwise widen to a negative int and index
    // skip[] out of bounds.
    for (i = 0;i < pattern_len - 1;i++)
    {
        skip[static_cast<unsigned char>(pPattern[i])] = pattern_len - i - 1;
    }
    while(i < size())
    {
        j = pattern_len - 1;
        while ((*stringData_)[i] == pPattern[j])
        {
            if (j == 0)
            {
                return i;
            }
            i--;
            j--;
        }
        if (skip[static_cast<unsigned char>((*stringData_)[i])] > pattern_len - j)
        {
            i = i + skip[static_cast<unsigned char>((*stringData_)[i])];
        }
        else
        {
            i = i + pattern_len - j;
       }
    }
    return -1;
}


int AQLString::findString(const char_t c) const
{
    unsigned int        i;
    bool    found = false;
    for (i = 0; i < size();i++)
    {
        if ((*stringData_)[i] == c)
        {
            found = true;
            break;
        }
    }
	// We are about to cast from (unsigned int) to (int).
	// Guard against the very unlikely case that the int might overflow.
    if (i > INT_MAX)
    {
        throw AQLCoreSystemError("String found at the point over INT_MAX.", __FILE__, __LINE__);
    }

    return found ? static_cast<int>(i) : -1;
}

bool AQLString::startsWith(const AQLString& pString) const noexcept
{
    return startsWith(pString.getCString());
}

bool AQLString::startsWith(const char_t* pString) const noexcept
{
    if (pString == nullptr) return false;
    const unsigned int prefixLen = STRLEN(pString);
    if (prefixLen > size()) return false;
    if (prefixLen == 0) return true;
    return memcmp(getCString(), pString, prefixLen * sizeof(char_t)) == 0;
}

bool AQLString::endsWith(const AQLString& pString) const noexcept
{
    return endsWith(pString.getCString());
}

bool AQLString::endsWith(const char_t* pString) const noexcept
{
    if (pString == nullptr) return false;
    const unsigned int suffixLen = STRLEN(pString);
    if (suffixLen > size()) return false;
    if (suffixLen == 0) return true;
    return memcmp(getCString() + (size() - suffixLen), pString, suffixLen * sizeof(char_t)) == 0;
}

bool AQLString::contains(const AQLString& pString) const noexcept
{
    return findString(pString) != -1;
}

bool AQLString::contains(const char_t* pString) const noexcept
{
    return findString(pString) != -1;
}


AQLString&
AQLString::toUpper(void)
{
    if (!stringData_) return *this;
    for (char_t& ch : *stringData_)
    {
        ch = (char_t)toupper((int)ch);
    }
    return *this;
}

AQLString&
AQLString::toLower(void)
{
    if (!stringData_) return *this;
    for (char_t& ch : *stringData_)
    {
        ch = (char_t)tolower((int)ch);
    }
    return *this;
}

AQLString&
AQLString::toUpperTrimmed(bool trimWhiteSpace)
{
    if (trimWhiteSpace)
    {
        trimLeft();
        trimRight();
    }
    toUpper();
    return *this;
}

AQLString&
AQLString::exchange( const char_t from, const char_t to)
{
    if (!stringData_) return *this;
    std::replace(stringData_->begin(), stringData_->end(), from, to);
    return *this;
}

AQLString&
AQLString::exchange(const AQLString& from, const AQLString& to)
{
    if (!stringData_ || !from.isDefined())
    {
        return *this;
    }
    replaceAllOccurrences(*stringData_, from.getCString(), to.isDefined() ? to.getCString() : "");
    return *this;
}

AQLString&
AQLString::exchange(const  char_t* from, const AQLString& to)
{
    if (!stringData_ || from == nullptr)
    {
        return *this;
    }
    replaceAllOccurrences(*stringData_, from, to.isDefined() ? to.getCString() : "");
    return *this;
}

AQLString&
AQLString::exchange(const AQLString& from, const char_t* to)
{
    if (!stringData_ || !from.isDefined())
    {
        return *this;
    }
    replaceAllOccurrences(*stringData_, from.getCString(), to == nullptr ? "" : to);
    return *this;
}

AQLString&
AQLString::exchange(const char_t* from, const char_t* to)
{
    if (!stringData_ || from == nullptr)
    {
        return *this;
    }
    replaceAllOccurrences(*stringData_, from, to == nullptr ? "" : to);
    return *this;
}

AQLString&
AQLString::replace(const unsigned int from, const char_t* pStr)
{
    // size() - 1 underflows to UINT_MAX when size() == 0 (unsigned arithmetic), which silently
    // defeats this guard on an empty/undefined string instead of catching it - check size() directly.
    if (size() == 0 || from >= size())
    {
        return *this;
    }
    if (pStr == nullptr)
    {
        return *this;
    }
    // std::basic_string::replace's count is auto-clamped to size()-from when it would overrun (the
    // standard (pos,count) convention shared with substr()/erase()) - that single call reproduces
    // both of the old StringData::replace's branches: grow the string when pStr runs past the
    // current end, or overwrite in place (preserving whatever tail follows) when it doesn't.
    const std::basic_string<char_t> repl(pStr);
    stringData_->replace(from, repl.size(), repl);
    return *this;
}

AQLString&
AQLString::replace(const unsigned int from, const AQLString& pStr)
{
    if (size() == 0 || from >= size())
    {
        return *this;
    }
    if (!pStr.isDefined())
    {
        return *this;
    }
    stringData_->replace(from, pStr.size(), pStr.getCString());
    return *this;
}

AQLString&
AQLString::insert(unsigned int from, const AQLString& rStr)
{
    if (from > size())
    {
        return *this;
    }
    if (!rStr.isDefined())
    {
        return *this;
    }
    if (!stringData_) stringData_.emplace();
    stringData_->insert(from, rStr.getCString());
    return *this;
}

AQLString&
AQLString::insert(unsigned int from, const char_t* pStr)
{
    if (pStr == nullptr)
    {
        return *this;
    }
    // The original StringData-based insert() had no such guard: an out-of-range `from` reached
    // StringData::extend(), where an unsigned subtraction (stringSize_ - fromSize) underflowed and
    // drove a large out-of-bounds copy loop - a real buffer-overflow bug, not just a latent crash.
    // std::string::insert would throw std::out_of_range safely even without this guard, but the
    // guard keeps this overload's silent-no-op contract consistent with its AQLString& sibling.
    if (from > size())
    {
        return *this;
    }
    if (!stringData_) stringData_.emplace();
    stringData_->insert(from, pStr);
    return *this;
}

AQLString&
AQLString::remove(const unsigned int from, const unsigned int num)
{
    if (size() == 0 || from >= size())
    {
        return *this;
    }
    // erase()'s count is auto-clamped the same way replace()'s is above, reproducing both of the
    // original StringData::remove's branches with one call. Deliberately not reproducing the
    // original's "shrink the buffer if usage drops below 1/4 of capacity" logic - that forced a
    // reallocation on every qualifying erase(), a net perf loss versus std::string's normal
    // (no auto-shrink) capacity behaviour.
    stringData_->erase(from, num);
    return *this;
}

AQLString&
AQLString::padLeft(const unsigned int n, char_t c)
{
    if (n == 0)
    {
        return *this;
    }
    try
    {
        if (!stringData_) stringData_.emplace();
        stringData_->insert(stringData_->begin(), n, c);
    }
    catch (bad_alloc & e)
    {
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
    return *this;
}

/*!
    @brief replaced with the specified character from the end of the string

    Replace with (c) the specified character n characters from the end of the string.
    If the string is not defined, the behavior is undetermined.

    @param[in] n number of characters to replace
    @param[in] c characters to replace

    @return string replaced
*/
AQLString&
AQLString::padRight(
    const unsigned int n,
    char_t c)
{
    if (n == 0)
    {
        return *this;
    }
    try
    {
        if (!stringData_) stringData_.emplace();
        stringData_->append(n, c);
    }
    catch (bad_alloc & e)
    {
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
    return *this;
}

/*!
    @brief returns true if the entire string is made of whitespace characters
*/
bool
AQLString::isWhiteSpace(void)
{
    if (!stringData_) return true;
    for (char_t ch : *stringData_)
    {
        if (!isspace((int)ch))
        {
            return false;
        }
    }
    return true;
}


/*!
    @brief delete a blank character at the beginning of the string

    @return string deleted
*/
AQLString&
AQLString::trimLeft(void)
{
    if (!stringData_) return *this;
    std::basic_string<char_t>::size_type i = 0;
    while (i < stringData_->size() && isspace((int)(*stringData_)[i]))
    {
        ++i;
    }
    if (i != 0)
    {
        stringData_->erase(0, i);
    }
    return *this;
}

/*!
    @brief delete a blank character at the end of the string

    @return string deleted
*/
AQLString&
AQLString::trimRight(void)
{
    if (!stringData_ || stringData_->empty()) return *this;
    std::basic_string<char_t>::size_type i = stringData_->size();
    while (i > 0 && isspace((int)(*stringData_)[i - 1]))
    {
        --i;
    }
    if (i != stringData_->size())
    {
        stringData_->erase(i);
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
AQLString&
AQLString::charUpdate(
    const unsigned int index,
    const char_t c)
{
    if (size() == 0 || index >= size())
    {
        return *this;
    }
    (*stringData_)[index] = c;
    return *this;
}

/*!
    @brief decompose into multiple strings based on the specified delimiter character
    If the string is not defined, the behavior is undetermined.
    @param[in] del delimiter
    @return strings after decomposition(store multi strings into vector)
*/
vector<AQLString>
AQLString::toToken(
    const char_t del) const
{
    const char_t* str = getCString();
    vector<AQLString> ret;
    for(;; ++str)
    {
        AQLString tmp("");
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
AQLString&
AQLString::assign( const char_t* inputString )
{
copy(inputString);
return *this;
}

AQLString&
AQLString::operator=(const std::string& inputString)
{
    copy(inputString.c_str());
    return *this;
}

AQLString&
AQLString::operator=(const AQLString& inputString)
{
    copy(inputString);
    return *this;
}

AQLString&
AQLString::operator=(AQLString&& inputString) noexcept
{
    if (this != &inputString)
    {
        stringData_ = std::move(inputString.stringData_);
    }
    return *this;
}

AQLString&
AQLString::operator=(const char_t* inputString)
{
    copy(inputString);
    return *this;
}

AQLString&
AQLString::operator=(const char_t inputChar)
{
    char_t str[2];
    str[0] = inputChar;
    str[1] = '\0';
    copy(str);
    return *this;
}

AQLString
AQLString::operator+(const char_t* inputString) const
{
    AQLString retString(*this);
    if (inputString == nullptr)
    {
        return retString;
    }
    if (!isDefined())
    {
        return AQLString(inputString);
    }
    retString += inputString;
    return retString;
}

AQLString
AQLString::operator+(const char_t inputChar) const
{
    AQLString retString(*this);
    AQLString plusString(inputChar);
    if (!isDefined())
    {
        return plusString;
    }
    retString += plusString;
    return retString;
}

AQLString
AQLString::operator+(const std::string& inputString) const
{
    AQLString retString(*this);
    if (!isDefined())
    {
        return inputString.c_str();
    }
    retString += inputString.c_str();
    return retString;
}

AQLString
AQLString::operator+(const AQLString& inputString) const
{
    AQLString retString(*this);
    if (!isDefined())
    {
        return inputString;
    }
    retString += inputString;
    return retString;
}

AQLString&
AQLString::operator+=(const std::string& inputString)
{
    if (!isDefined())
    {
        return *this = inputString.c_str();
    }
    insert(size(), inputString.c_str());
    return *this;
}

AQLString&
AQLString::operator+=(const AQLString& inputString)
{
    if (!isDefined())
    {
        return *this = inputString;
    }
    insert(size(), inputString);
    return *this;
}

AQLString&
AQLString::operator+=(const char_t* inputString)
{
    if (!isDefined())
    {
        return *this = inputString;
    }
    insert(size(), inputString);
    return *this;
}

AQLString&
AQLString::operator+=(const char_t c)
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

AQLString operator+(const char_t* charString, const AQLString& inputString)
{
    AQLString retString(charString);
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
operator==(const char_t* charString, const AQLString& inputString)
{
    return (inputString.cmp(charString)==0);
}

bool
operator!=(const char_t* charString, const AQLString& inputString)
{
    return !(inputString == charString);
}

bool
operator<=(const char_t* charString, const AQLString& inputString)
{
    return (inputString.cmp(charString)>=0);
}

/*!
    @brief relational operator
*/
bool
operator>=(
    const char_t* charString,
    const AQLString& inputString)
{
    return (inputString.cmp(charString)<=0);
}

bool
operator<(const char_t* charString, const AQLString& inputString)
{
    return (inputString.cmp(charString)>0);
}

bool
operator>(const char_t* charString, const AQLString& inputString)
{
    return (inputString.cmp(charString)<0);
}

ostream& operator<< ( ostream& os, const AQLString& st )
{
    os << st.getCString();
    return os;
}

istream& operator>> (istream& is,AQLString& st )
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

// String Comparison: AQLString
int AQLString::cmp(const AQLString& rString) const
{
    // Two undefined strings compare equal - matches the old pointer-identity fast path
    // (stringData_ == rString.stringData_, true when both were null), now expressed directly since
    // there's no shared pointer to compare any more.
    if (!isDefined() && !rString.isDefined()) return 0;
    if (!isDefined()) return -1;
    if (!rString.isDefined()) return 1;
    // Sign-only equivalence with the old char-difference comparator verified sufficient: every
    // caller (the ==/!=/<=/>=/</> operator overloads) only tests cmp()'s sign, never its magnitude.
    return stringData_->compare(*rString.stringData_);
}

// String Comparison: Standard String std::string
int AQLString::cmp(const std::string& rString) const
{
    AQLString myAQLString(rString);
    return cmp( myAQLString );
}

// String Comparison: char_t*
int AQLString::cmp(const char_t* pString) const
{
    if (!isDefined() && pString == nullptr) return 0;
    else if (!isDefined()) return -1;
    else if (pString == nullptr) return 1;
    return stringData_->compare(pString);
}

// Case-insensitive three-way comparison. Deliberately not implemented by uppercasing temporary
// copies of both sides and calling compare() - that allocates twice per comparison, wasteful for
// something meant to be usable in hot lookup paths (matching a calendar/currency code case-
// insensitively). Direct char-by-char comparison, same signed-char-to-int-via-unsigned-char care
// toUpper()/toLower() already take elsewhere in this file, no allocation either way.
int AQLString::compareIgnoreCase(const AQLString& rString) const noexcept
{
    if (!isDefined() && !rString.isDefined()) return 0;
    if (!isDefined()) return -1;
    if (!rString.isDefined()) return 1;
    return compareIgnoreCase(rString.getCString());
}

int AQLString::compareIgnoreCase(const std::string& rString) const noexcept
{
    // Routed through an AQLString, not rString.c_str() directly, to match cmp(const std::string&)'s
    // existing pattern - c_str() is always `const char*`, which only matches char_t in narrow
    // builds; the AQLString(const std::string&) constructor is the one place that distinction is
    // already handled.
    AQLString myAQLString(rString);
    return compareIgnoreCase(myAQLString);
}

int AQLString::compareIgnoreCase(const char_t* pString) const noexcept
{
    if (!isDefined() && pString == nullptr) return 0;
    if (!isDefined()) return -1;
    if (pString == nullptr) return 1;

    const char_t* a = getCString();
    const char_t* b = pString;
    while (*a != '\0' && *b != '\0')
    {
        const int ca = tolower(static_cast<unsigned char>(*a));
        const int cb = tolower(static_cast<unsigned char>(*b));
        if (ca != cb) return ca - cb;
        ++a;
        ++b;
    }
    return static_cast<int>(static_cast<unsigned char>(*a)) - static_cast<int>(static_cast<unsigned char>(*b));
}

bool AQLString::equalsIgnoreCase(const AQLString& rString) const noexcept
{
    return compareIgnoreCase(rString) == 0;
}

bool AQLString::equalsIgnoreCase(const std::string& rString) const noexcept
{
    return compareIgnoreCase(rString) == 0;
}

bool AQLString::equalsIgnoreCase(const char_t* pString) const noexcept
{
    return compareIgnoreCase(pString) == 0;
}

// ------------------------- Private Methods ----------------------------------------

// Convert a string to double
double AQLString::stringToDouble(const std::string & str) const
{
    // Pointer to first non-numeric input, can be used for error handling
    // We don't hande errors here to mimic legacy code e.g. if (*pEnd != 0) throw "Error";
    char* pEnd;

    // String to Double
    // Behaviour: Same as legacy code, where "abc" = 0, "2D" = 2, "2" = 2
    return strtod( str.c_str(), &pEnd );
}

// Convert a string to integer
int AQLString::stringToInteger(const std::string & str) const
{
    // Pointer to first non-numeric input, can be used for error handling
    // We don't hande errors here to mimic legacy code e.g. if (*pEnd != 0) throw "Error";
    char* pEnd;

    // String to Long Int
    // Behaviour: Same as legacy code, where "abc" = 0, "2D" = 2, "2" = 2
    return strtol( str.c_str(), &pEnd, 10 ); // base 10 number format
}

void
AQLString::init(void)
{
    stringData_.reset();
}

void
AQLString::clear(void)
{
    stringData_.reset();
}

// Shallow Copy (name kept from the COW era - now a plain, real copy of the optional<string>)
void
AQLString::copy(const AQLString& rString)
{
    // no copy itself
    if (this == &rString)
    {
        return;
    }
    stringData_ = rString.stringData_;
}

// Deep Copy
void
AQLString::copy(const char_t* pString)
{
    // Pointer-identity check, not content equality: guards specifically against self-reassignment
    // through our own buffer, e.g. `s = s.c_str();`, where clearing first would free the very memory
    // pString points into before we ever read it.
    if (isDefined() && getCString() == pString)
    {
        return;
    }

    if (pString == nullptr)
    {
        stringData_.reset();
        return;
    }
    try
    {
        stringData_.emplace(pString);
    }
    catch (bad_alloc & e)
    {
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
