/*! @file
    @brief Source code of of path element class that represents curve object


*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

//+++++ INCLUDE +++++
#include "AQLModelDynamicsAFFCurve.h"
#include "AQLString.h"
#include "AQLBasic.h"
#include "float.h"
#include <cmath>

using namespace std;
class AQLRatesPathElementSRCurve;

static const double MAXIMUM_10031 = AQLMath::log(DBL_MAX) - 1.0;
//================ AQLRatesPathElementAFFCurve ===================================
/*!
	@brief default constructor
	@param[in] t start time of this curve
*/
AQLRatesPathElementAFFCurve::AQLRatesPathElementAFFCurve(double t)
: AQLRatesPathElementSRCurve(t)
{
}

/*!
	@brief destructor
*/
AQLRatesPathElementAFFCurve::~AQLRatesPathElementAFFCurve() 
{
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesPathElementAFFCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_AFFCURVE ? true : AQLRatesPathElementSRCurve::isTypeOf(id));
}

double
AQLRatesPathElementAFFCurve::getP (double T) const
{
	if(m_t >= T)
		return 1.0;

	double afval = A(T)-B(T)*mValue[0];
	if(afval > MAXIMUM_10031)
		return AQLMath::exp(MAXIMUM_10031);
	else
		return AQLMath::exp(afval);
}
/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesPathElementAFFCurve::getType() const
{
	return PE_AFFCURVE;
}

