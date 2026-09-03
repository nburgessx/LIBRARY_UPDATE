/*! @file
    @brief Source code of of path element class that represents curve object


*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

//+++++ INCLUDE +++++
#include "LAModelDynamicsSRCurve.h"

using namespace std;

//
//------------------------------ LARatesPathElementSRCurve ------------------------------
//

/*!
	@brief default constructor
	@param[in] tÅ@spot time
    @param[in] r  short rate
*/
LARatesPathElementSRCurve::LARatesPathElementSRCurve( double t) : LARatesPathElementCurve( t )
{
	mValue.resize(1);
    mValue[0] = 0.0;
}

/*!
	@brief destructor
*/
LARatesPathElementSRCurve::~LARatesPathElementSRCurve() 
{
}
				
/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesPathElementSRCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_SRCURVE ? true : LARatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesPathElementSRCurve::getType() const
{
	return PE_SRCURVE;
}
