/*! @file
    @brief Class declaration to define basic data classes whose types are vector.

    Define following base Data classes. (LADataBools, LADataInts, LADataDoubles, LADataStrings,
    LADataDates, LADataDateTimes)

*/


#ifndef LADataVector_h
#define LADataVector_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceDataType.h"
#include "LADataBasics.h"
#include "LAString.h"
#include "LADate.h"
#include "LADateTime.h"
#include "LACoreTemplateType.h"
#include <vector>

#define DATA_COLL_DEL ':'     // use as the delimiter string representation

#define DATA_BOOLS        7   // LADataBools Data ID

#define DATA_STRINGS      8   // LADataStrings Data ID

#define DATA_DATETIMES    9   // LADataDateTimes Data ID

#define DATA_DATES        10  // LADataDates Data ID

#define DATA_INTS         11  // LADataInts Data ID

#define DATA_DOUBLES      12  // LADataDoubles Data ID

//============ LADataBools =============================
/*! 
    @brief Boolian Data class to represent the data (type bool).
*/
class LADataBools : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataBools(void);
    // copy constructor
    LADataBools(const LADataBools& attr);    
    // constructor
    explicit LADataBools(const BoolVector& b);
    // destructor
    virtual ~LADataBools(void);

//  QUERY
    // deep copy of this object
    virtual LAPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual LAString        convertToString(void) const;

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
    virtual void            convertFromString(const LAString& str);
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
    LADataBools&            operator=(const BoolVector& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual LAPriceDataType&    assignment(const LAPriceDataType& a);

    // compare the contents againt the other object
    virtual int             compare(const LAPriceDataType& a) const;
private:
    BoolVector  mData;  // bool vector data
};

//============ LADataStrings    =============================
/*! 
    @brief LAString Data class to represent the data (type string).
*/
class LADataStrings : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataStrings(void);
    // copy constructor
    LADataStrings(const LADataStrings& attr);
    // constructor
    explicit LADataStrings(const LAStringVector& b);
    // destructor
    virtual ~LADataStrings(void);

//  QUERY
    // deep copy of this object
    virtual LAPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual LAString        convertToString(void) const;

    // return string vector data this class has
    /*!
        @return string vector data
    */
    const LAStringVector&     get(void) const {return mData;}

    // return the data size this class has
    /*!
        @return data size
    */
    unsigned int            getSize(void) const {return mData.size();}

//  OPERATION
    // set data data from a specific string format
    virtual void            convertFromString(const LAString& str);

    // set data
    void                    set(const LAString& b, int i);
    // set data
    void                    set(const LAStringVector& d);

// Override of method of vector
    // add data to the end
    void                    push_back(const LAString& b);
    // an iterator pointing to the first element
    LAStringVector::const_iterator begin() const;
    // an iterator pointing to the element at the end
    LAStringVector::const_iterator end() const;
    // insert the data
//    void                    insert(LAStringVector::const_iterator it, const LAString& b);
    void                    insert(LAStringVector::iterator it, const LAString& b);
    // delete the data for the specified element
//    void                    erase(LAStringVector::const_iterator it);
    void                    erase(LAStringVector::iterator it);
    // clear all data
    void                    clear();
    // array subscript operator
    const LAString&         operator[](int i) const;
//  OPERATOR
    // assignment operator
    LADataStrings&          operator=(const LAStringVector& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual LAPriceDataType&    assignment(const LAPriceDataType& a);
    // compare the contents against the other objecet
    virtual int             compare(const LAPriceDataType& a) const;
private:
    LAStringVector    mData;    // LAString vector data
};

//============ LADataDateTimes    =============================
/*! 
    @brief LADateTime Data class to represent the data (type LADateTime).
*/
class LADataDateTimes : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataDateTimes(void);
    // copy constructor
    LADataDateTimes(const LADataDateTimes& attr);    
    // constructor
    explicit LADataDateTimes(const DateTimeVector& b);
    // destructor
    virtual ~LADataDateTimes(void);

//  QUERY
    // deep copy of this object
    virtual LAPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual LAString        convertToString(void) const;

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
    virtual void            convertFromString(const LAString& str);

    // set data
    void                    set(const LADateTime& b, int i);
    // set data
    void                    set(const DateTimeVector& d);

// Override of method of vector
    // add data to the end
    void                    push_back(const LADateTime& b);
    // an iterator pointing to the first element
    DateTimeVector::const_iterator
                            begin() const;
    // an iterator pointing to the element at the end
    DateTimeVector::const_iterator
                            end() const;
    // insert the data
//    void                    insert(DateTimeVector::const_iterator it, const LADateTime& b);
    void                    insert(DateTimeVector::iterator it, const LADateTime& b);
    // delete the data for the specified element
//    void                    erase(DateTimeVector::const_iterator it);
    void                    erase(DateTimeVector::iterator it);
    // clear all data
    void                    clear();
    // array subscript operator
    const LADateTime&           operator[](int i) const;

//  OPERATOR
    // assignment operator
    LADataDateTimes&        operator=(const DateTimeVector& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual LAPriceDataType&    assignment(const LAPriceDataType& a);
    // compare the contents against the other objecet
    virtual int             compare(const LAPriceDataType& a) const;
private:
    DateTimeVector      mData;  // LADateTime vector data
};

//============ LADataDates    =============================
/*! 
    @brief LADate Data class to represent the data (type LADate).
*/
class LADataDates : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataDates(void);
    // copy constructor
    LADataDates(const LADataDates& attr); 
    // constructor
    explicit LADataDates(const DateVector& b);
    // destructor
    virtual ~LADataDates(void);

//  QUERY
    // deep copy of this object
    virtual LAPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual LAString        convertToString(void) const;

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
    virtual void            convertFromString(const LAString& str);

    // set data
    void                    set(const LADate& b, int i);
    // set data
    void                    set(const DateVector& d);

// Override of method of vector
    // add data to the end
    void                    push_back(const LADate& b);
    // an iterator pointing to the first element
    DateVector::const_iterator
                            begin() const;
    // an iterator pointing to the element at the end
    DateVector::const_iterator
                            end() const;
    // insert the data
//    void                    insert(DateVector::const_iterator it, const LADate& b);
    void                    insert(DateVector::iterator it, const LADate& b);
    // delete the data for the specified element
//    void                    erase(DateVector::const_iterator it);
    void                    erase(DateVector::iterator it);
    // clear all data
    void                    clear();
    // array subscript operator
    const LADate&           operator[](int i) const;
//  OPERATOR
    // assignment operator
    LADataDates&            operator=(const DateVector& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual LAPriceDataType&    assignment(const LAPriceDataType& a);

    // compare the contents against the other objecet
    virtual int             compare(const LAPriceDataType& a) const;
private:
    DateVector      mData;  // LADate vector data
};

//============ LADataInts    =============================
/*! 
    @brief Integer Data class to represent the data (type integer).
*/
class LADataInts : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataInts(void);
    // copy constructor
    LADataInts(const LADataInts& attr);    
    // constructor
    explicit LADataInts(const IntArray& b);
	// destructor
    virtual ~LADataInts(void);

//  QUERY
    // deep copy of this object
    virtual LAPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual LAString        convertToString(void) const;

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
    virtual void            convertFromString(const LAString& str);

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
    LADataInts&             operator=(const IntArray& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual LAPriceDataType&    assignment(const LAPriceDataType& a);
    // compare the contents against the other objecet
    virtual int             compare(const LAPriceDataType& a) const;
private:
    IntArray       mData;  // int vector data
};

//============ LADataDoubles    =============================
/*! 
    @brief Double Data class to represent the data (type double).
*/
class LADataDoubles : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataDoubles(int pre=DOUBLE_PRECISION);
    // copy constructor
    LADataDoubles(const LADataDoubles& attr);    
    // constructor
    explicit LADataDoubles(const DoubleArray& b, 
                                            int pre=DOUBLE_PRECISION);
    // destructor
    virtual ~LADataDoubles(void);

//  QUERY
    // deep copy of this object
    virtual LAPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual LAString        convertToString(void) const;

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
    virtual void            convertFromString(const LAString& str);

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
    LADataDoubles&          operator=(const DoubleArray& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual LAPriceDataType&    assignment(const LAPriceDataType& a);
    // compare the contents against the other objecet
    virtual int             compare(const LAPriceDataType& a) const;
private:
    DoubleArray mData;  // double vector data
    mutable int     mPre;  // number of decimal places when converting from a string
};

#endif
