/*! @file
    @brief Ptberg SDE generator class. The SDE includes skew.
*/
//  2007, AlgoQuantHub.
#ifndef LACalibrateModelPtbergSkew_h
#define LACalibrateModelPtbergSkew_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelPtbergSkew.h
//
//  DESCRIPTION :       Ptberg SDE generator
//                      Create SDE which includes skew
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LACalibrateModelPtberg.h"



//===================== Class Declare LACalibrateModelPtbergSkew==================================
/*! 
    @brief Ptberg SDE generator. The SDE includes skew 
	

*/
class LACalibrateModelPtbergSkew : public LACalibrateModelPtberg
{

public:
	// constructor
	explicit LACalibrateModelPtbergSkew();
	// destructor
	virtual ~LACalibrateModelPtbergSkew(void);
	// copy constructor
	LACalibrateModelPtbergSkew(const LACalibrateModelPtbergSkew &rhs);
	LACalibrateModelPtbergSkew &operator=(const LACalibrateModelPtbergSkew &rhs);
protected:
	friend class MAPtbergUtils;
	//==============================================
	// set drift 
	virtual  void setDrift(const AQLString &fx, AQLRatesSDEBase &sde)  const;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const AQLString &fx, AQLRatesSDEBase &sde)  const;
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const;

};
#endif
