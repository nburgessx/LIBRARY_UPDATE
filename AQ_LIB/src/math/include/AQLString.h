#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#pragma warning( disable : 4267 )
#endif

#include "AQLCoreSystemError.h"
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#define AQLSTRING_DOUBLESIZE 18  // the number of digits after the decimal point
#define DOUBLE_LEN 64           // the maximum number of digits with double type
#define INT_LEN    16           // the maximum number of digits with int integer type

typedef std::istream            InStream;
typedef std::ostream            OutStream;
typedef std::iostream           IOStream;

//==========================================================================
/*!
    @brief Class to represent the string.
*/
class AQLString
{

public:

    // default constructor
    AQLString(void);

    // copy constructor
    AQLString(const AQLString& rString);

    // move constructor - steals the other string's buffer outright, no allocation at all
    AQLString(AQLString&& rString) noexcept;

    // constructor
    explicit AQLString(const char_t  inputChar);

    // constructor
    AQLString(const std::string & standardString );

    // constructor
    AQLString(const char_t* pString);

    // constructor
    explicit AQLString(const int c);

    // constructor
    explicit AQLString(const double c, unsigned int stringDoubleSize=AQLSTRING_DOUBLESIZE);

    // destructor
    ~AQLString(void);


    // get a string terminated by a nullptr
    const char_t*       getCString(void) const noexcept;

    // get a string terminated by a nullptr
    const char_t*       c_str(void) const noexcept;

    // get the value that has been converted to a double number from a string
    double              getDoubleValue(void) const;

    // get the value that has been converted to an integer number from a string
    int                 getIntValue(void) const;

    // extract the string
    AQLString            subString(unsigned int start, unsigned int end) const;

    // get the number of characters
    unsigned int        size(void) const noexcept;

    // return whether a string has been defined
    bool                isDefined(void) const noexcept {return stringData_.has_value();}

    // return the position of the beginning of the string to search for a string that you specify
    int                 findString(const AQLString& rStr) const;

    // return the position of the beginning of the string to search for a string that you specify
    int                 findString(const char_t* pString) const;

    // return the character position of the beginning of the search for the string that you specify
    int                 findString(const char_t c) const;

    // convert all characters to uppercase
    AQLString&           toUpper(void);

    // convert all characters to lowercase
    AQLString&           toLower(void);

    // replace the characters in the string
    AQLString&           exchange(const char_t from, const char_t to);

    // replace the string in a string
    AQLString&           exchange(const AQLString& from, const AQLString& to);

    // replace the string in a string
    AQLString&           exchange(const char_t* from, const AQLString& to);

    // replace the string in a string
    AQLString&           exchange(const AQLString& from, const char_t* to);

    // replace the string in a string
    AQLString&           exchange(const char_t* from, const char_t* to);

    // overwrite part of a string with the specified string
    AQLString&           replace(const unsigned int from, const char_t* pStr);

    // overwrite part of a string with the specified string
    AQLString&           replace(const unsigned int from, const AQLString& pStr);

    // insert a string at the specified location
    AQLString&           insert(const unsigned int from, const AQLString& rStr);

    // insert a string at the specified location
    AQLString&           insert(const unsigned int from, const char_t* pStr);

    // remove a string from the specified location
    AQLString&           remove(const unsigned int from, const unsigned int num);

    // replaced with the specified character from the beginning of the string
    AQLString&           padLeft(const unsigned int n, char_t c=' ');

    // replaced with the specified character from the end of the string
    AQLString&           padRight(const unsigned int n, char_t c=' ');

	// returns true if the string consists entirely of whitespaxe
	bool                isWhiteSpace();

    // delete a blank character at the beginning of the string
    AQLString&           trimLeft(void);

    // delete a blank character at the end of the string
    AQLString&           trimRight(void);

    // change the character with another character at the specified position
    AQLString&           charUpdate(const unsigned int index, const char_t c);

    // decompose into multiple strings based on the specified delimiter character
    std::vector<AQLString>   toToken(const char_t del) const;

    // array subscript operator
    const char_t&       operator[] (const unsigned int index) const {return (*stringData_)[index];}

    //  ------------------------------------------------------------------------------------
    // *** FOR IMMEDIATE WINDOW in VISUAL STUDIO TO AMEND STRINGS WHEN DEBUGGING ***
    //

    AQLString& assign( const char_t* inputString );

    //  ------------------------------------------------------------------------------------

    // String Operators
    AQLString&           operator = (const std::string & inputString);
    AQLString&           operator = (const AQLString & inputString);
    AQLString&           operator = (AQLString && inputString) noexcept;
    AQLString&           operator = (const char_t* inputString);
    AQLString&           operator = (const char_t inputChar);

    AQLString            operator + (const std::string & inputString) const;
    AQLString            operator + (const AQLString & inputString) const;
    AQLString            operator + (const char_t* inputString) const;
    AQLString            operator + (const char_t  inputChar) const;

    AQLString&           operator += (const std::string & inputString);
    AQLString&           operator += (const AQLString & inputString);
    AQLString&           operator += (const char_t* inputString);
    AQLString&           operator += (const char_t c);

    bool                operator == (const std::string & inputString) const     { return  (cmp(inputString) == 0);  }
    bool                operator == (const AQLString & inputString) const        { return  (cmp(inputString) == 0);  }
    bool                operator == (const char_t* inputString) const           { return  (cmp(inputString) == 0);  }

    bool                operator != (const std::string & inputString) const     { return !(cmp(inputString) == 0);  }
    bool                operator != (const AQLString & inputString) const        { return !(cmp(inputString) == 0);  }
    bool                operator != (const char_t* inputString) const           { return !(cmp(inputString) == 0);  }

    bool                operator <= (const std::string & inputString) const     { return  (cmp(inputString) <= 0);  }
    bool                operator <= (const AQLString & inputString) const        { return  (cmp(inputString) <= 0);  }
    bool                operator <= (const char_t* inputString) const           { return  (cmp(inputString) <= 0);  }

    bool                operator >= (const std::string & inputString) const     { return  (cmp(inputString) >= 0);  }
    bool                operator >= (const AQLString & inputString) const        { return  (cmp(inputString) >= 0);  }
    bool                operator >= (const char_t* inputString) const           { return  (cmp(inputString) >= 0);  }

    bool                operator < (const std::string & inputString) const      { return  (cmp(inputString) < 0);   }
    bool                operator < (const AQLString & inputString) const         { return  (cmp(inputString) < 0);   }
    bool                operator < (const char_t* inputString) const            { return  (cmp(inputString) < 0);   }

    bool                operator > (const std::string & inputString) const      { return  (cmp(inputString) > 0);   }
    bool                operator > (const AQLString & inputString) const         { return  (cmp(inputString) > 0);   }
    bool                operator > (const char_t* inputString) const            { return  (cmp(inputString) > 0);   }

    // Friend Operators
    friend AQLString     operator + (const char_t* charString1, const AQLString & inputString);

    friend bool         operator == (const char_t* charString, const AQLString & inputString);
    friend bool         operator != (const char_t* charString, const AQLString & inputString);
    friend bool         operator <= (const char_t* charString, const AQLString & inputString);
    friend bool         operator >= (const char_t* charString, const AQLString & inputString);
    friend bool         operator < (const char_t* charString, const AQLString & inputString);
    friend bool         operator > (const char_t* charString, const AQLString & inputString);

    friend OutStream&   operator<< ( OutStream& os, const AQLString& st );
    friend InStream&    operator>> ( InStream& is, AQLString& st );

public:

    // String Comparison: AQLString
    int cmp(const AQLString& rString) const;

    // String Comparison: Standard String std::string
    int cmp(const std::string& rString) const;

    // String Comparison: char_t*
    int cmp(const char_t* pString) const;


private:

    // Convert a string to double
    double stringToDouble(const std::string & str) const;

    // Convert a string to integer
    int stringToInteger(const std::string & str) const;

    // perform the initialization process, called only from constructor
    void                init(void);

    //  in consideration of the reference counter, make clear the member, and init should be called
    void                clear(void);

    // Shallow Copy
    void                copy(const AQLString& rString);

    // Deep Copy
    void                copy(const char_t* pString);


    //
    // MEMBER VARIABLES
    //
    // ------------------------------------------------------------------------------------------------------------------------

    // std::nullopt ("undefined") vs a defined-but-empty string are two different states the rest of
    // this class's public API depends on (isDefined(), and the early-return-if-undefined guards
    // throughout exchange()/replace()/insert()/etc) - a bare std::basic_string<char_t> can't
    // represent that distinction on its own (there's no "no string" state, only ""), hence the
    // optional wrapper.
    //
    // This used to be copy-on-write: first a raw StringData* + a separate, independently-allocated
    // std::atomic<int>* refcount (looked thread-safe because the count was atomic, but wasn't - the
    // count only protected the number from corruption, not the payload it counted references to;
    // see git history for the full account), then a std::shared_ptr<StringData> (which did fix that
    // race, via shared_ptr's correct acquire/release pairing, but still meant two heap allocations
    // for the StringData class's own hand-rolled buffer - manual new[]/delete[], a manual extend()
    // for buffer growth, malloc/realloc/free mixed in for exchange() - none of which does anything
    // std::basic_string doesn't already do, with decades more hardening behind it).
    //
    // Now it's a plain, uniquely-owned std::basic_string<char_t>: no sharing, so no race to have;
    // real small-string-optimization, so the short strings this library actually deals with in bulk
    // (currency codes, calendar codes, YYYYMMDD-style date strings) cost zero heap allocations
    // instead of the old design's two; real move semantics, inherited rather than hand-written.
    // Copying a long, shared string is no longer O(1) the way COW made it - a deliberate trade,
    // since this library's string usage is dominated by short strings, exactly the case where SSO
    // wins outright over a refcount bump.
    std::optional<std::basic_string<char_t>>     stringData_;

    // ------------------------------------------------------------------------------------------------------------------------
};


bool operator == (const char_t* s, const AQLString & inputString);
bool operator != (const char_t* s, const AQLString & inputString);
bool operator <= (const char_t* s, const AQLString & inputString);
bool operator >= (const char_t* s, const AQLString & inputString);
bool operator < (const char_t* s, const AQLString & inputString);
bool operator > (const char_t* s, const AQLString & inputString);
std::ostream& operator<< ( std::ostream& os, const AQLString& st );

AQLString  operator + (const char_t* aString1, const AQLString & inputString);
