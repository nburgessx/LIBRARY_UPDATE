/*! @file
    @brief Implementation of data which represents Matrix type.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LADataMatrix.h"

using namespace std;

// ================= LADataDoubleMatrix ======================
/*!
    @brief constructor
    
    @param[in] pre number of decimal places
*/
LADataDoubleMatrix::LADataDoubleMatrix(int pre) : LAPriceDataType(DATA_DOUBLE_MATRIX)
{
    mPre = pre;
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LADataDoubleMatrix::LADataDoubleMatrix(const LADataDoubleMatrix& attr) 
    : LAPriceDataType(DATA_DOUBLE_MATRIX), mData(attr.mData), mPre(attr.mPre)
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
LADataDoubleMatrix::LADataDoubleMatrix(const DoubleMatrix& matrix, int pre) 
    : LAPriceDataType(DATA_DOUBLE_MATRIX), mData(matrix), mPre(pre)
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
LADataDoubleMatrix::~LADataDoubleMatrix(void)
{
}
/*!
    @brief check if NULL

    @return true if NULL, false otherwise
*/
bool
LADataDoubleMatrix::isNull(void) const
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

    @note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set
*/
LAPriceDataType*
LADataDoubleMatrix::clone() const 
{
    try {
        LADataDoubleMatrix*  pAttr = new LADataDoubleMatrix(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief convert matrix type data of the data into string format
    
    convert a string from a two-dimensional element in order, that is
	val[i][j] is converted to string and stored in order by
	val[0][0], val[0][1], val[0][2],..., val[1][0], val[1][1], ...

    @return in case of NULL data, return "NULL", otherwise, return double data separated by delimiter
*/
LAString
LADataDoubleMatrix::convertToString(void) const
{
	// OLD
	// ---

    // LAString ret;
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
    //             ret += LAString(mData[i][j], mPre);
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
    //         ret += LAString(mData[size - 1][j], mPre);
    //         // no delimiter for the last data
	// 		if (j != static_cast<int>(mData[size - 1].size() - 1))
	// 			ret += DATA_COLL_DEL;
    //     }
    // }
    // 
    // return ret;


	// NEW
	// ---
	
	LAString result;
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
LADataDoubleMatrix::get(void)
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
LADataDoubleMatrix::get(const unsigned int &i)
{
    char_t str[16]; // 16 digits is enough
    // If the memory of the specified element has not been reserved, then throw the error.
    if (i + 1 > mData.size()){
        LAString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += LAString(str) + "]";
        throw LACoreSystemError(msg.getCString(), __FILE__, __LINE__);
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
LADataDoubleMatrix::get(const unsigned int &i, const unsigned int &j) const
{
    char_t str[16]; // 16 digits is enough

    // If the memory of the specified element has not been reserved, then throw the error.
    if (i + 1 > mData.size()){
        LAString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += LAString(str) + "]";
        throw LACoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }
    if (j + 1 > mData[i].size()){
        LAString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += LAString(str) + "][";
        SPRINTF(str, "%d", j);
        msg += LAString(str) + "]";
        throw LACoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }

    return mData[i][j];
}

/*!
    @brief return the total number of elements in the two-dimensional data

    @return the total number of elements
*/
unsigned int 
LADataDoubleMatrix::getSize() const
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
LADataDoubleMatrix::getSize(const unsigned int& i) const
{
    char_t str[16]; // 16 digits is enough
    // If the memory of the specified element has not been reserved, then throw the error.
    if (i + 1 > mData.size()){
        LAString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += LAString(str) + "]";
        throw LACoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }

    return mData[i].size();
}

/*!
    @brief set (valarray two-dimensional data type double) DoubleMatrix data from a string of the specified floating-point format.

  
	@param[in] str double string separated by a delimiter string
*/
void
LADataDoubleMatrix::convertFromString(const LAString& str)
{
    update();

    this->clear();
    if (str == NULL_STR || ! str.isDefined()) 
    {
        setNull();
        return;
    }

    vector<LAString> tokens = str.toToken( ';' );
    vector<LAString>::iterator it, it2;
    unsigned int i, j;

	setNull();
	mData.resize(tokens.size());
	for (i = 0, it = tokens.begin(); it != tokens.end(); i++, it++)
	{
		vector<LAString> tokens2 = it->toToken( DATA_COLL_DEL );
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
LADataDoubleMatrix::set(const unsigned int& i, const unsigned int& j, const double& value)
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
LADataDoubleMatrix::set(const unsigned int &i, const DoubleArray& dValarray)
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
LADataDoubleMatrix::set(const DoubleMatrix& d)
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
LADataDoubleMatrix::clear()
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
LADataDoubleMatrix&
LADataDoubleMatrix::operator=(const DoubleMatrix& d)
{
    set(d);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents
*/
LAPriceDataType&
LADataDoubleMatrix::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;
    if (a.getType() != DATA_DOUBLE_MATRIX) 
    {
        // exception
        LAString err = "Assignment error for LADataDoubleMatrix : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const LADataDoubleMatrix&>(a).mData);
    mPre = dynamic_cast<const LADataDoubleMatrix&>(a).mPre;
    return *this;
}

/*
    @brief compare the contents against the other objecet
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared

	@return  (number of elements of this object) - (number of elements of object a)
*/
int
LADataDoubleMatrix::compare(const LAPriceDataType& a) const
{
    if (a.getType() != DATA_DOUBLE_MATRIX) 
    {
        // exception
        LAString err = "Compare error for LADataDoubleMatrix : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return (int)this->getSize() - 
                (int)dynamic_cast<const LADataDoubleMatrix&>(a).getSize();
}

///////////////////////////////////
// ================= LADataStringMatrix ======================
LADataStringMatrix::LADataStringMatrix(void) : LAPriceDataType(DATA_STRING_MATRIX)
{
}

LADataStringMatrix::LADataStringMatrix(const LADataStringMatrix& attr) 
    : LAPriceDataType(DATA_STRING_MATRIX), mData(attr.mData)
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

LADataStringMatrix::LADataStringMatrix(const LAStringMatrix& matrix) 
    : LAPriceDataType(DATA_STRING_MATRIX), mData(matrix)
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

LADataStringMatrix::~LADataStringMatrix(void)
{
}

bool
LADataStringMatrix::isNull(void) const
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


LAPriceDataType*
LADataStringMatrix::clone() const 
{
    try {
        LADataStringMatrix*  pAttr = new LADataStringMatrix(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

LAString
LADataStringMatrix::convertToString(void) const
{
    LAString result;
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

LAStringMatrix&
LADataStringMatrix::get(void)
{
	update();
	return mData;
}

const LAStringVector&
LADataStringMatrix::get(const unsigned int &i)
{
    char_t str[16];
    if (i + 1 > mData.size()){
        LAString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += LAString(str) + "]";
        throw LACoreSystemError(msg.getCString(), __FILE__, __LINE__);
    }
    return mData[i];
}

LAString
LADataStringMatrix::get(const unsigned int &i, const unsigned int &j) const
{
    char_t str[16];

    if (i + 1 > mData.size()){
        LAString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += LAString(str) + "]";
        throw LACoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }
    if (j + 1 > mData[i].size()){
        LAString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += LAString(str) + "][";
        SPRINTF(str, "%d", j);
        msg += LAString(str) + "]";
        throw LACoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }

    return mData[i][j];
}

unsigned int 
LADataStringMatrix::getSize() const
{
    unsigned int count = 0;
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        count += mData[i].size();
    }
    return count;
}

unsigned int
LADataStringMatrix::getSize(const unsigned int& i) const
{
    char_t str[16];
    if (i + 1 > mData.size()){
        LAString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += LAString(str) + "]";
        throw LACoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }

    return mData[i].size();
}

void
LADataStringMatrix::convertFromString(const LAString& str)
{
    update();

    this->clear();
    if (str == NULL_STR || ! str.isDefined()) 
    {
        setNull();
        return;
    }

    vector<LAString> tokens = str.toToken( ';' );
    vector<LAString>::iterator it, it2;
    unsigned int i, j;

	setNull();
	mData.resize(tokens.size());
	for (i = 0, it = tokens.begin(); it != tokens.end(); i++, it++)
	{
		vector<LAString> tokens2 = it->toToken( DATA_COLL_DEL );
		mData[i].resize(tokens2.size());
		for (j = 0, it2 = tokens2.begin(); it2 != tokens2.end(); j++, it2++)
		{
			setNull(false);
			mData[i][j] = *it2;
		}
	}
}


void
LADataStringMatrix::set(const unsigned int& i, const unsigned int& j, const LAString& value)
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
LADataStringMatrix::set(const unsigned int &i, const LAStringVector& strValarray)
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
LADataStringMatrix::set(const LAStringMatrix& d)
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
LADataStringMatrix::clear()
{
    update();
    setNull();
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        mData[i].clear();
    }
	mData.clear();
}

LADataStringMatrix&
LADataStringMatrix::operator=(const LAStringMatrix& d)
{
    set(d);
    return *this;
}

LAPriceDataType&
LADataStringMatrix::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;
    if (a.getType() != DATA_STRING_MATRIX) 
    {
        LAString err = "Assignment error for LADataStringMatrix : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const LADataStringMatrix&>(a).mData);
    return *this;
}

int
LADataStringMatrix::compare(const LAPriceDataType& a) const
{
    if (a.getType() != DATA_STRING_MATRIX) 
    {
        LAString err = "Compare error for LADataStringMatrix : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return (int)this->getSize() - 
                (int)dynamic_cast<const LADataStringMatrix&>(a).getSize();
}

//============ LADataBoolMatrix  =============================
/*!
    @brief default constructor
*/
LADataBoolMatrix::LADataBoolMatrix(void) : LAPriceDataType(DATA_BOOL_MATRIX)
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LADataBoolMatrix::LADataBoolMatrix(const LADataBoolMatrix& attr) : LAPriceDataType(DATA_BOOL_MATRIX), mData(attr.mData)
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
LADataBoolMatrix::LADataBoolMatrix(const BoolMatrix& matrix) : LAPriceDataType(DATA_BOOL_MATRIX), mData(matrix)
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
LADataBoolMatrix::~LADataBoolMatrix(void)
{
}

bool
LADataBoolMatrix::isNull(void) const
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

    @note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set
*/
LAPriceDataType*
LADataBoolMatrix::clone() const
{
    try
	{
        LADataBoolMatrix*    pAttr = new LADataBoolMatrix(*this);
        return pAttr;
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

    @return if it is NULL, return the "NULL" string data.
	           In the case of true / false, return "TRUE", "FALSE" string respectively.
*/
LAString      
LADataBoolMatrix::convertToString(void) const
{
    LAString result;
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
LADataBoolMatrix::get(void)
{
	update();
	return mData;
}

const BoolVector&
LADataBoolMatrix::get(const unsigned int &i)
{
    char_t str[16];
    if (i + 1 > mData.size()){
        LAString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += LAString(str) + "]";
        throw LACoreSystemError(msg.getCString(), __FILE__, __LINE__);
    }
    return mData[i];
}

bool
LADataBoolMatrix::get(const unsigned int &i, const unsigned int &j) const
{
    char_t str[16];

    if (i + 1 > mData.size()){
        LAString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += LAString(str) + "]";
        throw LACoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }
    if (j + 1 > mData[i].size()){
        LAString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += LAString(str) + "][";
        SPRINTF(str, "%d", j);
        msg += LAString(str) + "]";
        throw LACoreSystemError(msg.getCString(),__FILE__,__LINE__);
    }

    return mData[i][j];
}

unsigned int 
LADataBoolMatrix::getSize() const
{
    unsigned int count = 0;
    for (unsigned int i = 0; i < mData.size(); i++)
	{
        count += mData[i].size();
    }
    return count;
}

unsigned int
LADataBoolMatrix::getSize(const unsigned int& i) const
{
    char_t str[16];
    if (i + 1 > mData.size()){
        LAString msg("Unable to access data:mData[");
        SPRINTF(str, "%d", i);
        msg += LAString(str) + "]";
        throw LACoreSystemError(msg.getCString(),__FILE__,__LINE__);
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
LADataBoolMatrix::convertFromString(
    const LAString& str)
{
    update();

     this->clear();
    if (str == NULL_STR || ! str.isDefined()) 
    {
        setNull();
        return;
    }

    vector<LAString> tokens = str.toToken( ';' );
    vector<LAString>::iterator it, it2;
	unsigned int i, j;

	setNull();
	mData.resize(tokens.size());
	for (i = 0, it = tokens.begin(); it != tokens.end(); i++, it++)
	{
		vector<LAString> tokens2 = it->toToken( DATA_COLL_DEL );
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
				LAString err = "InvalidString for LADataBoolMatrix : ";
				err += str;
				throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
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
LADataBoolMatrix::set(const unsigned int& i, const unsigned int& j, bool b)
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
LADataBoolMatrix::set(const unsigned int &i, const BoolVector& boolValarray)
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
LADataBoolMatrix::set(const BoolMatrix& b)
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
LADataBoolMatrix::clear()
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
LADataBoolMatrix&
LADataBoolMatrix::operator =(const BoolMatrix& b)
{
    set(b);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
LAPriceDataType&
LADataBoolMatrix::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;
    if (a.getType() != DATA_BOOL_MATRIX) 
    {
        // exception
        LAString err = "Assignment error for LADataBoolMatrix : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const LADataBoolMatrix&>(a).mData);
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
LADataBoolMatrix::compare(
    const LAPriceDataType& a) const
{
    if (a.getType() != DATA_BOOL_MATRIX) 
    {
        LAString err = "Compare error for LADataBoolMatrix : from ";
        err += LAString(a.getType());
        // exception
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return mData.size() - 
            dynamic_cast<const LADataBoolMatrix&>(a).mData.size();
}
