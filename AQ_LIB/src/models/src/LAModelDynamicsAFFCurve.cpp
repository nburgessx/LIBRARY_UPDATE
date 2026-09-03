/*! @file
    @brief Source code of of path element class that represents curve object


*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

//+++++ INCLUDE +++++
#include "LAModelDynamicsAFFCurve.h"
#include "LAString.h"
#include "LABasic.h"
#include "float.h"
#include <cmath>

using namespace std;
class LARatesPathElementSRCurve;

static const double MAXIMUM_10031 = LAMath::log(DBL_MAX) - 1.0;
//================ LARatesPathElementAFFCurve ===================================
/*!
	@brief default constructor
	@param[in] t start time of this curve
*/
LARatesPathElementAFFCurve::LARatesPathElementAFFCurve(double t)
: LARatesPathElementSRCurve(t)
{
}

/*!
	@brief destructor
*/
LARatesPathElementAFFCurve::~LARatesPathElementAFFCurve() 
{
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesPathElementAFFCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_AFFCURVE ? true : LARatesPathElementSRCurve::isTypeOf(id));
}

double
LARatesPathElementAFFCurve::getP (double T) const
{
	if(m_t >= T)
		return 1.0;

	double afval = A(T)-B(T)*mValue[0];
	if(afval > MAXIMUM_10031)
		return LAMath::exp(MAXIMUM_10031);
	else
		return LAMath::exp(afval);
}
/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesPathElementAFFCurve::getType() const
{
	return PE_AFFCURVE;
}

