/*! @file
    @brief Class declaration of data which represents Matrix type.
*/



#ifndef AQLDataMatrix_h
#define AQLDataMatrix_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceDataType.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLCoreTemplateType.h"  // DoubleMatrix

#include "AQLCoreAppError.h"
#include "AQLCoreSystemError.h"

#define DATA_DOUBLE_MATRIX      26  // AQLDataDoubleMatrix Data ID
#define DATA_STRING_MATRIX      27  // AQLDataStringMatrix Data ID
#define DATA_BOOL_MATRIX        28  // AQLDataBoolMatrix Data ID


/*! 
    @brief Double matrix Data class to represent the data (type double matrix).
*/
class AQLDataDoubleMatrix : public AQLPriceDataType
{
public:
    // default constructor
    AQLDataDoubleMatrix(int pre=DOUBLE_PRECISION);
    // copy constructor
    AQLDataDoubleMatrix(const AQLDataDoubleMatrix& attr);    
    // constructor
    explicit AQLDataDoubleMatrix(const DoubleMatrix& matrix, int pre=DOUBLE_PRECISION);
    // destructor
    virtual ~AQLDataDoubleMatrix(void);

	// check Null
  	virtual bool			isNull(void) const;

	// deep copy of this object
    virtual AQLPriceDataType*    clone() const;

    // convert matrix type data of the data into string format
    virtual AQLString        convertToString(void) const;

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

    // set the Data data from the specified string
    virtual void            convertFromString(const AQLString& str);

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



    // assignment operator
    AQLDataDoubleMatrix&          operator=(const DoubleMatrix& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual AQLPriceDataType&    assignment(const AQLPriceDataType& a);
    // compare the contents against the other objecet
    virtual int             compare(const AQLPriceDataType& a) const;

private:
    DoubleMatrix mData; // valarray data with double vector type
    mutable int  mPre;  // number of decimal places when converting from a string
};

/*! 
    @brief String matrix Data class to represent the data (type string matrix).
*/
class AQLDataStringMatrix : public AQLPriceDataType
{
public:
    // default constructor
    AQLDataStringMatrix(void);
    // copy constructor
    AQLDataStringMatrix(const AQLDataStringMatrix& attr);    
    // constructor
    explicit AQLDataStringMatrix(const AQLStringMatrix& matrix);
    // destructor
    virtual ~AQLDataStringMatrix(void);

	virtual bool			isNull(void) const;

	virtual AQLPriceDataType*    clone() const;

    virtual AQLString        convertToString(void) const;

    const AQLStringMatrix&		get(void) const {return mData;}
	
	AQLStringMatrix&			get(void);
    
    const AQLStringVector&    get(const unsigned int &i);

    AQLString get(const unsigned int &i, const unsigned int &j) const;

    unsigned int            getSize(void) const;

    unsigned int            get1DSize(void) const { return mData.size(); }

    unsigned int            getSize(const unsigned int &i) const;

    virtual void            convertFromString(const AQLString& str);

    void                    set(const unsigned int &i, const unsigned int &j, const AQLString& value);
    
    void                    set(const unsigned int &i, const AQLStringVector& d);
    
    void                    set(const AQLStringMatrix& d);

    void                    clear();



    AQLDataStringMatrix&          operator=(const AQLStringMatrix& b);

protected:
    virtual AQLPriceDataType&    assignment(const AQLPriceDataType& a);
    
    virtual int             compare(const AQLPriceDataType& a) const;

private:
    AQLStringMatrix mData;
    
};


//============ AQLDataBoolMatrix =============================
/*! 
    @brief Boolian Data class to represent the data (type bool).
*/
class AQLDataBoolMatrix : public AQLPriceDataType
{
public:
    // default constructor
    AQLDataBoolMatrix(void);
    // copy constructor
    AQLDataBoolMatrix(const AQLDataBoolMatrix& attr);    
    // constructor
    explicit AQLDataBoolMatrix(const BoolMatrix& matrix);
    // destructor
    virtual ~AQLDataBoolMatrix(void);

	virtual bool			isNull(void) const;

    // deep copy of this object
    virtual AQLPriceDataType*    clone() const;

    // converted to a string separated by delimiter Data array
    virtual AQLString        convertToString(void) const;

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

    // set data data from a specific string format
    virtual void            convertFromString(const AQLString& str);
    // set data
    void                    set(const unsigned int& i, const unsigned int& j, bool b);
	// set data
    void                    set(const unsigned int &i, const BoolVector& boolValarray);
    // set data
    void                    set(const BoolMatrix& b);
    // clear all data
    void                    clear();

    // assignment operator
    AQLDataBoolMatrix&            operator=(const BoolMatrix& b);

protected:
    // the contents of the object to be initialized with those of another object
    virtual AQLPriceDataType&    assignment(const AQLPriceDataType& a);

    // compare the contents againt the other object
    virtual int             compare(const AQLPriceDataType& a) const;
private:
    BoolMatrix  mData;  // bool matrix data
};
#endif
