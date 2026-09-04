/*! @file
    @brief Implementation of data which represents Matrix type.
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLDataMatrix.h"

using namespace std;

// ================= AQLDataDoubleMatrix ======================
/*!
    @brief constructor
    
    @param[in] pre number of decimal places
*/
AQLDataDoubleMatrix::AQLDataDoubleMatrix(int pre) : AQLPriceDataType(DATA_DOUBLE_MATRIX)
{
    mPre = pre;
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLDataDoubleMatrix::AQLDataDoubleMatrix(const AQLDataDoubleMatrix& attr) 
    : AQLPriceDataType(DATA_DOUBLE_MATRIX), mData(attr.mData), mPre(attr.mPre)
{
	// if at least one data is set( the size of data is not 0) in two-dimensional elements,
	// then it is regarded as not NULL (default set is true)
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        if (mData[i].size() != 0)
		{
            // at least one data is set, then set mIsNull by false and break the loop
            setNull(false);
            break;
        }
    }
}

/*!
    @brief constructor

    @param[in] matrix DoubleMatrix data(double 2-dim valarray)
    @param[in] pre number of decimal places
*/
AQLDataDoubleMatrix::AQLDataDoubleMatrix(const DoubleMatrix& matrix, int pre) 
    : AQLPriceDataType(DATA_DOUBLE_MATRIX), mData(matrix), mPre(pre)
{
    // if at least one data is set( the size of data is not 0) in two-dimensional elements,
	// then it is regarded as not NULL (default set is true)
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        if (mData[i].size() != 0)
		{
            // at least one data is set, then set mIsNull by false and break the loop
            setNull(false);
            break;
        }
    }
}

/*!
    @brief destructor
*/
AQLDataDoubleMatrix::~AQLDataDoubleMatrix(void)
{
}
/*!
    @brief check if NULL

    @return true if NULL, false otherwise
*/
bool
AQLDataDoubleMatrix::isNull(void) const
{
    bool ret = true;
    for(unsigned int j = 0; j < mData.size(); j++)
	{
        if (mData[j].size() != 0)
		{
            // at least one data is set, then set mIsNull by false and break the loop
            ret = false;
			break;
        }
    }
	return ret;
}

/*!
    @brief deep copy of this object

    @note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set
*/
AQLPriceDataType*
AQLDataDoubleMatrix::clone() const 
{
    try {
        AQLDataDoubleMatrix*  pAttr = new AQLDataDoubleMatrix(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief convert matrix type data of the data into string format
    
    convert a string from a two-dimensional element in order, that is
	val[i][j] is converted to string and stored in order by
	val[0][0], val[0][1], val[0][2],..., val[1][0], val[1][1], ...

    @return in case of NULL data, return "NULL", otherwise, return double data separated by delimiter
*/
AQLString
AQLDataDoubleMatrix::convertToString(void) const
{
	// OLD
	// ---

    // AQLString ret;
    // if (isNull() )
    // {
    //     ret = NULL_STR;
    // }
    // else
    // {
    //     ret = "";
    //     int i;
    //     int j;
    //     int size = mData.size();
    //     for (i = 0; i < size - 1; i++)
	// 	{
    //         for (j = 0; j < static_cast<int>(mData[i].size()); j++)
	// 		{
    //             ret += AQLString(mData[i][j], mPre);
	// 			// no delimiter for the last data
    //             if (j != static_cast<int>(mData[i].size() - 1))
	// 				ret += DATA_COLL_DEL;
    //         }
	// 		ret += ';';
    //     }
	// 
    //     // last data of first element(mData[size - 1][])
    //     for (j = 0; j < static_cast<int>(mData[size - 1].size()); j++)
	// 	{
    //         ret += AQLString(mData[size - 1][j], mPre);
    //         // no delimiter for the last data
	// 		if (j != static_cast<int>(mData[size - 1].size() - 1))
	// 			ret += DATA_COLL_DEL;
    //     }
    // }
    // 
    // return ret;


	// NEW
	// ---
	
	AQLString result;
    if (isNull() )
    {
        result = NULL_STR;
    }
    else
    {
        result = "";
        size_t size = mData.size();
        for (size_t i = 0; i < size-1; ++i)
		{
            for (size_t j = 0; j < mData[i].size()-1; ++j)
			{
                result += std::to_string(mData[i][j]) + DATA_COLL_DEL;
            }
			// no delimiter for the last data
			result += std::to_string(mData[i][mData[i].size()-1]) + ';';
        }
	
        for (size_t j = 0; j < mData[size - 1].size()-1; ++j)
		{
            result += std::to_string(mData[size - 1][j]) + DATA_COLL_DEL;
        }
		// no delimiter for the last data
		result += std::to_string(mData[size - 1][mData[size - 1].size()-1]);
    }
    return result;
}

/*!
    @brief return double matrix data this class has

    @return double matrix data
*/
DoubleMatrix&
AQLDataDoubleMatrix::get(void)
{
	update();
	return mData;
}

/*!
    @brief return vector data when selected i-th element of an array of one-dimensional
    
    If the memory of the specified element has not been reserved, then throw the error.

    @param[in] i the element number of 1st dimention

    @return DoubleArray data (reference)
*/
const DoubleArray&
AQLDataDoubleMatrix::get(const unsigned int &i)
{
    char_t str[16]; // 16 digits is enough
    // If the memory of the specified element has not been reserved, then throw the error.
    if (i + 1 > mData.size()){
        AQLString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += AQLString(str) + "]";
        throw AQLCoreSystemError(msg.getCString(), __FILE__, __LINE__);
    }
    return mData[i];
}

/*!
    @brief  return double value of matrix data[i][j]
    
    If the memory of the specified element has not been reserved, then throw the error.

    @param[in] i the element number of 1st dimention
    @param[in] j the element number of 2nd dimention

    @return double
*/
double
AQLDataDoubleMatrix::get(const unsigned int &i, const unsigned int &j) const
{
    char_t str[16]; // 16 digits is enough

    // If the memory of the specified element has not been reserved, then throw the error.
    if (i + 1 > mData.size()){
        AQLString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += AQLString(str) + "]";
        throw AQLCoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }
    if (j + 1 > mData[i].size()){
        AQLString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += AQLString(str) + "][";
        SPRINTF(str, "%d", j);
        msg += AQLString(str) + "]";
        throw AQLCoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }

    return mData[i][j];
}

/*!
    @brief return the total number of elements in the two-dimensional data

    @return the total number of elements
*/
unsigned int 
AQLDataDoubleMatrix::getSize() const
{
    unsigned int count = 0;
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        count += mData[i].size(); // add the number of 2-dim elements
    }
    return count;
}

/*!
    @brief return size data of an array of i-th element

    If the memory of the specified element has not been reserved, then throw the error.

    @param[in] i the element number of 1st dimention(starting at 0)

    @return number of elements
*/
unsigned int
AQLDataDoubleMatrix::getSize(const unsigned int& i) const
{
    char_t str[16]; // 16 digits is enough
    // If the memory of the specified element has not been reserved, then throw the error.
    if (i + 1 > mData.size()){
        AQLString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += AQLString(str) + "]";
        throw AQLCoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }

    return mData[i].size();
}

/*!
    @brief set (valarray two-dimensional data type double) DoubleMatrix data from a string of the specified floating-point format.

  
	@param[in] str double string separated by a delimiter string
*/
void
AQLDataDoubleMatrix::convertFromString(const AQLString& str)
{
    update();

    this->clear();
    if (str == NULL_STR || ! str.isDefined()) 
    {
        setNull();
        return;
    }

    vector<AQLString> tokens = str.toToken( ';' );
    vector<AQLString>::iterator it, it2;
    unsigned int i, j;

	setNull();
	mData.resize(tokens.size());
	for (i = 0, it = tokens.begin(); it != tokens.end(); i++, it++)
	{
		vector<AQLString> tokens2 = it->toToken( DATA_COLL_DEL );
		mData[i].resize(tokens2.size());
		for (j = 0, it2 = tokens2.begin(); it2 != tokens2.end(); j++, it2++)
		{
			setNull(false);
			mData[i][j] = it2->getDoubleValue();
		}
	}
}


/*!
    @brief set the data for the specified element[i][j]

    If the area of the specified element number has not been reserved, set the data so that the area was secured.
	Be careful that area to be secured is only from [i][0] to [i][j].
	If there is no area secured, and if set(3, 4, 5.2); is called, then,
	<br> 1. area from [0] to [3] of 1-dim element is secured
	<br> 2. area from [3][0] to [3][4] is only secured
	Be careful that element such that from [0][0] to [0][4], [1][0] to [1][4] and [2][0] to [2][4] is not secured.

    @param[in] i the element number of 1st dimention(starting at 0)
    @param[in] j the element number of 2nd dimention(starting at 0)
    @param[in] value data to be set
*/
void
AQLDataDoubleMatrix::set(const unsigned int& i, const unsigned int& j, const double& value)
{
	// check whether area of arrya is secured, and if not, secure the area
	if (i + 1 > mData.size()){ // in case of not securing the area about 1-dim element
        mData.resize(i + 1); // secure area
        mData[i].resize(j + 1); // secure 2-dim area(only from [i][0] to [i][j])
    }
    else {  // in case 1-dim area is secured
        if (j + 1 > mData[i].size()){ // in case of not securing the area about 2-dim element
            mData[i].resize(j + 1); // secure area(only from [i][0] to [i][j])
        }
    }
    mData[i][j] = value;
    update();
    setNull(false);
}

/*!
    @brief set the data elements specified in the [i]

    If the area of the specified element number has not been reserved, set the data so that the area was secured.

    @param[in] i the element number of 1st dimention(starting at 0)
    @param[in] dValarray DoubleArray data to be set
*/
void
AQLDataDoubleMatrix::set(const unsigned int &i, const DoubleArray& dValarray)
{
    // check whether area of arrya is secured, and if not, secure the area
    if (i + 1 > mData.size()){ // in case of not securing the area about 1-dim element, then secure the area
        mData.resize(i + 1); // secure area
    }
    size_t valSize = dValarray.size();
    if (mData[i].size() < valSize){ // in case of not securing the area about 2-dim element, then secure the area
        mData[i].resize(valSize);
    }

    // set data
    mData[i] = dValarray;
    update();

    // if at least one data is set( the size of data is not 0) in two-dimensional elements,
	// then it is regarded as not NULL
    setNull();
    for(unsigned int j = 0; j < mData.size(); j++)
	{
        if (mData[j].size() != 0)
		{
            // at least one data is set, then set mIsNull by false and break the loop
            setNull(false);
            break;
        }
    }
}


/*!
    @brief set double matrix data

    @param[in] d double matrix data to be set
*/
void
AQLDataDoubleMatrix::set(const DoubleMatrix& d)
{
    mData = d;
    update();
    // if at least one data is set( the size of data is not 0) in two-dimensional elements,
	// then it is regarded as not NULL
    setNull(); // set mIsNull by true
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        if (mData[i].size() != 0)
		{
            // at least one data is set, then set mIsNull by false and break the loop
            setNull(false);
            break;
        }
    }
}

/*!
    @brief clear all data
*/
void                    
AQLDataDoubleMatrix::clear()
{
    update();
    setNull();
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        mData[i].clear();
    }
	mData.clear();
}

/*
    @brief assignment operator

    @param[in] d right hand side data of assignment operator 
*/
AQLDataDoubleMatrix&
AQLDataDoubleMatrix::operator=(const DoubleMatrix& d)
{
    set(d);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents
*/
AQLPriceDataType&
AQLDataDoubleMatrix::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;
    if (a.getType() != DATA_DOUBLE_MATRIX) 
    {
        // exception
        AQLString err = "Assignment error for AQLDataDoubleMatrix : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const AQLDataDoubleMatrix&>(a).mData);
    mPre = dynamic_cast<const AQLDataDoubleMatrix&>(a).mPre;
    return *this;
}

/*
    @brief compare the contents against the other objecet
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared

	@return  (number of elements of this object) - (number of elements of object a)
*/
int
AQLDataDoubleMatrix::compare(const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_DOUBLE_MATRIX) 
    {
        // exception
        AQLString err = "Compare error for AQLDataDoubleMatrix : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return (int)this->getSize() - 
                (int)dynamic_cast<const AQLDataDoubleMatrix&>(a).getSize();
}

///////////////////////////////////
// ================= AQLDataStringMatrix ======================
AQLDataStringMatrix::AQLDataStringMatrix(void) : AQLPriceDataType(DATA_STRING_MATRIX)
{
}

AQLDataStringMatrix::AQLDataStringMatrix(const AQLDataStringMatrix& attr) 
    : AQLPriceDataType(DATA_STRING_MATRIX), mData(attr.mData)
{
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        if (mData[i].size() != 0)
		{
            setNull(false);
            break;
        }
    }
}

AQLDataStringMatrix::AQLDataStringMatrix(const AQLStringMatrix& matrix) 
    : AQLPriceDataType(DATA_STRING_MATRIX), mData(matrix)
{
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        if (mData[i].size() != 0)
		{
            setNull(false);
            break;
        }
    }
}

AQLDataStringMatrix::~AQLDataStringMatrix(void)
{
}

bool
AQLDataStringMatrix::isNull(void) const
{
    bool ret = true;
    for(unsigned int j = 0; j < mData.size(); j++)
	{
        if (mData[j].size() != 0)
		{
            ret = false;
			break;
        }
    }
	return ret;
}


AQLPriceDataType*
AQLDataStringMatrix::clone() const 
{
    try {
        AQLDataStringMatrix*  pAttr = new AQLDataStringMatrix(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

AQLString
AQLDataStringMatrix::convertToString(void) const
{
    AQLString result;
    if (isNull() )
    {
        result = NULL_STR;
    }
    else
    {
        result = "";
        size_t size = mData.size();
        for (size_t i = 0; i < size-1; ++i)
		{
            for (size_t j = 0; j < mData[i].size()-1; ++j)
			{
                result += mData[i][j] + DATA_COLL_DEL;
            }
			// no delimiter for the last data
			result += mData[i][mData[i].size()-1] + ';';
        }

        for (size_t j = 0; j < mData[size-1].size()-1; ++j)
		{
            result += mData[size - 1][j] + DATA_COLL_DEL;
        }
		// no delimiter for the last data
		result += mData[size - 1][mData[size-1].size()-1];
    }
    return result;
}

AQLStringMatrix&
AQLDataStringMatrix::get(void)
{
	update();
	return mData;
}

const AQLStringVector&
AQLDataStringMatrix::get(const unsigned int &i)
{
    char_t str[16];
    if (i + 1 > mData.size()){
        AQLString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += AQLString(str) + "]";
        throw AQLCoreSystemError(msg.getCString(), __FILE__, __LINE__);
    }
    return mData[i];
}

AQLString
AQLDataStringMatrix::get(const unsigned int &i, const unsigned int &j) const
{
    char_t str[16];

    if (i + 1 > mData.size()){
        AQLString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += AQLString(str) + "]";
        throw AQLCoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }
    if (j + 1 > mData[i].size()){
        AQLString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += AQLString(str) + "][";
        SPRINTF(str, "%d", j);
        msg += AQLString(str) + "]";
        throw AQLCoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }

    return mData[i][j];
}

unsigned int 
AQLDataStringMatrix::getSize() const
{
    unsigned int count = 0;
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        count += mData[i].size();
    }
    return count;
}

unsigned int
AQLDataStringMatrix::getSize(const unsigned int& i) const
{
    char_t str[16];
    if (i + 1 > mData.size()){
        AQLString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += AQLString(str) + "]";
        throw AQLCoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }

    return mData[i].size();
}

void
AQLDataStringMatrix::convertFromString(const AQLString& str)
{
    update();

    this->clear();
    if (str == NULL_STR || ! str.isDefined()) 
    {
        setNull();
        return;
    }

    vector<AQLString> tokens = str.toToken( ';' );
    vector<AQLString>::iterator it, it2;
    unsigned int i, j;

	setNull();
	mData.resize(tokens.size());
	for (i = 0, it = tokens.begin(); it != tokens.end(); i++, it++)
	{
		vector<AQLString> tokens2 = it->toToken( DATA_COLL_DEL );
		mData[i].resize(tokens2.size());
		for (j = 0, it2 = tokens2.begin(); it2 != tokens2.end(); j++, it2++)
		{
			setNull(false);
			mData[i][j] = *it2;
		}
	}
}


void
AQLDataStringMatrix::set(const unsigned int& i, const unsigned int& j, const AQLString& value)
{
    if (i + 1 > mData.size()){ 
        mData.resize(i + 1);
        mData[i].resize(j + 1);
    }
    else {
        if (j + 1 > mData[i].size()){
            mData[i].resize(j + 1);
        }
    }
    mData[i][j] = value;
    update();
    setNull(false);
}

void
AQLDataStringMatrix::set(const unsigned int &i, const AQLStringVector& strValarray)
{
    if (i + 1 > mData.size()){
        mData.resize(i + 1);
    }
    size_t valSize = strValarray.size();
    if (mData[i].size() < valSize){
        mData[i].resize(valSize);
    }

    mData[i] = strValarray;
    update();

    setNull();
    for(unsigned int j = 0; j < mData.size(); j++)
	{
        if (mData[j].size() != 0)
		{
            setNull(false);
            break;
        }
    }
}


void
AQLDataStringMatrix::set(const AQLStringMatrix& d)
{
    mData = d;
    update();
    setNull();
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        if (mData[i].size() != 0)
		{
            setNull(false);
            break;
        }
    }
}

void                    
AQLDataStringMatrix::clear()
{
    update();
    setNull();
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        mData[i].clear();
    }
	mData.clear();
}

AQLDataStringMatrix&
AQLDataStringMatrix::operator=(const AQLStringMatrix& d)
{
    set(d);
    return *this;
}

AQLPriceDataType&
AQLDataStringMatrix::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;
    if (a.getType() != DATA_STRING_MATRIX) 
    {
        AQLString err = "Assignment error for AQLDataStringMatrix : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const AQLDataStringMatrix&>(a).mData);
    return *this;
}

int
AQLDataStringMatrix::compare(const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_STRING_MATRIX) 
    {
        AQLString err = "Compare error for AQLDataStringMatrix : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return (int)this->getSize() - 
                (int)dynamic_cast<const AQLDataStringMatrix&>(a).getSize();
}

//============ AQLDataBoolMatrix  =============================
/*!
    @brief default constructor
*/
AQLDataBoolMatrix::AQLDataBoolMatrix(void) : AQLPriceDataType(DATA_BOOL_MATRIX)
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLDataBoolMatrix::AQLDataBoolMatrix(const AQLDataBoolMatrix& attr) : AQLPriceDataType(DATA_BOOL_MATRIX), mData(attr.mData)
{
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        if (mData[i].size() != 0)
		{
            setNull(false);
            break;
        }
    }
}

/*!
    @brief constructor

    @param[in] b bool vector data
*/
AQLDataBoolMatrix::AQLDataBoolMatrix(const BoolMatrix& matrix) : AQLPriceDataType(DATA_BOOL_MATRIX), mData(matrix)
{
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        if (mData[i].size() != 0)
		{
            setNull(false);
            break;
        }
    }
}

/*!
    @brief destructor
*/
AQLDataBoolMatrix::~AQLDataBoolMatrix(void)
{
}

bool
AQLDataBoolMatrix::isNull(void) const
{
    bool ret = true;
    for(unsigned int j = 0; j < mData.size(); j++)
	{
        if (mData[j].size() != 0)
		{
            ret = false;
			break;
        }
    }
	return ret;
}


/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set
*/
AQLPriceDataType*
AQLDataBoolMatrix::clone() const
{
    try
	{
        AQLDataBoolMatrix*    pAttr = new AQLDataBoolMatrix(*this);
        return pAttr;
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

    @return if it is NULL, return the "NULL" string data.
	           In the case of true / false, return "TRUE", "FALSE" string respectively.
*/
AQLString      
AQLDataBoolMatrix::convertToString(void) const
{
    AQLString result;
    if (isNull())
    {
        result = NULL_STR;
    }
    else
    {
		result = "";
        size_t size = mData.size();
        for (size_t i = 0; i < size-1; ++i)
		{
            for (size_t j = 0; j < mData[i].size()-1; ++j)
			{
                result += (mData[i][j] ? "TRUE" : "FALSE") + DATA_COLL_DEL;
            }
			// no delimiter for the last data
			result += (mData[i][mData[i].size()-1] ? "TRUE" : "FALSE") + ';';
        }

        for (size_t j = 0; j < mData[size-1].size()-1; ++j)
		{
            result += (mData[size - 1][j] ? "TRUE" : "FALSE") + DATA_COLL_DEL;
        }
		// no delimiter for the last data
		result += (mData[size - 1][mData[size-1].size()-1] ? "TRUE" : "FALSE");
    }
    return result;
}

BoolMatrix&
AQLDataBoolMatrix::get(void)
{
	update();
	return mData;
}

const BoolVector&
AQLDataBoolMatrix::get(const unsigned int &i)
{
    char_t str[16];
    if (i + 1 > mData.size()){
        AQLString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += AQLString(str) + "]";
        throw AQLCoreSystemError(msg.getCString(), __FILE__, __LINE__);
    }
    return mData[i];
}

bool
AQLDataBoolMatrix::get(const unsigned int &i, const unsigned int &j) const
{
    char_t str[16];

    if (i + 1 > mData.size()){
        AQLString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += AQLString(str) + "]";
        throw AQLCoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }
    if (j + 1 > mData[i].size()){
        AQLString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += AQLString(str) + "][";
        SPRINTF(str, "%d", j);
        msg += AQLString(str) + "]";
        throw AQLCoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }

    return mData[i][j];
}

unsigned int 
AQLDataBoolMatrix::getSize() const
{
    unsigned int count = 0;
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        count += mData[i].size();
    }
    return count;
}

unsigned int
AQLDataBoolMatrix::getSize(const unsigned int& i) const
{
    char_t str[16];
    if (i + 1 > mData.size()){
        AQLString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += AQLString(str) + "]";
        throw AQLCoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }

    return mData[i].size();
}

/*!
    @brief set boolian vector data from a specific string format

	true and false data is set by selecting "TRUE" or "FALSE" string separated by delimiter(mixed lowercase is OK)
	NULL value is set if no selecting or selecting "NULL" string.
	Other selected strings causes exception.

    @param[in] str "TRUE" or "FALSE"
*/
void          
AQLDataBoolMatrix::convertFromString(
    const AQLString& str)
{
    update();

     this->clear();
    if (str == NULL_STR || ! str.isDefined()) 
    {
        setNull();
        return;
    }

    vector<AQLString> tokens = str.toToken( ';' );
    vector<AQLString>::iterator it, it2;
	unsigned int i, j;

	setNull();
	mData.resize(tokens.size());
	for (i = 0, it = tokens.begin(); it != tokens.end(); i++, it++)
	{
		vector<AQLString> tokens2 = it->toToken( DATA_COLL_DEL );
		mData[i].resize(tokens2.size());
		for (j = 0, it2 = tokens2.begin(); it2 != tokens2.end(); j++, it2++)
		{
			setNull(false);
			it2->toUpper();
			if (*it2 == "TRUE") 
			{
				mData[i][j] = true;
			}
			else if (*it2 == "FALSE") 
			{
				mData[i][j] = false;
			}
			else 
			{
				// exception
				AQLString err = "InvalidString for AQLDataBoolMatrix : ";
				err += str;
				throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
			}
		}
    }
}

/*!
    @brief set data at the specified position

    @param[in] b data to be set
    @param[in] i element number(starting at 0)
*/
void
AQLDataBoolMatrix::set(const unsigned int& i, const unsigned int& j, bool b)
{
    if (i + 1 > mData.size()){ 
        mData.resize(i + 1);
        mData[i].resize(j + 1);
    }
    else {
        if (j + 1 > mData[i].size()){
            mData[i].resize(j + 1);
        }
    }
    mData[i][j] = b;
    update();
    setNull(false);
}

void
AQLDataBoolMatrix::set(const unsigned int &i, const BoolVector& boolValarray)
{
    if (i + 1 > mData.size()){
        mData.resize(i + 1);
    }
    size_t valSize = boolValarray.size();
    if (mData[i].size() < valSize){
        mData[i].resize(valSize);
    }

    mData[i] = boolValarray;
    update();

    setNull();
    for(unsigned int j = 0; j < mData.size(); j++)
	{
        if (mData[j].size() != 0)
		{
            setNull(false);
            break;
        }
    }
}

/*!
    @brief set data

    @param[in] b data to be set
*/
void
AQLDataBoolMatrix::set(const BoolMatrix& b)
{
    mData = b;
    update();
    setNull();
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        if (mData[i].size() != 0)
		{
            setNull(false);
            break;
        }
    }
}

/*!
    @brief clear all data
*/
void
AQLDataBoolMatrix::clear()
{
    update();
    setNull();
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        mData[i].clear();
    }
	mData.clear();
}

/*
    @brief assignment operator

    @param[in] b right hand side vector data of assignment operator

    @return the object
*/
AQLDataBoolMatrix&
AQLDataBoolMatrix::operator =(const BoolMatrix& b)
{
    set(b);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
AQLPriceDataType&
AQLDataBoolMatrix::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;
    if (a.getType() != DATA_BOOL_MATRIX) 
    {
        // exception
        AQLString err = "Assignment error for AQLDataBoolMatrix : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const AQLDataBoolMatrix&>(a).mData);
    setNull(a.isNull());
    return *this;
}

/*
    @brief compare the contents against the other objecet (Magnitude comparison is carried out in the number of elements.)
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared
    
    @retval Number of elements of the object - Number of elements of the object to be compared
*/
int
AQLDataBoolMatrix::compare(
    const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_BOOL_MATRIX) 
    {
        AQLString err = "Compare error for AQLDataBoolMatrix : from ";
        err += AQLString(a.getType());
        // exception
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return mData.size() - 
            dynamic_cast<const AQLDataBoolMatrix&>(a).mData.size();
}
