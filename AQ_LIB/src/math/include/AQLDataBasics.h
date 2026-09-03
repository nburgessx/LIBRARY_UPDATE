#pragma once

#ifdef __GNUG__
#pragma interface
#endif
#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif

#include "AQLPriceDataType.h"
#include "AQLCoreAppError.h"
#include "AQLCoreSystemError.h"
#include "AQLString.h"
#include "AQLDate.h"
#include "AQLDateTime.h"

#define DATA_BOOL        1  // AQLDataBool Data ID

#define DATA_STRING      2  // AQLDataString Data ID

#define DATA_DATETIME    3  // AQLDataDateTime Data ID

#define DATA_DATE        4  // AQLDataDate Data ID

#define DATA_INT         5  // AQLDataInt Data ID

#define DATA_DOUBLE      6  // AQLDataDouble Data ID

// Set number of decimal places of convertToString() for AQLDataDouble, LADataiDoubles, AQLDataDoubleMatrix
const int DOUBLE_PRECISION = 18;


//============ AQLDataBool ==============================================
/*! 
    @brief Boolean Data class to represent the data (type bool).
*/
class AQLDataBool : public AQLPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    AQLDataBool(void);
    // copy constructor
	AQLDataBool(const AQLDataBool& attr);
    // constructor
    explicit AQLDataBool(bool b);
    // destructor
    virtual ~AQLDataBool(void);

//  QUERY
    // deep copy of this object
    virtual AQLPriceDataType*	clone() const;
    // convert boolian type data of the data into string format
	virtual AQLString		convertToString(void) const;

	// return bool data this class has
    /*!
        @return true or false
    */
	bool					get() const {return mData;}

    // set boolian data from a specific string format
    virtual void			convertFromString(const AQLString& str);
    // set boolian data
	void					set(bool b);

//  OPERATOR
	// static cast into boolian type
    /*!
        @return true or false
    */
							operator bool() const { return mData;}
    // assignment operator
	AQLDataBool&				operator=(bool b);

protected:
    // the contents of the object to be initialized with those of another object
	virtual AQLPriceDataType&	assignment(const AQLPriceDataType& a);

    // compare the contents against the other objecet
	virtual int				compare(const AQLPriceDataType& a) const;

private:
    // boolian data
	bool mData;
};

//============ AQLDataString  ==============================================
/*! 
    @brief AQLString Data class to represent the data (type string).
*/
class AQLDataString : public AQLPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    AQLDataString(void);
    // copy constructor
    AQLDataString(const AQLDataString& attr);
    // constructor
	explicit AQLDataString(const char_t*   str);
    // constructor
    explicit AQLDataString(const AQLString& str);
    // destructor
    virtual ~AQLDataString(void);

//  QUERY
    // deep copy of this object
    virtual AQLPriceDataType*	clone() const;

    // convert string type data of the data into string format
    virtual AQLString		convertToString(void) const;

	// return string data this class has
    /*!
        @return AQLString object
    */
	const AQLString&			get() const {return mData;}

    // set the specified string to the object
    virtual void			convertFromString(const AQLString& str);

    // set the specified string to the object
    /*!
        @param[in] str string to be set(AQLString object)
    */
	void					set(const AQLString& str) { convertFromString(str); }

//  OPERATOR
    // static cast into AQLString
    /*!
        @return AQLString object
    */
							operator const AQLString&() const {return mData;}
    // assignment operator
	AQLDataString&			operator=(const AQLString& str);

protected:
    // the contents of the object to be initialized with those of another object
	virtual AQLPriceDataType&	assignment(const AQLPriceDataType& a);

    // compare the contents against the other objecet
	virtual int				compare(const AQLPriceDataType& a) const;

private:
    // string data
	AQLString			mData;
};

//============ AQLDataDateTime  ============================================
/*! 
    @brief AQLDateTime Data class to represent the data (type AQLDateTime).
*/
class AQLDataDateTime : public AQLPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    AQLDataDateTime(void);
    // copy constructor
    AQLDataDateTime(const AQLDataDateTime& attr);
    // constructor
	explicit AQLDataDateTime(const char_t*   str);
    // constructor
	explicit AQLDataDateTime(const AQLString& str);
    // constructor
    explicit AQLDataDateTime(const AQLDateTime& dt);
    // destructor
    virtual ~AQLDataDateTime(void);

//  QUERY
    // deep copy of this object
    virtual AQLPriceDataType*	clone() const;

    // convert time data of the data into string format
    virtual AQLString		convertToString(void) const;

	// return time data this class has
    /*!
        @return AQLDateTime object
    */
	const AQLDateTime&		get() const {return mData;}
	
//  OPERATION
    // set time data from a specific string format
    virtual void			convertFromString(const AQLString& str);
    // set the time data
	void					set(const AQLDateTime& dt);

//  OPERATOR
	 // static cast into AQLDateTime
    /*!
        @return AQLDateTime object
    */
							operator const AQLDateTime&() const {return mData;}
    // assignment operator
	AQLDataDateTime&			operator=(const AQLDateTime& str);


protected:
    // the contents of the object to be initialized with those of another object
	virtual AQLPriceDataType&	assignment(const AQLPriceDataType& a);

    // compare the contents against the other objecet
	virtual int				compare(const AQLPriceDataType& a) const;

private:
    // datetime data
	AQLDateTime			mData;
};

//============ AQLDataDate ================================================
/*! 
    @brief AQLDate Data class to represent the data (type AQLDate).
*/
class AQLDataDate : public AQLPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    AQLDataDate(void);
    // copy constructor
    AQLDataDate(const AQLDataDate& d);
    // constructor
	explicit AQLDataDate(const AQLDate& d);
    // constructor
	explicit AQLDataDate(const char_t*   d);
    // constructor
    AQLDataDate(const AQLString& d);
    // destructor
    virtual ~AQLDataDate(void);

//  QUERY
    // deep copy of this object
    virtual AQLPriceDataType*	clone() const; 

   // convert date data of the data into string format
    virtual AQLString		convertToString(void) const;

    // return date data this class has
    /*!
        @return AQLDate object
    */
	const AQLDate&			get() const {return mData;}

//  OPERATION
    // set date data from a specific string format
    virtual void			convertFromString(const AQLString& str);
    // set the date data
	void					set(const AQLDate& dt);

//  OPERATOR
	 // static cast into AQLDate
    /*!
        @return AQLDate object
    */
							operator const AQLDate&() const {return mData;}
    // assignment operator
	AQLDataDate&				operator=(const AQLDate& str);

protected:
    // the contents of the object to be initialized with those of another object
	virtual AQLPriceDataType&	assignment(const AQLPriceDataType& a);

    // compare the contents against the other objecet
	virtual int				compare(const AQLPriceDataType& a) const;

private:
    // date data
	AQLDate				mData;
};

//============ AQLDataInt  =================================================
/*! 
    @brief Integer Data class to represent the data (type integer).
*/
class AQLDataInt : public AQLPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    AQLDataInt(void);
    // copy constructor
	AQLDataInt(const AQLDataInt& attr);
    // constructor
    explicit AQLDataInt(int b);
    // destructor
    virtual ~AQLDataInt(void);

//  QUERY
    // deep copy of this object
    virtual AQLPriceDataType*	clone() const;

    // convert integer data of the data into string format
    virtual AQLString		convertToString(void) const;

    // return integer data this class has
    /*!
        @return integer
    */
	int						get() const {return mData;}

//  OPERATION
    // set integer data from a specific string format
    virtual void			convertFromString(const AQLString& str);

    // set integer data
	void					set(int b);

//  OPERATOR
	// static cast into integer type
 							operator int() const { return mData;}
    // assignment operator
	AQLDataInt&				operator=(int b);

protected:
   // the contents of the object to be initialized with those of another object
	virtual AQLPriceDataType&	assignment(const AQLPriceDataType& a);
    // compare the contents against the other objecet
	virtual int				compare(const AQLPriceDataType& a) const;

private:
    // integer data
    int mData;
};


//============ AQLDataDouble  ===============================================
/*! 
    @brief Double Data class to represent the data (type double).
*/
class AQLDataDouble : public AQLPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    AQLDataDouble(void);
    // copy constructor
    AQLDataDouble(const AQLDataDouble& d);
    // constructor
	AQLDataDouble(double d, int pre=DOUBLE_PRECISION);
    // destructor
    virtual ~AQLDataDouble(void);

//  QUERY
    // deep copy of this object
    virtual AQLPriceDataType*	clone() const;
    // convert double data of the data into string format
    virtual AQLString		convertToString(void) const;
	// return double data the class has
    /*!
        @return double
    */
	double					get() const {return mData;}

//  OPERATION
    // set double data from a specific string format
    virtual void			convertFromString(const AQLString& str);

    // specify the number of decimal places when converting from a string
    /*!
        @param[in] pre number of decimal places
    */
	void					setPrecision(int pre) const {mPrecision=pre;}

    // set the double data
	void					set(double);

	//static cast into double type
    /*!
        @return double
    */
							operator double() const { return mData;}
    // assignment operator
	AQLDataDouble&			operator=(double b);

protected:
    // the contents of the object to be initialized with those of another object
	virtual AQLPriceDataType&	assignment(const AQLPriceDataType& a);
    // compare the contents against the other objecet
	virtual int				compare(const AQLPriceDataType& a) const;

private:
	// number of decimal places when converting from a string
    mutable int			mPrecision;
    // double data
	double				mData;
};

