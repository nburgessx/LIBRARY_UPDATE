/*! @file
    @brief Ptberg sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef AQLCalibrateModelPtberg3F_h
#define AQLCalibrateModelPtberg3F_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "AQLCalibrateModelPtberg.h"


class AQLDataInstance;
class AQLMathVolatility;
class AQLCalibrationParametersPtberg;

//===================== Class Declare AQLCalibrateModelPtberg3F==================================
/*! 
    @brief Ptberg SDE generator 
	

*/
class AQLCalibrateModelPtberg3F : public AQLCalibrateModelPtberg
{

public:
	// constructor
	explicit AQLCalibrateModelPtberg3F();
	// destructor
	virtual ~AQLCalibrateModelPtberg3F(void);
	// copy constructor
	AQLCalibrateModelPtberg3F(const AQLCalibrateModelPtberg3F &rhs);
	AQLCalibrateModelPtberg3F &operator=(const AQLCalibrateModelPtberg3F &rhs);
protected:
	//==============================================
	// set drift 
	virtual  void setDrift(const AQLString &fx, AQLRatesSDEBase &sde)  const;
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const AQLString &fx) const;
private:
	//==============================================
	// set volatility object
	void setUpVolEntity(const AQLString &fx, AQLMathVolatility &vol) const;
	//==============================================
	// create calibration info
	virtual AQLCalibrationParametersPtberg *createCalibInfoCreator(void) const;

};
#endif
