/*! @file
    @brief Class declaration to define basic data classes whose types are vector.

    Define following base Data classes. (AQLDataBools, AQLDataInts, AQLDataDoubles, AQLDataStrings,
    AQLDataDates, AQLDataDateTimes)
*/


#ifndef AQLDataVector_h
#define AQLDataVector_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceDataType.h"
#include "AQLDataBasics.h"
#include "AQLString.h"
#include "AQLDate.h"
#include "AQLDateTime.h"
#include "AQLCoreTemplateType.h"
#include <vector>

#define DATA_COLL_DEL ':'     // use as the delimiter string representation

#define DATA_BOOLS        7   // AQLDataBools Data ID

#define DATA_STRINGS      8   // AQLDataStrings Data ID

#define DATA_DATETIMES    9   // AQLDataDateTimes Data ID

#define DATA_DATES        10  // AQLDataDates Data ID

#define DATA_INTS         11  // AQLDataInts Data ID

#define DATA_DOUBLES      12  // AQLDataDoubles Data ID

//============ AQLDataBools =============================
/*! 
    @brief Boolian Data class to represent the data (type bool).
*/
class AQLDataBools : public AQLPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    AQLDataBools(void);
    // copy constructor
    AQLDataBools(const AQLDataBools& attr);    
    // constructor
    explicit AQLDataBools(const BoolVector& b);
    // destructor
    virtual ~AQLDataBools(void);

//  QUERY
    // deep copy of this object
    virtual AQLPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual AQLString        convertToString(void) const;

    // return vector data this class has
    /*!
        @return vector data
    */
    const BoolVector&       get(void) const {return mData;}
    
    // return the data size this class has
    /*!
        @return data size
    */
    unsigned int            getSize(void) const {return mData.size();}

//  OPERATION
    // set data data from a specific string format
    virtual void            convertFromString(const AQLString& str);
    // set data
    void                    set(bool b, int i);
    // set data
    void                    set(const BoolVector& b);

// Override of method of vector
    // add data to the end
    void                    push_back(bool b);
    // insert the data
//    void                    insert(BoolVector::const_iterator it, bool b);
    void                    insert(BoolVector::iterator it, bool b);

    // an iterator pointing to the first element
    BoolVector::const_iterator begin() const;
    // an iterator pointing to the element at the end
    BoolVector::const_iterator end() const;
    // delete the data for the specified element
//    void                    erase(BoolVector::const_iterator it);
    void                    erase(BoolVector::iterator it);
    // clear all data
    void                    clear();
    // array subscript operator
    bool                    operator[](int i) const;

//  OPERATOR
    // assignment operator
    AQLDataBools&            operator=(const BoolVector& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual AQLPriceDataType&    assignment(const AQLPriceDataType& a);

    // compare the contents againt the other object
    virtual int             compare(const AQLPriceDataType& a) const;
private:
    BoolVector  mData;  // bool vector data
};

//============ AQLDataStrings    =============================
/*! 
    @brief AQLString Data class to represent the data (type string).
*/
class AQLDataStrings : public AQLPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    AQLDataStrings(void);
    // copy constructor
    AQLDataStrings(const AQLDataStrings& attr);
    // constructor
    explicit AQLDataStrings(const AQLStringVector& b);
    // destructor
    virtual ~AQLDataStrings(void);

//  QUERY
    // deep copy of this object
    virtual AQLPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual AQLString        convertToString(void) const;

    // return string vector data this class has
    /*!
        @return string vector data
    */
    const AQLStringVector&     get(void) const {return mData;}

    // return the data size this class has
    /*!
        @return data size
    */
    unsigned int            getSize(void) const {return mData.size();}

//  OPERATION
    // set data data from a specific string format
    virtual void            convertFromString(const AQLString& str);

    // set data
    void                    set(const AQLString& b, int i);
    // set data
    void                    set(const AQLStringVector& d);

// Override of method of vector
    // add data to the end
    void                    push_back(const AQLString& b);
    // an iterator pointing to the first element
    AQLStringVector::const_iterator begin() const;
    // an iterator pointing to the element at the end
    AQLStringVector::const_iterator end() const;
    // insert the data
//    void                    insert(AQLStringVector::const_iterator it, const AQLString& b);
    void                    insert(AQLStringVector::iterator it, const AQLString& b);
    // delete the data for the specified element
//    void                    erase(AQLStringVector::const_iterator it);
    void                    erase(AQLStringVector::iterator it);
    // clear all data
    void                    clear();
    // array subscript operator
    const AQLString&         operator[](int i) const;
//  OPERATOR
    // assignment operator
    AQLDataStrings&          operator=(const AQLStringVector& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual AQLPriceDataType&    assignment(const AQLPriceDataType& a);
    // compare the contents against the other objecet
    virtual int             compare(const AQLPriceDataType& a) const;
private:
    AQLStringVector    mData;    // AQLString vector data
};

//============ AQLDataDateTimes    =============================
/*! 
    @brief AQLDateTime Data class to represent the data (type AQLDateTime).
*/
class AQLDataDateTimes : public AQLPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    AQLDataDateTimes(void);
    // copy constructor
    AQLDataDateTimes(const AQLDataDateTimes& attr);    
    // constructor
    explicit AQLDataDateTimes(const DateTimeVector& b);
    // destructor
    virtual ~AQLDataDateTimes(void);

//  QUERY
    // deep copy of this object
    virtual AQLPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual AQLString        convertToString(void) const;

    // return datetime vector data this class has
    /*!
        @return datetime vector
    */
    const DateTimeVector&   get(void) const {return mData;}

    // return the data size this class has
    /*!
        @return data size
    */
    unsigned int            getSize(void) const {return mData.size();}

//  OPERATION
    // set data data from a specific string format
    virtual void            convertFromString(const AQLString& str);

    // set data
    void                    set(const AQLDateTime& b, int i);
    // set data
    void                    set(const DateTimeVector& d);

// Override of method of vector
    // add data to the end
    void                    push_back(const AQLDateTime& b);
    // an iterator pointing to the first element
    DateTimeVector::const_iterator
                            begin() const;
    // an iterator pointing to the element at the end
    DateTimeVector::const_iterator
                            end() const;
    // insert the data
//    void                    insert(DateTimeVector::const_iterator it, const AQLDateTime& b);
    void                    insert(DateTimeVector::iterator it, const AQLDateTime& b);
    // delete the data for the specified element
//    void                    erase(DateTimeVector::const_iterator it);
    void                    erase(DateTimeVector::iterator it);
    // clear all data
    void                    clear();
    // array subscript operator
    const AQLDateTime&           operator[](int i) const;

//  OPERATOR
    // assignment operator
    AQLDataDateTimes&        operator=(const DateTimeVector& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual AQLPriceDataType&    assignment(const AQLPriceDataType& a);
    // compare the contents against the other objecet
    virtual int             compare(const AQLPriceDataType& a) const;
private:
    DateTimeVector      mData;  // AQLDateTime vector data
};

//============ AQLDataDates    =============================
/*! 
    @brief AQLDate Data class to represent the data (type AQLDate).
*/
class AQLDataDates : public AQLPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    AQLDataDates(void);
    // copy constructor
    AQLDataDates(const AQLDataDates& attr); 
    // constructor
    explicit AQLDataDates(const DateVector& b);
    // destructor
    virtual ~AQLDataDates(void);

//  QUERY
    // deep copy of this object
    virtual AQLPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual AQLString        convertToString(void) const;

    // return date vector data this class has
    /*!
        @return date vector
    */
    const DateVector&       get(void) const {return mData;}

    // return the data size this class has
    /*!
        @return data size
    */
    unsigned int            getSize(void) const {return mData.size();}

//  OPERATION
    // set data data from a specific string format
    virtual void            convertFromString(const AQLString& str);

    // set data
    void                    set(const AQLDate& b, int i);
    // set data
    void                    set(const DateVector& d);

// Override of method of vector
    // add data to the end
    void                    push_back(const AQLDate& b);
    // an iterator pointing to the first element
    DateVector::const_iterator
                            begin() const;
    // an iterator pointing to the element at the end
    DateVector::const_iterator
                            end() const;
    // insert the data
//    void                    insert(DateVector::const_iterator it, const AQLDate& b);
    void                    insert(DateVector::iterator it, const AQLDate& b);
    // delete the data for the specified element
//    void                    erase(DateVector::const_iterator it);
    void                    erase(DateVector::iterator it);
    // clear all data
    void                    clear();
    // array subscript operator
    const AQLDate&           operator[](int i) const;
//  OPERATOR
    // assignment operator
    AQLDataDates&            operator=(const DateVector& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual AQLPriceDataType&    assignment(const AQLPriceDataType& a);

    // compare the contents against the other objecet
    virtual int             compare(const AQLPriceDataType& a) const;
private:
    DateVector      mData;  // AQLDate vector data
};

//============ AQLDataInts    =============================
/*! 
    @brief Integer Data class to represent the data (type integer).
*/
class AQLDataInts : public AQLPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    AQLDataInts(void);
    // copy constructor
    AQLDataInts(const AQLDataInts& attr);    
    // constructor
    explicit AQLDataInts(const IntArray& b);
	// destructor
    virtual ~AQLDataInts(void);

//  QUERY
    // deep copy of this object
    virtual AQLPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual AQLString        convertToString(void) const;

    // return integer vector data this class has
    /*!
        @return integer vector data
    */
    const IntArray&        get(void) const {return mData;}

    // return the data size this class has
    /*!
        @return data size
    */
    unsigned int              getSize(void) const {return mData.size();}

//  OPERATION
    // set data data from a specific string format
    virtual void            convertFromString(const AQLString& str);

    // set data
    void                    set(int b, int index);
    // set data
    void                    set(const IntArray& d);

    // resize data
    void					resize(unsigned int num, const int i=0);	
	// clear all data
    void                    clear();
    // array subscript operator
    int                     operator[] (int i) const;
//  OPERATOR
    // assignment operator
    AQLDataInts&             operator=(const IntArray& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual AQLPriceDataType&    assignment(const AQLPriceDataType& a);
    // compare the contents against the other objecet
    virtual int             compare(const AQLPriceDataType& a) const;
private:
    IntArray       mData;  // int vector data
};

//============ AQLDataDoubles    =============================
/*! 
    @brief Double Data class to represent the data (type double).
*/
class AQLDataDoubles : public AQLPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    AQLDataDoubles(int pre=DOUBLE_PRECISION);
    // copy constructor
    AQLDataDoubles(const AQLDataDoubles& attr);    
    // constructor
    explicit AQLDataDoubles(const DoubleArray& b, 
                                            int pre=DOUBLE_PRECISION);
    // destructor
    virtual ~AQLDataDoubles(void);

//  QUERY
    // deep copy of this object
    virtual AQLPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual AQLString        convertToString(void) const;

    // return double vector data this class has
    /*!
        @return double vector
    */
    const DoubleArray&      get(void) const {return mData;}

    // return the data size this class has
    /*!
        @return data size
    */
    unsigned int               getSize(void) const {return mData.size();}

//  OPERATION
    // set data data from a specific string format
    virtual void            convertFromString(const AQLString& str);

    // set data
    void                    set(double b, int index);
    // set data
    void                    set(const DoubleArray& d);

    // set the number of decimal places
    void                    setPrecision(int pre) const {mPre = pre;}

    // resize data
    void					resize(unsigned int num, const double d=0.0);	
	// clear all data
    void                    clear();
    // array subscript operator
    double                  operator[] (int i) const;
//  OPERATOR
    // assignment operator
    AQLDataDoubles&          operator=(const DoubleArray& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual AQLPriceDataType&    assignment(const AQLPriceDataType& a);
    // compare the contents against the other objecet
    virtual int             compare(const AQLPriceDataType& a) const;
private:
    DoubleArray mData;  // double vector data
    mutable int     mPre;  // number of decimal places when converting from a string
};

#endif
