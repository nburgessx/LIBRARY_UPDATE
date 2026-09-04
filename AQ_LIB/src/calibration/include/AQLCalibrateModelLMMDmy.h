/*! @file
    @brief LMM Dmy sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef AQLCalibrateModelLMMDmy_h
#define AQLCalibrateModelLMMDmy_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCalibrateModelLMM.h"


class AQLDataInstance;
class AQLString;
class AQLRatesCurveLogLinearInterpolation;

//===================== Class Declare AQLCalibrateModelLMMDmy ==================================
/*! 
    @brief LMM Dmy SDE generator 
	

*/
class AQLCalibrateModelLMMDmy : public AQLCalibrateModelLMM
{

public:
	// constructor
	explicit AQLCalibrateModelLMMDmy(const AQLString &baseCurrency);
	// destructor
	virtual ~AQLCalibrateModelLMMDmy(void);
	// copy constructor
	AQLCalibrateModelLMMDmy(const AQLCalibrateModelLMMDmy &rhs);
	AQLCalibrateModelLMMDmy &operator=(const AQLCalibrateModelLMMDmy &rhs);
protected:
	//==============================================
	// create sde instance 
	virtual AQLRatesSDEBase *createSDEInstance(const AQLString &currency, AQLDataInstance &dataInstance) const;
	//==============================================
	// create log linear interpolation
	virtual  AQLRatesCurveLogLinearInterpolation *createCurveLogLinearInterpolation() const;


private:

};
#endif
