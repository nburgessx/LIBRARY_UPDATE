/*! @file
    @brief Ptberg SDE generator class. The SDE includes skew.
*/
#ifndef AQLCalibrateModelPtbergSkew_h
#define AQLCalibrateModelPtbergSkew_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCalibrateModelPtberg.h"



//===================== Class Declare AQLCalibrateModelPtbergSkew==================================
/*! 
    @brief Ptberg SDE generator. The SDE includes skew 
	

*/
class AQLCalibrateModelPtbergSkew : public AQLCalibrateModelPtberg
{

public:
	// constructor
	explicit AQLCalibrateModelPtbergSkew();
	// destructor
	virtual ~AQLCalibrateModelPtbergSkew(void);
	// copy constructor
	AQLCalibrateModelPtbergSkew(const AQLCalibrateModelPtbergSkew &rhs);
	AQLCalibrateModelPtbergSkew &operator=(const AQLCalibrateModelPtbergSkew &rhs);
protected:
	friend class AQLPtbergUtils;
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
