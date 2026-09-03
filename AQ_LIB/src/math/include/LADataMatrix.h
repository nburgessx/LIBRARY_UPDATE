/*! @file
    @brief Class declaration of data which represents Matrix type.
	
*/



#ifndef LADataMatrix_h
#define LADataMatrix_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceDataType.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LACoreTemplateType.h"  // DoubleMatrix

#include "LACoreAppError.h"
#include "LACoreSystemError.h"

#define DATA_DOUBLE_MATRIX      26  // LADataDoubleMatrix Data ID
#define DATA_STRING_MATRIX      27  // LADataStringMatrix Data ID
#define DATA_BOOL_MATRIX        28  // LADataBoolMatrix Data ID


/*! 
    @brief Double matrix Data class to represent the data (type double matrix).
*/
class LADataDoubleMatrix : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataDoubleMatrix(int pre=DOUBLE_PRECISION);
    // copy constructor
    LADataDoubleMatrix(const LADataDoubleMatrix& attr);    
    // constructor
    explicit LADataDoubleMatrix(const DoubleMatrix& matrix, int pre=DOUBLE_PRECISION);
    // destructor
    virtual ~LADataDoubleMatrix(void);

//  QUERY
	// check Null
  	virtual bool			isNull(void) const;

	// deep copy of this object
    virtual LAPriceDataType*    clone() const;

    // convert matrix type data of the data into string format
    virtual LAString        convertToString(void) const;

    // return double matrix data this class has
    /*!
        @return dobule matrix
    */
    const DoubleMatrix&		get(void) const {return mData;}
	// return double matrix data this class has
	DoubleMatrix&			get(void);
    // return vector data when selected i-th element of an array of one-dimensional
    const DoubleArray&    get(const unsigned int &i);

    // return double val of matrix data[i][j] this class has
    double get(const unsigned int &i, const unsigned int &j) const;

    // return the total number of elements in the two-dimensional data 
    unsigned int            getSize(void) const;

    // return the number of elements in the one-dimensional element of two-dimensional data 
    unsigned int            get1DSize(void) const { return mData.size(); }

    // return size data of an array of i-th element
    unsigned int            getSize(const unsigned int &i) const;

//  OPERATION
    // set the Data data from the specified string
    virtual void            convertFromString(const LAString& str);

    // set the data for the specified element[i][j] 
    void                    set(const unsigned int &i, const unsigned int &j, const double &value);
    // set the data elements specified in the [i]
    void                    set(const unsigned int &i, const DoubleArray& d);
    // set double matrix data
    void                    set(const DoubleMatrix& d);

    // set the number of decimal places
    /*!
        @return number of decimal places
    */
    void                    setPrecision(int pre) const {mPre = pre;}

    // [Warning] we can not use valarray in iterator of STL so that we do not provide methods such as begin() and end()
    // clear all data
    void                    clear();



//  OPERATOR
    // assignment operator
    LADataDoubleMatrix&          operator=(const DoubleMatrix& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual LAPriceDataType&    assignment(const LAPriceDataType& a);
    // compare the contents against the other objecet
    virtual int             compare(const LAPriceDataType& a) const;

private:
    DoubleMatrix mData; // valarray data with double vector type
    mutable int  mPre;  // number of decimal places when converting from a string
};

/*! 
    @brief String matrix Data class to represent the data (type string matrix).
*/
class LADataStringMatrix : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataStringMatrix(void);
    // copy constructor
    LADataStringMatrix(const LADataStringMatrix& attr);    
    // constructor
    explicit LADataStringMatrix(const LAStringMatrix& matrix);
    // destructor
    virtual ~LADataStringMatrix(void);

//  QUERY
	virtual bool			isNull(void) const;

	virtual LAPriceDataType*    clone() const;

    virtual LAString        convertToString(void) const;

    const LAStringMatrix&		get(void) const {return mData;}
	
	LAStringMatrix&			get(void);
    
    const LAStringVector&    get(const unsigned int &i);

    LAString get(const unsigned int &i, const unsigned int &j) const;

    unsigned int            getSize(void) const;

    unsigned int            get1DSize(void) const { return mData.size(); }

    unsigned int            getSize(const unsigned int &i) const;

//  OPERATION
    virtual void            convertFromString(const LAString& str);

    void                    set(const unsigned int &i, const unsigned int &j, const LAString& value);
    
    void                    set(const unsigned int &i, const LAStringVector& d);
    
    void                    set(const LAStringMatrix& d);

    void                    clear();



//  OPERATOR
    LADataStringMatrix&          operator=(const LAStringMatrix& b);

protected:
    virtual LAPriceDataType&    assignment(const LAPriceDataType& a);
    
    virtual int             compare(const LAPriceDataType& a) const;

private:
    LAStringMatrix mData;
    
};


//============ LADataBoolMatrix =============================
/*! 
    @brief Boolian Data class to represent the data (type bool).
*/
class LADataBoolMatrix : public LAPriceDataType
{
public:
//  LIFECYCLE
    // default constructor
    LADataBoolMatrix(void);
    // copy constructor
    LADataBoolMatrix(const LADataBoolMatrix& attr);    
    // constructor
    explicit LADataBoolMatrix(const BoolMatrix& matrix);
    // destructor
    virtual ~LADataBoolMatrix(void);

//  QUERY
	virtual bool			isNull(void) const;

    // deep copy of this object
    virtual LAPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual LAString        convertToString(void) const;

    // return matrix data this class has
    /*!
        @return matrix data
    */
    const BoolMatrix&       get(void) const {return mData;}

    BoolMatrix&				get(void);

	const BoolVector&    get(const unsigned int &i);

    bool get(const unsigned int &i, const unsigned int &j) const;
    
    // return the data size this class has
    /*!
        @return data size
    */
    unsigned int            getSize(void) const;

	unsigned int            get1DSize(void) const { return mData.size(); }

    unsigned int            getSize(const unsigned int &i) const;

//  OPERATION
    // set data data from a specific string format
    virtual void            convertFromString(const LAString& str);
    // set data
    void                    set(const unsigned int& i, const unsigned int& j, bool b);
	// set data
    void                    set(const unsigned int &i, const BoolVector& boolValarray);
    // set data
    void                    set(const BoolMatrix& b);
    // clear all data
    void                    clear();

//  OPERATOR
    // assignment operator
    LADataBoolMatrix&            operator=(const BoolMatrix& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual LAPriceDataType&    assignment(const LAPriceDataType& a);

    // compare the contents againt the other object
    virtual int             compare(const LAPriceDataType& a) const;
private:
    BoolMatrix  mData;  // bool matrix data
};
#endif
