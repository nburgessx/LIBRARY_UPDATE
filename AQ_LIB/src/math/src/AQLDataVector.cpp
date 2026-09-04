/*! @file
    @brief Implementation of basic data classes whose types are vector.

    Implement following base Data classes. (AQLDataBools, AQLDataInts, AQLDataDoubles, AQLDataStrings,
    AQLDataDates, AQLDataDateTimes)
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLDataVector.h"

using namespace std;

//============ AQLDataBools  =============================
/*!
    @brief default constructor
*/
AQLDataBools::AQLDataBools(void) : AQLPriceDataType(DATA_BOOLS)
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLDataBools::AQLDataBools(const AQLDataBools& attr) : AQLPriceDataType(DATA_BOOLS)
{
    setNull(attr.isNull());
    mData = attr.mData;
}

/*!
    @brief constructor

    @param[in] b bool vector data
*/
AQLDataBools::AQLDataBools(const BoolVector& b) : AQLPriceDataType(DATA_BOOLS)
{
    setNull(b.size() == 0);
    mData = b;
}

/*!
    @brief destructor
*/
AQLDataBools::~AQLDataBools(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set
*/
AQLPriceDataType*
AQLDataBools::clone() const
{
    try {
        AQLDataBools*    pAttr = new AQLDataBools(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

    @return if it is NULL, return the "NULL" string data.
	           In the case of true / false, return "TRUE", "FALSE" string respectively.
*/
AQLString      
AQLDataBools::convertToString(void) const
{
    AQLString ret;
    if (isNull() )
    {
        ret = NULL_STR;
    }
    else
    {
        ret = "";
        vector<bool>::const_iterator it;
        for (it = mData.begin(); it != (mData.end()-1); ++it)
        {
            ret += (*it ? "TRUE" : "FALSE");
            ret += DATA_COLL_DEL;
        }
        ret += (*it ? "TRUE" : "FALSE");
    }
    return ret;
}

/*!
    @brief set boolian vector data from a specific string format

	true and false data is set by selecting "TRUE" or "FALSE" string separated by delimiter(mixed lowercase is OK)
	NULL value is set if no selecting or selecting "NULL" string.
	Other selected strings causes exception.

    @param[in] str "TRUE" or "FALSE"
*/
void          
AQLDataBools::convertFromString(
    const AQLString& str)
{
    update();

    mData.clear();
    if (str == NULL_STR) 
    {
        setNull();
        return;
    }

    vector<AQLString> tokens = str.toToken( DATA_COLL_DEL );
    vector<AQLString>::iterator it;
    for (it = tokens.begin(); it != tokens.end(); ++it)
    {
        it->toUpper();
        if (*it == "TRUE") 
        {
            mData.push_back(true);
        }
        else if (*it == "FALSE") 
        {
            mData.push_back(false);
        }
        else 
        {
            // exception
            AQLString err = "InvalidString for AQLDataBools : ";
            err += str;
            throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
        }
    }
    if (it != tokens.begin())
    {
        setNull(false);
    }
    else
    {
        setNull();
    }
}

/*!
    @brief set data at the specified position

    @param[in] b data to be set
    @param[in] i element number(starting at 0)
*/
void
AQLDataBools::set(bool b, int i)
{
    mData[i] = b;
    update();
}

/*!
    @brief set data

    @param[in] b data to be set
*/
void
AQLDataBools::set(const BoolVector& b)
{
    mData = b;
    if (mData.size() == 0) 
    {
        setNull();
    }
    else
    {
        setNull(false);
    }
    update();
}

/*!
    @brief add data to the end

    @param[in] b data to be added
*/
void
AQLDataBools::push_back(bool b)
{
    update();
    mData.push_back(b);
    if (isNull()) setNull(false);
}

/*!
    @brief an iterator pointing to the first element

    @return iterator pointing to the first element
*/
BoolVector::const_iterator
AQLDataBools::begin() const
{
    return mData.begin();
}

/*!
    @brief an iterator pointing to the element at the end

    @return iterator pointing to the element at the end
*/
BoolVector::const_iterator
AQLDataBools::end() const
{
    return mData.end();
}

/*!
    @brief insert the data
    
    @param[in] it iterator pointing to the element at the insertion (inserted before the element to be specified)
    @param[in] b data to be inserted
*/
void
//AQLDataBools::insert(BoolVector::const_iterator it, bool b)
AQLDataBools::insert(BoolVector::iterator it, bool b)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.insert(mData.begin()+i, b);
    mData.insert(it, b);
    if (isNull()) setNull(false);
}

/*!
    @brief delete the data for the specified element
    
    @param[in] it iterator pointing to the element to be deleted
*/
void                    
//AQLDataBools::erase(BoolVector::const_iterator it)
AQLDataBools::erase(BoolVector::iterator it)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.erase(mData.begin() + i);
    mData.erase(it);
    if (mData.size() == 0) setNull();
}

/*!
    @brief clear all data
*/
void
AQLDataBools::clear()
{
    update();
    setNull();
    mData.clear();
}

/*
    @brief array subscript operator
    
    @param[in] i array subscript

    @return the value at the specified position
*/
bool            
AQLDataBools::operator[](int i) const
{
    return mData[i];
}

/*
    @brief assignment operator

    @param[in] b right hand side vector data of assignment operator

    @return the object
*/
AQLDataBools&
AQLDataBools::operator =(const BoolVector& b)
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
AQLDataBools::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;
    if (a.getType() != DATA_BOOLS) 
    {
        // exception
        AQLString err = "Assignment error for AQLDataBools : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const AQLDataBools&>(a).mData);
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
AQLDataBools::compare(
    const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_BOOLS) 
    {
        AQLString err = "Compare error for AQLDataBools : from ";
        err += AQLString(a.getType());
        // exception
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return mData.size() - 
            dynamic_cast<const AQLDataBools&>(a).mData.size();
}

//============ AQLDataStrings  =============================
/*!
    @brief default constructor
*/
AQLDataStrings::AQLDataStrings(void) : AQLPriceDataType(DATA_STRINGS)
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLDataStrings::AQLDataStrings(const AQLDataStrings& attr) 
: AQLPriceDataType(DATA_STRINGS), mData(attr.mData)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief constructor

    @param[in] attr AQLString vector data
*/
AQLDataStrings::AQLDataStrings(const AQLStringVector& attr) 
: AQLPriceDataType(DATA_STRINGS), mData(attr)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief destructor
*/
AQLDataStrings::~AQLDataStrings(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set
*/
AQLPriceDataType*
AQLDataStrings::clone() const
{
    try {
        AQLDataStrings*  pAttr = new AQLDataStrings(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

    @retval NULL in case of NULL data
	@retval string that it holds otherwise

*/
AQLString
AQLDataStrings::convertToString(void) const
{
    AQLString ret;
    if (isNull() )
    {
        ret = NULL_STR;
    }
    else
    {
        ret = "";
        vector<AQLString>::const_iterator it;
        for (it = mData.begin(); it != (mData.end()-1); ++it)
        {
            ret += *it;
            ret += DATA_COLL_DEL;
        }
        ret += *it;
    }
    return ret;
}

/*!
    @brief set string vector data from a specific string format

	string data is set into vector if selecting string separated by delimiter

    @param[in] str string separated by a delimiter string
*/
void          
AQLDataStrings::convertFromString(
    const AQLString& str)
{
    vector<AQLString> tokens = str.toToken( DATA_COLL_DEL );
    set(tokens);
}

/*!
    @brief set data at the specified position

    @param[in] str data to be set
    @param[in] i element number(starting at 0)
*/
void
AQLDataStrings::set(const AQLString& str, int i)
{
    mData[i] = str;
    update();
}

/*!
    @brief set data

    @param[in] d data to be set
*/
void
AQLDataStrings::set(const AQLStringVector& d)
{
    update();
    mData = d;
    if (mData.size() != 0)
    {
        setNull(false);
    }
    else
    {
        setNull();
    }
}

/*!
    @brief add data to the end

    @param[in] b data to be added
*/
void
AQLDataStrings::push_back(const AQLString& b)
{
    update();
    mData.push_back(b);
    if (isNull()) setNull(false);
}

/*!
    @brief an iterator pointing to the first element

    @return iterator pointing to the first element
*/
AQLStringVector::const_iterator
AQLDataStrings::begin() const
{
    return mData.begin();
}

/*!
    @brief an iterator pointing to the element at the end

    @return iterator pointing to the element at the end
*/
AQLStringVector::const_iterator
AQLDataStrings::end() const
{
    return mData.end();
}

/*!
    @brief insert the data
    
    @param[in] it iterator pointing to the element at the insertion (inserted before the element to be specified)
    @param[in] b data to be inserted
*/
void
//AQLDataStrings::insert(AQLStringVector::const_iterator it, const AQLString& b)
AQLDataStrings::insert(AQLStringVector::iterator it, const AQLString& b)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.insert(mData.begin()+i, b);
    mData.insert(it, b);
    if (isNull()) setNull(false);
}

/*!
    @brief delete the data for the specified element
    
    @param[in] it iterator pointing to the element to be deleted
*/
void                    
//AQLDataStrings::erase(AQLStringVector::const_iterator it)
AQLDataStrings::erase(AQLStringVector::iterator it)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.erase(mData.begin() + i);
    mData.erase(it);
    if (mData.size() == 0) setNull();
}

/*!
    @brief clear all data
*/
void
AQLDataStrings::clear()
{
    update();
    setNull();
    mData.clear();
}

/*
    @brief array subscript operator
    
    @param[in] i array subscript

    @return the value at the specified position
*/
const AQLString&         
AQLDataStrings::operator[](int i) const
{
    return mData[i];
}

/*
    @brief assignment operator

    @param[in] d right hand side vector data of assignment operator

    @return the object
*/
AQLDataStrings&
AQLDataStrings::operator=(const AQLStringVector& d)
{
    set(d);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
AQLPriceDataType&
AQLDataStrings::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_STRINGS) 
    {
        // exception
        AQLString err = "Assignment error for AQLDataStrings : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const AQLDataStrings&>(a).mData);
    return *this;
}

/*
    @brief compare the contents against the other objecet (Magnitude comparison is carried out in the number of elements.)
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared
    
    @retval Number of elements of the object - Number of elements of the object to be compared
*/
int
AQLDataStrings::compare(
    const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_STRINGS) 
    {
        // exception
        AQLString err = "Compare error for AQLDataStrings : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return mData.size() - 
        dynamic_cast<const AQLDataStrings&>(a).mData.size();
}

//============ AQLDataDateTimes  =============================
/*!
    @brief default constructor
*/
AQLDataDateTimes::AQLDataDateTimes(void) : AQLPriceDataType(DATA_DATETIMES)
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLDataDateTimes::AQLDataDateTimes(const AQLDataDateTimes& attr) 
: AQLPriceDataType(DATA_DATETIMES), mData(attr.mData)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief constructor

    @param[in] attr bool AQLDateTime vector data
*/
AQLDataDateTimes::AQLDataDateTimes(const DateTimeVector& attr) 
: AQLPriceDataType(DATA_DATETIMES), mData(attr)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief destructor
*/
AQLDataDateTimes::~AQLDataDateTimes(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set
*/
AQLPriceDataType*
AQLDataDateTimes::clone() const
{
    try {
        AQLDataDateTimes*    pAttr = new AQLDataDateTimes(*this);
        return pAttr;   
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

	@retval NULL in case of NULL data
	@retval string of date time data that it holds otherwise

*/
AQLString
AQLDataDateTimes::convertToString(void) const
{
    AQLString ret;
    if (isNull())
    {
        ret = NULL_STR;
    }
    else
    {
        ret = "";
        vector<AQLDateTime>::const_iterator it;
        for (it = mData.begin(); it != (mData.end()-1); ++it)
        {
            ret += (*it).stringWithFormat();
            ret += DATA_COLL_DEL;
        }
        ret += (*it).stringWithFormat();
    }
    return ret;
}

/*!
    @brief set date vector data from a specific string format

	date time data is set into vector if selecting date time string separated by delimiter

    @param[in] str date time string separated by a delimiter string
*/
void
AQLDataDateTimes::convertFromString(
    const AQLString& str)
{
    update();
	mData.clear();
    if (str == NULL_STR)
    {
        setNull();
        return;
    }

    vector<AQLString> tokens = str.toToken( DATA_COLL_DEL );
    vector<AQLString>::iterator it;
    for (it = tokens.begin(); it != tokens.end(); ++it)
    {
        try 
        {
            AQLDateTime d((*it).getCString());
            mData.push_back(d);
        }
        catch(...) 
        {
            // exception
            AQLString err = "InvalidString for AQLDataDates : ";
            err += str;
            throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
        }
    }
    if (it != tokens.begin())
    {
        setNull(false);
    }
    else
    {
        setNull();
    }
}

/*!
    @brief set data

    @param[in] d data to be set
    @param[in] i element number(starting at 0)
*/
void
AQLDataDateTimes::set(const AQLDateTime& d, int i)
{
    mData[i] = d;
    update();
}

/*!
    @brief set the specified data

    @param[in] d data to be set
*/
void
AQLDataDateTimes::set(const DateTimeVector& d)
{
    mData = d;
    update();
    if (mData.size() == 0)
    {
        setNull();
    }
    else
    {
        setNull(false);
    }
}

/*!
    @brief add data to the end

    @param[in] b data to be added
*/
void
AQLDataDateTimes::push_back(const AQLDateTime& b)
{
    update();
    mData.push_back(b);
    if (isNull()) setNull(false);
}

/*!
    @brief an iterator pointing to the first element

    @return iterator pointing to the first element
*/
DateTimeVector::const_iterator
AQLDataDateTimes::begin() const
{
    return mData.begin();
}

/*!
    @brief an iterator pointing to the element at the end

    @return iterator pointing to the element at the end
*/
DateTimeVector::const_iterator
AQLDataDateTimes::end() const
{
    return mData.end();
}

/*!
    @brief insert the data
    
    @param[in] it iterator pointing to the element at the insertion (inserted before the element to be specified)
    @param[in] b data to be inserted
*/
void                    
//AQLDataDateTimes::insert(DateTimeVector::const_iterator it, const AQLDateTime& b)
AQLDataDateTimes::insert(DateTimeVector::iterator it, const AQLDateTime& b)
{
    update();
//  ysuzuki
//    int i = it - begin();
//    mData.insert(mData.begin() + i, b);
    mData.insert(it, b);
    if (isNull()) setNull(false);
}

/*!
    @brief delete the data for the specified element
    
    @param[in] it iterator pointing to the element to be deleted
*/
void                    
//AQLDataDateTimes::erase(DateTimeVector::const_iterator it)
AQLDataDateTimes::erase(DateTimeVector::iterator it)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.erase(mData.begin() + i);
    mData.erase(it);
    if (mData.size() == 0) setNull();
}

/*!
    @brief clear all data
*/
void                    
AQLDataDateTimes::clear()
{
    update();
    setNull();
    mData.clear();
}

/*
    @brief array subscript operator
    
    @param[in] i array subscript

    @return the value at the specified position
*/
const AQLDateTime&                   
AQLDataDateTimes::operator[] (int i) const
{
    return mData[i];
}

/*
    @brief assignment operator

    @param[in] d right hand side vector data of assignment operator

    @return the object
*/
AQLDataDateTimes&
AQLDataDateTimes::operator=(const DateTimeVector& d)
{
    set(d);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
AQLPriceDataType&
AQLDataDateTimes::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_DATETIMES) 
    {
        // exception
        AQLString err = "Assignment error for AQLDataDateTimes : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const AQLDataDateTimes&>(a).mData);
    return *this;
}

/*
    @brief compare the contents against the other objecet (Magnitude comparison is carried out in the number of elements.)
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared
    
    @retval Number of elements of the object - Number of elements of the object to be compared
*/
int
AQLDataDateTimes::compare(
    const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_DATETIMES) 
    {
        // exception
        AQLString err = "Compare error for AQLDataDateTimes : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }

    return mData.size() - 
            dynamic_cast<const AQLDataDateTimes&>(a).mData.size();
}

//============ AQLDataDates =============================
/*!
    @brief default constructor
*/
AQLDataDates::AQLDataDates(void) : AQLPriceDataType(DATA_DATES)
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLDataDates::AQLDataDates(const AQLDataDates& attr) 
: AQLPriceDataType(DATA_DATES), mData(attr.mData)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief constructor

    @param[in] attr AQLDate vector data
*/
AQLDataDates::AQLDataDates(const DateVector& attr) 
: AQLPriceDataType(DATA_DATES), mData(attr)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief destructor
*/
AQLDataDates::~AQLDataDates(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set
*/
AQLPriceDataType*
AQLDataDates::clone() const
{
    try {
        AQLDataDates*    pAttr = new AQLDataDates(*this);
        return pAttr;   
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

	@retval NULL in case of NULL data
	@retval string of date that it holds otherwise

*/
AQLString
AQLDataDates::convertToString(void) const
{
    AQLString ret;
    if (isNull() )
    {
        ret = NULL_STR;
    }
    else
    {
        ret = "";
        vector<AQLDate>::const_iterator it;
        for (it = mData.begin(); it != (mData.end()-1); ++it)
        {
            ret += (*it).stringWithFormat();
            ret += DATA_COLL_DEL;
        }
        ret += (*it).stringWithFormat();
    }
    return ret;
}

/*!
    @brief set date vector data from a specific string format

	date data is set into vector if selecting date string separated by delimiter

	@param[in] str date string separated by a delimiter string
*/
void
AQLDataDates::convertFromString(
    const AQLString& str)
{
    update();
	mData.clear();
    if (str == NULL_STR)
    {
        setNull();
        return;
    }

    vector<AQLString> tokens = str.toToken( DATA_COLL_DEL );
    vector<AQLString>::iterator it;
    for (it = tokens.begin(); it != tokens.end(); ++it)
    {
        try 
        {
            AQLDate d((*it).getCString());
            mData.push_back(d);
        }
        catch(...) 
        {
            // exception
            AQLString err = "InvalidString for AQLDataDates : ";
            err += str;
            throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
        }
    }
    if (it != tokens.begin())
    {
        setNull(false);
    }
    else
    {
        setNull();
    }
}

/*!
    @brief set data at the specified position 

    @param[in] d data to be set
    @param[in] i element number(starting at 0)
*/
void
AQLDataDates::set(const AQLDate& d, int i)
{
    mData[i] = d;
    update();
}

/*!
    @brief set data

    @param[in] d data to be set
*/
void
AQLDataDates::set(const DateVector& d)
{
    mData = d;
    update();
    if (mData.size() == 0)
    {
        setNull();
    }
    else
    {
        setNull(false);
    }
}

/*!
    @brief add data to the end

    @param[in] b data to be added
*/
void
AQLDataDates::push_back(const AQLDate& b)
{
    update();
    mData.push_back(b);
    if (isNull()) setNull(false);
}

/*!
    @brief an iterator pointing to the first element

    @return iterator pointing to the first element
*/
DateVector::const_iterator
AQLDataDates::begin() const
{
    return mData.begin();
}

/*!
    @brief an iterator pointing to the element at the end

    @return iterator pointing to the element at the end
*/
DateVector::const_iterator
AQLDataDates::end() const
{
    return mData.end();
}

/*!
    @brief insert the data
    
    @param[in] it iterator pointing to the element at the insertion (inserted before the element to be specified)
    @param[in] b data to be inserted
*/
void                    
//AQLDataDates::insert(DateVector::const_iterator it, const AQLDate& b)
AQLDataDates::insert(DateVector::iterator it, const AQLDate& b)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.insert(mData.begin()+i, b);
    mData.insert(it, b);
    if (isNull()) setNull(false);
}

/*!
    @brief delete the data for the specified element
    
    @param[in] it iterator pointing to the element to be deleted
*/
void                    
//AQLDataDates::erase(DateVector::const_iterator it)
AQLDataDates::erase(DateVector::iterator it)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.erase(mData.begin() + i);
    mData.erase(it);
    if (mData.size() == 0) setNull();
}

/*!
    @brief clear all data
*/
void                    
AQLDataDates::clear()
{
    update();
    setNull();
    mData.clear();
}

/*
    @brief array subscript operator
    
    @param[in] i array subscript

    @return the value at the specified position
*/
const AQLDate&                   
AQLDataDates::operator[] (int i) const
{
    return mData[i];
}

/*
    @brief assignment operator

    @param[in] d right hand side vector data of assignment operator

    @return the object
*/
AQLDataDates&
AQLDataDates::operator=(const DateVector& d)
{
    set(d);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
AQLPriceDataType&
AQLDataDates::assignment(const AQLPriceDataType& a)
{
    if (a.getType() != DATA_DATES) 
    {
        // exception
        AQLString err = "Assignment error for AQLDataDates : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const AQLDataDates&>(a).mData);
    return *this;
}

/*
    @brief compare the contents against the other objecet (Magnitude comparison is carried out in the number of elements.)
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared
    
    @retval Number of elements of the object - Number of elements of the object to be compared
*/
int
AQLDataDates::compare(
    const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_DATES) 
    {
        // exception
        AQLString err = "Compare error for AQLDataDates : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }

    return mData.size() - 
            dynamic_cast<const AQLDataDates&>(a).mData.size();
}

//============ AQLDataInts =============================
/*!
    @brief default constructor
*/
AQLDataInts::AQLDataInts(void) : AQLPriceDataType(DATA_INTS)
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLDataInts::AQLDataInts(const AQLDataInts& attr) 
: AQLPriceDataType(DATA_INTS), mData(attr.mData)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief constructor

    @param[in] attr valarray data with int type
*/
AQLDataInts::AQLDataInts(const IntArray& attr) 
: AQLPriceDataType(DATA_INTS), mData(attr)
{
    if (mData.size() != 0)
        setNull(false);
}


/*!
    @brief destructor
*/
AQLDataInts::~AQLDataInts(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set
*/
AQLPriceDataType*
AQLDataInts::clone() const
{
    try {
        AQLDataInts* pAttr = new AQLDataInts(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

	@retval NULL in case of NULL data
	@retval string of integer data that it holds otherwise

*/
AQLString
AQLDataInts::convertToString(void) const
{
    AQLString ret;
    if (isNull() )
    {
        ret = NULL_STR;
    }
    else
    {
        ret = "";
		int i;
        int size = mData.size();
        for (i=0; i< size-1; i++)
        {
            ret += AQLString(mData[i]);
            ret += DATA_COLL_DEL;
        }
        ret += AQLString(mData[size-1]);
    }
    return ret;
}

/*!
    @brief set integer vector data from a specific string format

	integer data is set into vector if selecting integer string separated by delimiter

    @param[in] str inteter string separated by a delimiter string
*/
void
AQLDataInts::convertFromString(
    const AQLString& str)
{
    update();

    mData.clear();
    if (str == NULL_STR) 
    {
        setNull();
        return;
    }

    vector<AQLString> tokens = str.toToken( DATA_COLL_DEL );
    vector<AQLString>::iterator it;
    for (it = tokens.begin(); it != tokens.end(); ++it)
    {
        mData.resize(mData.size() + 1, (*it).getIntValue());
    }
    if (it != tokens.begin())
    {
        setNull(false);
    }
    else
    {
        setNull();
    }
}

/*!
    @brief set data at the specified position

    @param[in] d data to be set
    @param[in] i element number(starting at 0)
*/
void
AQLDataInts::set(int d, int i)
{
    mData[i] = d;
    update();
}

/*!
    @brief set data

    @param[in] d data to be set
*/
void
AQLDataInts::set(const IntArray& d)
{
    mData = d;
    update();
    if (mData.size() == 0)
    {
        setNull();
    }
    else
    {
        setNull(false);
    }
}


/*!
    @brief resize data. In case of element added, assign a value i

    @param[in] num new size
	@param[in] i value to be assigned in case of element added
*/   
void
AQLDataInts::resize(unsigned int num, const int i)
{
    update();
	mData.resize(num,i);
}

/*!
    @brief clear all data
*/
void                    
AQLDataInts::clear()
{
    update();
    setNull();
    mData.clear();
}

/*
    @brief array subscript operator
    
    @param[in] i array subscript

    @return the value at the specified position
*/
int                 
AQLDataInts::operator[] (int i) const
{
    return mData[i];
}

/*
    @brief assignment operator

    @param[in] d right hand side vector data of assignment operator

    @return the object
*/
AQLDataInts&
AQLDataInts::operator=(const IntArray& d)
{
    set(d);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
AQLPriceDataType&
AQLDataInts::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_INTS)
    {
        // exception
        AQLString err = "Assignment error for AQLDataInts : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const AQLDataInts&>(a).mData);
    return *this;
}

/*
    @brief compare the contents against the other objecet (Magnitude comparison is carried out in the number of elements.)
    
    @param[in] a the object to be compared
    
    @retval Number of elements of the object - Number of elements of the object to be compared
*/
int
AQLDataInts::compare(
    const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_INTS) 
    {
        // exception
        AQLString err = "Compare error for AQLDataInts : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return mData.size() - 
        dynamic_cast<const AQLDataInts&>(a).mData.size();
}

//============ AQLDataDoubles =============================
/*!
    @brief constructor
    
    @param[in] pre number of decimal places
*/
AQLDataDoubles::AQLDataDoubles(int pre) : AQLPriceDataType(DATA_DOUBLES)
{
    mPre = pre;
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLDataDoubles::AQLDataDoubles(const AQLDataDoubles& attr) 
: AQLPriceDataType(DATA_DOUBLES), mData(attr.mData), mPre(attr.mPre)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief constructor

    @param[in] attr valarray data with doubl type 
    @param[in] pre number of decimal places
*/
AQLDataDoubles::AQLDataDoubles(const DoubleArray& attr, int pre) 
: AQLPriceDataType(DATA_DOUBLES), mData(attr), mPre(pre)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief destructor
*/
AQLDataDoubles::~AQLDataDoubles(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set
*/
AQLPriceDataType*
AQLDataDoubles::clone() const 
{
    try {
        AQLDataDoubles*  pAttr = new AQLDataDoubles(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

	@retval NULL in case of NULL data
	@retval string of double data that it holds otherwise

*/
AQLString
AQLDataDoubles::convertToString(void) const
{
    AQLString result;
    if (isNull() )
    {
        result = NULL_STR;
    }
    else
    {
		// OLD
		// ***

		// result = "";
		// size_t size = mData.size();
		//
		// for ( size_t i = 0; i < size-1; ++i)
        // {
        //     result += AQLString(mData[i], mPre);
        //     result += DATA_COLL_DEL;
		// 	
		// 	
        // }
        // result += AQLString(mData[size-1], mPre);

		// NEW
		// ***

		result = "";
		size_t size = mData.size();

		for ( size_t i = 0; i < size-1; ++i)
        {
            result += std::to_string(mData[i]) + DATA_COLL_DEL;
        }
        result += std::to_string(mData[size-1]);
    }
	
    return result;
}

/*!
    @brief set double vector data from a specific string format

    double data is set into vector if selecting double string separated by delimiter

    @param[in] str double string separated by a delimiter string
*/
void
AQLDataDoubles::convertFromString(
    const AQLString& str)
{
    update();

	mData.clear();
    if (str == NULL_STR) 
    {
        setNull();
        return;
    }

    vector<AQLString> tokens = str.toToken( DATA_COLL_DEL );
    vector<AQLString>::iterator it;
    for (it = tokens.begin(); it != tokens.end(); ++it)
    {
        mData.resize(mData.size()+1,(*it).getDoubleValue());
    }
    if (it != tokens.begin())
    {
        setNull(false);
    }
    else
    {
        setNull();
    }
}

/*!
    @brief set data at the specified position

    @param[in] d data to be set
    @param[in] i element number(starting at 0)
*/
void
AQLDataDoubles::set(double d, int i)
{
    mData[i] = d;
    update();
}

/*!
    @brief set data

    @param[in] d data to be set
*/
void
AQLDataDoubles::set(const DoubleArray& d)
{
    mData = d;
    update();
    if (mData.size() == 0)
    {
        setNull();
    }
    else
    {
        setNull(false);
    }
}

/*!
    @brief  resize data. In case of element added, assign a value i.

    @param[in] num new size
	@param[in] d value to be assigned in case of element added
*/   
void
AQLDataDoubles::resize(unsigned int num, const double d)
{
    update();
	mData.resize(num,d);
}

/*!
    @brief clear all data
*/
void                    
AQLDataDoubles::clear()
{
    update();
    setNull();
    mData.clear();
}

/*
    @briefarray subscript operator
    
    @param[in] i array subscript

    @return the value at the specified position
*/
double                  
AQLDataDoubles::operator[] (int i) const
{
    return mData[i];
}

/*
    @brief assignment operator

    @param[in] d right hand side vector data of assignment operator

    @return the object
*/
AQLDataDoubles&
AQLDataDoubles::operator=(const DoubleArray& d)
{
    set(d);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
AQLPriceDataType&
AQLDataDoubles::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;
    if (a.getType() != DATA_DOUBLES) 
    {
        // exception
        AQLString err = "Assignment error for AQLDataDoubles : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const AQLDataDoubles&>(a).mData);
    mPre = dynamic_cast<const AQLDataDoubles&>(a).mPre;
    return *this;
}

/*
    @brief compare the contents against the other objecet (Magnitude comparison is carried out in the number of elements.)
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared
    
    @retval Number of elements of the object - Number of elements of the object to be compared
*/
int
AQLDataDoubles::compare(
    const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_DOUBLES) 
    {
        // exception
        AQLString err = "Compare error for AQLDataDoubles : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return mData.size() - 
                dynamic_cast<const AQLDataDoubles&>(a).mData.size();
}
