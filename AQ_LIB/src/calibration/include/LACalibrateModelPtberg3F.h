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

#include "LAString.h"
#include "LACalibrateModelPtberg.h"


class LADataInstance;
class LAMathVolatility;
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
	virtual  void setDrift(const LAString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const LAString &fx) const;
private:
	//==============================================
	// set volatility object
	void setUpVolEntity(const LAString &fx, LAMathVolatility &vol) const;
	//==============================================
	// create calibration info
	virtual LACalibrationParametersPtberg *createCalibInfoCreator(void) const;

};
#endif
