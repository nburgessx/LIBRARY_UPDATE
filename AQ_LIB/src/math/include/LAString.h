#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#pragma warning( disable : 4267 )
#endif

#include "LACoreSystemError.h"
#include <iostream>
#include <vector>
#include <atomic>

#define LASTRING_DOUBLESIZE 18  // the number of digits after the decimal point
#define DOUBLE_LEN 64           // the maximum number of digits with double type
#define INT_LEN    16           // the maximum number of digits with int integer type

typedef std::istream            InStream;
typedef std::ostream            OutStream;
typedef std::iostream           IOStream;

//==========================================================================
/*! 
    @brief Class to represent the string.
*/
class LAString 
{

    class StringData;

public:

    // default constructor
    LAString(void);
    
    // copy constructor
    LAString(const LAString& rString);
    
    // constructor
    explicit LAString(const char_t  inputChar);
    
    // constructor
    LAString(const std::string & standardString );

    // constructor
    LAString(const char_t* pString);
    
    // constructor
    explicit LAString(const int c);
    
    // constructor
    explicit LAString(const double c, unsigned int stringDoubleSize=LASTRING_DOUBLESIZE);
    
    // destructor
    ~LAString(void);


    // get a string terminated by a nullptr
    const char_t*       getCString(void) const;

    // get a string terminated by a nullptr
    const char_t*       c_str(void) const;

    // get the value that has been converted to a double number from a string
    double              getDoubleValue(void) const;

    // get the value that has been converted to an integer number from a string
    int                 getIntValue(void) const;

    // extract the string
    LAString            subString(unsigned int start, unsigned int end) const;

    // get the number of characters
    unsigned int        size(void) const;

    // return whether a string has been defined
    bool                isDefined(void) const {return refCount_ != nullptr;}

    // return the position of the beginning of the string to search for a string that you specify
    int                 findString(const LAString& rStr) const;

    // return the position of the beginning of the string to search for a string that you specify
    int                 findString(const char_t* pString) const;

    // return the character position of the beginning of the search for the string that you specify
    int                 findString(const char_t c) const;

    // convert all characters to uppercase
    LAString&           toUpper(void);

    // convert all characters to lowercase
    LAString&           toLower(void);

    // replace the characters in the string
    LAString&           exchange(const char_t from, const char_t to);
    
    // replace the string in a string
    LAString&           exchange(const LAString& from, const LAString& to);
    
    // replace the string in a string
    LAString&           exchange(const char_t* from, const LAString& to);
    
    // replace the string in a string
    LAString&           exchange(const LAString& from, const char_t* to);
    
    // replace the string in a string
    LAString&           exchange(const char_t* from, const char_t* to);
                                                                        
    // overwrite part of a string with the specified string 
    LAString&           replace(const unsigned int from, const char_t* pStr);

    // overwrite part of a string with the specified string 
    LAString&           replace(const unsigned int from, const LAString& pStr);

    // insert a string at the specified location
    LAString&           insert(const unsigned int from, const LAString& rStr);

    // insert a string at the specified location
    LAString&           insert(const unsigned int from, const char_t* pStr);

    // remove a string from the specified location
    LAString&           remove(const unsigned int from, const unsigned int num);

    // replaced with the specified character from the beginning of the string
    LAString&           padLeft(const unsigned int n, char_t c=' ');

    // replaced with the specified character from the end of the string
    LAString&           padRight(const unsigned int n, char_t c=' ');

	// returns true if the string consists entirely of whitespaxe
	bool                isWhiteSpace();

    // delete a blank character at the beginning of the string
    LAString&           trimLeft(void);

    // delete a blank character at the end of the string
    LAString&           trimRight(void);

    // change the character with another character at the specified position
    LAString&           charUpdate(const unsigned int index, const char_t c);

    // decompose into multiple strings based on the specified delimiter character 
    std::vector<LAString>   toToken(const char_t del) const;

    // array subscript operator
    const char_t&       operator[] (const unsigned int index) const {return stringData_->getChar(index);}
    
    //  ------------------------------------------------------------------------------------
    // *** FOR IMMEDIATE WINDOW in VISUAL STUDIO TO AMEND STRINGS WHEN DEBUGGING ***
    //

    LAString& assign( const char_t* inputString );

    //  ------------------------------------------------------------------------------------

    // String Operators
    LAString&           operator = (const std::string & inputString);
    LAString&           operator = (const LAString & inputString);
    LAString&           operator = (const char_t* inputString);
    LAString&           operator = (const char_t inputChar);
    
    LAString            operator + (const std::string & inputString) const;
    LAString            operator + (const LAString & inputString) const;
    LAString            operator + (const char_t* inputString) const;
    LAString            operator + (const char_t  inputChar) const;
    
    LAString&           operator += (const std::string & inputString);
    LAString&           operator += (const LAString & inputString);
    LAString&           operator += (const char_t* inputString);
    LAString&           operator += (const char_t c);
    
    bool                operator == (const std::string & inputString) const     { return  (cmp(inputString) == 0);  }
    bool                operator == (const LAString & inputString) const        { return  (cmp(inputString) == 0);  }
    bool                operator == (const char_t* inputString) const           { return  (cmp(inputString) == 0);  }

    bool                operator != (const std::string & inputString) const     { return !(cmp(inputString) == 0);  }
    bool                operator != (const LAString & inputString) const        { return !(cmp(inputString) == 0);  }
    bool                operator != (const char_t* inputString) const           { return !(cmp(inputString) == 0);  }

    bool                operator <= (const std::string & inputString) const     { return  (cmp(inputString) <= 0);  }
    bool                operator <= (const LAString & inputString) const        { return  (cmp(inputString) <= 0);  }
    bool                operator <= (const char_t* inputString) const           { return  (cmp(inputString) <= 0);  }

    bool                operator >= (const std::string & inputString) const     { return  (cmp(inputString) >= 0);  }
    bool                operator >= (const LAString & inputString) const        { return  (cmp(inputString) >= 0);  }
    bool                operator >= (const char_t* inputString) const           { return  (cmp(inputString) >= 0);  }

    bool                operator < (const std::string & inputString) const      { return  (cmp(inputString) < 0);   }
    bool                operator < (const LAString & inputString) const         { return  (cmp(inputString) < 0);   }
    bool                operator < (const char_t* inputString) const            { return  (cmp(inputString) < 0);   }

    bool                operator > (const std::string & inputString) const      { return  (cmp(inputString) > 0);   }
    bool                operator > (const LAString & inputString) const         { return  (cmp(inputString) > 0);   }
    bool                operator > (const char_t* inputString) const            { return  (cmp(inputString) > 0);   }
    
    // Friend Operators
    friend LAString     operator + (const char_t* charString1, const LAString & inputString);

    friend bool         operator == (const char_t* charString, const LAString & inputString);
    friend bool         operator != (const char_t* charString, const LAString & inputString);
    friend bool         operator <= (const char_t* charString, const LAString & inputString);
    friend bool         operator >= (const char_t* charString, const LAString & inputString);
    friend bool         operator < (const char_t* charString, const LAString & inputString);
    friend bool         operator > (const char_t* charString, const LAString & inputString);

    friend OutStream&   operator<< ( OutStream& os, const LAString& st );
    friend InStream&    operator>> ( InStream& is, LAString& st );

public:

    // String Comparison: LAString
    int cmp(const LAString& rString) const;

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
    void                copy(const LAString& rString);
    
    // Deep Copy
    void                copy(const char_t* pString);

    // reset the reference count of the string pointer that this object holds
    void                makeUnShared(void);


    //
    // MEMBER VARIABLES
    //
    // ------------------------------------------------------------------------------------------------------------------------
    
    StringData*                     stringData_;
    mutable std::atomic<int>*       refCount_;

    // ------------------------------------------------------------------------------------------------------------------------


    // Internal Class
    // class to hold string data, for memory management and basic string operations.
    class StringData
    {
    public:
    
        StringData(const unsigned int allocSize=1);
        explicit StringData(const char_t* pString);
        ~StringData() {delete [] string_;}

        unsigned int size() const {return stringSize_;}
        const char_t* getCString() const {return string_;}
        char_t& getChar(const unsigned int pos) const {return string_[pos];}

        void toUpper(void);
        void toLower(void);

        void exchange(const char_t from, const char_t to);
        void exchange(const char_t* pFromString, const char_t* pToString);
        void replace(const unsigned int from, const char_t* pString);
        void insert(const unsigned int from, const char_t* pString);
        void remove(const unsigned int from, const unsigned int num);

        // Comparator
        inline int cmp(const char_t* pString) const
        {
            const char_t* fromStr = string_;
            const char_t* toStr = pString;

            while(*fromStr != '\0' && *toStr != '\0' && *fromStr == *toStr)
            {
                ++toStr;
                ++fromStr;
            }
            return *fromStr - *toStr;
        }
       
    private:

        // Extend memory from size "fromSize" to size "toSize". Note: stringSize_ += to
        void extend(const unsigned int fromSize, const unsigned int toSize);

        unsigned int           allocationSize_;     // allocated memory size
        unsigned int           stringSize_;         // string size
        char_t*                string_;             // pointer to string
    };                  
};


bool operator == (const char_t* s, const LAString & inputString);
bool operator != (const char_t* s, const LAString & inputString);
bool operator <= (const char_t* s, const LAString & inputString);
bool operator >= (const char_t* s, const LAString & inputString);
bool operator < (const char_t* s, const LAString & inputString);
bool operator > (const char_t* s, const LAString & inputString);
std::ostream& operator<< ( std::ostream& os, const LAString& st );

LAString  operator + (const char_t* aString1, const LAString & inputString);
