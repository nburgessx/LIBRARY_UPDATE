/*! @file
    @brief LMM Dmy sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef LACalibrateModelLMMDmy_h
#define LACalibrateModelLMMDmy_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelLMMDmy.h
//
//  DESCRIPTION :       Dmy LMM SDE generator 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LACalibrateModelLMM.h"


class AQLDataInstance;
class AQLString;
class AQLRatesCurveLogLinearInterpolation;

//===================== Class Declare LACalibrateModelLMMDmy ==================================
/*! 
    @brief LMM Dmy SDE generator 
	

*/
class LACalibrateModelLMMDmy : public LACalibrateModelLMM
{

public:
	// constructor
	explicit LACalibrateModelLMMDmy(const AQLString &baseCurrency);
	// destructor
	virtual ~LACalibrateModelLMMDmy(void);
	// copy constructor
	LACalibrateModelLMMDmy(const LACalibrateModelLMMDmy &rhs);
	LACalibrateModelLMMDmy &operator=(const LACalibrateModelLMMDmy &rhs);
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
