/*! @file
    @brief Source code of of path element class that represents SCALAR value
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLModelDynamicsScalar.h"
#include "AQLString.h"


using namespace std;
//================ AQLRatesPathElementScalar ===================================
/*!
	@brief default constructor
	@param[in] a value
*/
AQLRatesPathElementScalar::AQLRatesPathElementScalar(SCALAR a) 
: AQLRatesPathElementBase()/*, mValue(a)*/
{
	mValue.resize(1);
	mValue[0] = a;
}

/*!
	@brief copy constructor
	@param[in] v copy source
*/
AQLRatesPathElementScalar::AQLRatesPathElementScalar(const AQLRatesPathElementScalar& v)
: AQLRatesPathElementBase(v)
{	
}

/*!
	@brief destructor
*/
AQLRatesPathElementScalar::~AQLRatesPathElementScalar() 
{
	;
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesPathElementScalar::isTypeOf(pathelement_t id) const
{
	return (id == PE_SCALAR ? true : AQLRatesPathElementBase::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesPathElementScalar::getType() const
{
	return PE_SCALAR;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLRatesPathElementBase*	
AQLRatesPathElementScalar::clone() const
{
    try 
	{
		return new AQLRatesPathElementScalar(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief equal operator
    @param[in] a source object
    @return this object
*/
AQLRatesPathElementScalar&
AQLRatesPathElementScalar::operator = (const AQLRatesPathElementScalar& a)
{
	// 
	if (this == &a) return *this;
	
	set(a);
	return *this;
}
/*!
    @brief set value
    @param[in] a source object
*/
void
AQLRatesPathElementScalar::set(const AQLRatesPathElementBase& a)
{
	if (a.getType() != PE_SCALAR) 
	{	
		// 
		// 
		AQLString err = "set error for AQLRatesPathElementScalar : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	// 
	mValue = dynamic_cast<const AQLRatesPathElementScalar&>(a).mValue;
}

/*!
    @brief set value
	@param[in] a value to set
*/
void
AQLRatesPathElementScalar::set(const SCALARARRAY& a)
{
	if (a.size() == 1)
		 mValue[0] = a[0];
	else
	{
		throw AQLCoreInvalidData("input size must be one", __FILE__, __LINE__);
	}
}
