#pragma once

#ifdef __GNUG__
#pragma interface
#endif
#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif

#include "LAPriceDataType.h"
#include "LACoreAppError.h"
#include "LACoreSystemError.h"
#include "LAString.h"
#include "LADate.h"
#include "LADateTime.h"

#define DATA_BOOL        1  // LADataBool Data ID

#define DATA_STRING      2  // LADataString Data ID

#define DATA_DATETIME    3  // LADataDateTime Data ID

#define DATA_DATE        4  // LADataDate Data ID

#define DATA_INT         5  // LADataInt Data ID

#define DATA_DOUBLE      6  // LADataDouble Data ID

// Set number of decimal places of convertToString() for LADataDouble, LADataiDoubles, LADataDoubleMatrix
const int DOUBLE_PRECISION = 18;


//============ LADataBool ==============================================
/*! 
    @brief Boolean Data class to represent the data (type bool).
*/
class LADataBool : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataBool(void);
    // copy constructor
	LADataBool(const LADataBool& attr);
    // constructor
    explicit LADataBool(bool b);
    // destructor
    virtual ~LADataBool(void);

//  QUERY
    // deep copy of this object
    virtual LAPriceDataType*	clone() const;
    // convert boolian type data of the data into string format
	virtual LAString		convertToString(void) const;

	// return bool data this class has
    /*!
        @return true or false
    */
	bool					get() const {return mData;}

    // set boolian data from a specific string format
    virtual void			convertFromString(const LAString& str);
    // set boolian data
	void					set(bool b);

//  OPERATOR
	// static cast into boolian type
    /*!
        @return true or false
    */
							operator bool() const { return mData;}
    // assignment operator
	LADataBool&				operator=(bool b);

protected:
    // the contents of the object to be initialized with those of another object
	virtual LAPriceDataType&	assignment(const LAPriceDataType& a);

    // compare the contents against the other objecet
	virtual int				compare(const LAPriceDataType& a) const;

private:
    // boolian data
	bool mData;
};

//============ LADataString  ==============================================
/*! 
    @brief LAString Data class to represent the data (type string).
*/
class LADataString : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataString(void);
    // copy constructor
    LADataString(const LADataString& attr);
    // constructor
	explicit LADataString(const char_t*   str);
    // constructor
    explicit LADataString(const LAString& str);
    // destructor
    virtual ~LADataString(void);

//  QUERY
    // deep copy of this object
    virtual LAPriceDataType*	clone() const;

    // convert string type data of the data into string format
    virtual LAString		convertToString(void) const;

	// return string data this class has
    /*!
        @return LAString object
    */
	const LAString&			get() const {return mData;}

    // set the specified string to the object
    virtual void			convertFromString(const LAString& str);

    // set the specified string to the object
    /*!
        @param[in] str string to be set(LAString object)
    */
	void					set(const LAString& str) { convertFromString(str); }

//  OPERATOR
    // static cast into LAString
    /*!
        @return LAString object
    */
							operator const LAString&() const {return mData;}
    // assignment operator
	LADataString&			operator=(const LAString& str);

protected:
    // the contents of the object to be initialized with those of another object
	virtual LAPriceDataType&	assignment(const LAPriceDataType& a);

    // compare the contents against the other objecet
	virtual int				compare(const LAPriceDataType& a) const;

private:
    // string data
	LAString			mData;
};

//============ LADataDateTime  ============================================
/*! 
    @brief LADateTime Data class to represent the data (type LADateTime).
*/
class LADataDateTime : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataDateTime(void);
    // copy constructor
    LADataDateTime(const LADataDateTime& attr);
    // constructor
	explicit LADataDateTime(const char_t*   str);
    // constructor
	explicit LADataDateTime(const LAString& str);
    // constructor
    explicit LADataDateTime(const LADateTime& dt);
    // destructor
    virtual ~LADataDateTime(void);

//  QUERY
    // deep copy of this object
    virtual LAPriceDataType*	clone() const;

    // convert time data of the data into string format
    virtual LAString		convertToString(void) const;

	// return time data this class has
    /*!
        @return LADateTime object
    */
	const LADateTime&		get() const {return mData;}
	
//  OPERATION
    // set time data from a specific string format
    virtual void			convertFromString(const LAString& str);
    // set the time data
	void					set(const LADateTime& dt);

//  OPERATOR
	 // static cast into LADateTime
    /*!
        @return LADateTime object
    */
							operator const LADateTime&() const {return mData;}
    // assignment operator
	LADataDateTime&			operator=(const LADateTime& str);


protected:
    // the contents of the object to be initialized with those of another object
	virtual LAPriceDataType&	assignment(const LAPriceDataType& a);

    // compare the contents against the other objecet
	virtual int				compare(const LAPriceDataType& a) const;

private:
    // datetime data
	LADateTime			mData;
};

//============ LADataDate ================================================
/*! 
    @brief LADate Data class to represent the data (type LADate).
*/
class LADataDate : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataDate(void);
    // copy constructor
    LADataDate(const LADataDate& d);
    // constructor
	explicit LADataDate(const LADate& d);
    // constructor
	explicit LADataDate(const char_t*   d);
    // constructor
    LADataDate(const LAString& d);
    // destructor
    virtual ~LADataDate(void);

//  QUERY
    // deep copy of this object
    virtual LAPriceDataType*	clone() const; 

   // convert date data of the data into string format
    virtual LAString		convertToString(void) const;

    // return date data this class has
    /*!
        @return LADate object
    */
	const LADate&			get() const {return mData;}

//  OPERATION
    // set date data from a specific string format
    virtual void			convertFromString(const LAString& str);
    // set the date data
	void					set(const LADate& dt);

//  OPERATOR
	 // static cast into LADate
    /*!
        @return LADate object
    */
							operator const LADate&() const {return mData;}
    // assignment operator
	LADataDate&				operator=(const LADate& str);

protected:
    // the contents of the object to be initialized with those of another object
	virtual LAPriceDataType&	assignment(const LAPriceDataType& a);

    // compare the contents against the other objecet
	virtual int				compare(const LAPriceDataType& a) const;

private:
    // date data
	LADate				mData;
};

//============ LADataInt  =================================================
/*! 
    @brief Integer Data class to represent the data (type integer).
*/
class LADataInt : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataInt(void);
    // copy constructor
	LADataInt(const LADataInt& attr);
    // constructor
    explicit LADataInt(int b);
    // destructor
    virtual ~LADataInt(void);

//  QUERY
    // deep copy of this object
    virtual LAPriceDataType*	clone() const;

    // convert integer data of the data into string format
    virtual LAString		convertToString(void) const;

    // return integer data this class has
    /*!
        @return integer
    */
	int						get() const {return mData;}

//  OPERATION
    // set integer data from a specific string format
    virtual void			convertFromString(const LAString& str);

    // set integer data
	void					set(int b);

//  OPERATOR
	// static cast into integer type
 							operator int() const { return mData;}
    // assignment operator
	LADataInt&				operator=(int b);

protected:
   // the contents of the object to be initialized with those of another object
	virtual LAPriceDataType&	assignment(const LAPriceDataType& a);
    // compare the contents against the other objecet
	virtual int				compare(const LAPriceDataType& a) const;

private:
    // integer data
    int mData;
};


//============ LADataDouble  ===============================================
/*! 
    @brief Double Data class to represent the data (type double).
*/
class LADataDouble : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataDouble(void);
    // copy constructor
    LADataDouble(const LADataDouble& d);
    // constructor
	LADataDouble(double d, int pre=DOUBLE_PRECISION);
    // destructor
    virtual ~LADataDouble(void);

//  QUERY
    // deep copy of this object
    virtual LAPriceDataType*	clone() const;
    // convert double data of the data into string format
    virtual LAString		convertToString(void) const;
	// return double data the class has
    /*!
        @return double
    */
	double					get() const {return mData;}

//  OPERATION
    // set double data from a specific string format
    virtual void			convertFromString(const LAString& str);

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
	LADataDouble&			operator=(double b);

protected:
    // the contents of the object to be initialized with those of another object
	virtual LAPriceDataType&	assignment(const LAPriceDataType& a);
    // compare the contents against the other objecet
	virtual int				compare(const LAPriceDataType& a) const;

private:
	// number of decimal places when converting from a string
    mutable int			mPrecision;
    // double data
	double				mData;
};

