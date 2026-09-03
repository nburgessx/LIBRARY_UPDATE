/*! @file
    @brief Source code of of path element class that represents SCALAR value


*/
//  2007, Mizuho International London..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAModelDynamicsScalar.h"
#include "LAString.h"


using namespace std;
//================ LARatesPathElementScalar ===================================
/*!
	@brief default constructor
	@param[in] a value
*/
LARatesPathElementScalar::LARatesPathElementScalar(SCALAR a) 
: LARatesPathElementBase()/*, mValue(a)*/
{
	mValue.resize(1);
	mValue[0] = a;
}

/*!
	@brief copy constructor
	@param[in] v copy source
*/
LARatesPathElementScalar::LARatesPathElementScalar(const LARatesPathElementScalar& v)
: LARatesPathElementBase(v)
{	
}

/*!
	@brief destructor
*/
LARatesPathElementScalar::~LARatesPathElementScalar() 
{
	;
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesPathElementScalar::isTypeOf(pathelement_t id) const
{
	return (id == PE_SCALAR ? true : LARatesPathElementBase::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesPathElementScalar::getType() const
{
	return PE_SCALAR;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LARatesPathElementBase*	
LARatesPathElementScalar::clone() const
{
    try 
	{
		return new LARatesPathElementScalar(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief equal operator
    @param[in] a source object
    @return this object
*/
LARatesPathElementScalar&
LARatesPathElementScalar::operator = (const LARatesPathElementScalar& a)
{
	// 自分自身のコピーはしない
	if (this == &a) return *this;
	
	set(a);
	return *this;
}
/*!
    @brief set value
    @param[in] a source object
*/
void
LARatesPathElementScalar::set(const LARatesPathElementBase& a)
{
	if (a.getType() != PE_SCALAR) 
	{	
		// タイプチェックの際おかしいことが起こったら
		// 例外発生
		LAString err = "set error for LARatesPathElementScalar : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	// コピー。
	mValue = dynamic_cast<const LARatesPathElementScalar&>(a).mValue;
}

/*!
    @brief set value
	@param[in] a value to set
*/
void
LARatesPathElementScalar::set(const SCALARARRAY& a)
{
	if (a.size() == 1)
		 mValue[0] = a[0];
	else
	{
		throw LACoreInvalidData("input size must be one", __FILE__, __LINE__);
	}
}
