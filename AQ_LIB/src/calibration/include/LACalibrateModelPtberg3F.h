/*! @file
    @brief Ptberg sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef LACalibrateModelPtberg3F_h
#define LACalibrateModelPtberg3F_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelPtberg3F.h
//
//  DESCRIPTION :       Ptberg SDE generator 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "LACalibrateModelPtberg.h"


class AQLDataInstance;
class AQLMathVolatility;
class LACalibrationParametersPtberg;

//===================== Class Declare LACalibrateModelPtberg3F==================================
/*! 
    @brief Ptberg SDE generator 
	

*/
class LACalibrateModelPtberg3F : public LACalibrateModelPtberg
{

public:
	// constructor
	explicit LACalibrateModelPtberg3F();
	// destructor
	virtual ~LACalibrateModelPtberg3F(void);
	// copy constructor
	LACalibrateModelPtberg3F(const LACalibrateModelPtberg3F &rhs);
	LACalibrateModelPtberg3F &operator=(const LACalibrateModelPtberg3F &rhs);
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
	virtual LACalibrationParametersPtberg *createCalibInfoCreator(void) const;

};
#endif
