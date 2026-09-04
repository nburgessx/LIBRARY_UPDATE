/*! @file
    @brief Source code of of path element class that represents curve object
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

//+++++ INCLUDE +++++
#include "AQLModelDynamicsSRCurve.h"

using namespace std;

//
//------------------------------ AQLRatesPathElementSRCurve ------------------------------
//

/*!
	@brief default constructor
	@param[in] t spot time
    @param[in] r  short rate
*/
AQLRatesPathElementSRCurve::AQLRatesPathElementSRCurve( double t) : AQLRatesPathElementCurve( t )
{
	mValue.resize(1);
    mValue[0] = 0.0;
}

/*!
	@brief destructor
*/
AQLRatesPathElementSRCurve::~AQLRatesPathElementSRCurve() 
{
}
				
/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesPathElementSRCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_SRCURVE ? true : AQLRatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesPathElementSRCurve::getType() const
{
	return PE_SRCURVE;
}
