/*! @file
    @brief  Vanilla object setup class
*/
//  2008, Mizuho International London.
#ifndef LAObjectConfigurationVanilla_h
#define LAObjectConfigurationVanilla_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAObjectConfigurationVanilla.h
//
//  DESCRIPTION :        Vanilla object setup class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif


#include "LAObjectConfiguration.h"
#include "LADefinitions.h"
#include "LAInterpolationBase.h"
#include <map>


class LAObjectPool;
class LAStaticData;
//===================== Class Declare LAObjectConfigurationVanilla==================================
/*! 
    @brief Object setup class
	
	object setup for Vanilla

*/
class LAObjectConfigurationVanilla : public LAObjectConfiguration
{
public:
	// constructor
	explicit LAObjectConfigurationVanilla(void);
	// destructor
	virtual ~LAObjectConfigurationVanilla(void);
	// copy constructor
	LAObjectConfigurationVanilla(const LAObjectConfigurationVanilla &rhs);
	LAObjectConfigurationVanilla &operator=(const LAObjectConfigurationVanilla &rhs);
	// setup  pathentity
	virtual void setUpPathEntity(LAObjectPool &objPool, const LAString *pPathName = 0) const;

protected:
	//==============================================
	// setup  valuable object
	virtual void setUpValuableEntity(LAObjectPool &objPool) const; //{return;};
	////==============================================
	// setup  extra object
	virtual void setUpExtraEntity(LAObjectPool &objPool) const ;
	//==============================================
	// get model name
	virtual LAString getModelName() const { return MODEL_IRVANILLA; }
	//==============================================
	// get sde timegrid
	virtual void getSDETimeGrid(DoubleArray &timeGrid) const {timeGrid; return;};
		//==============================================
	// get sde timegrid
	virtual void getSDEIntegralTimeGrid(DoubleArray &timeGrid) const {timeGrid; return;};
	// setUpCallSpreadForFXDigitalOption
	void setUpCallSpreadForFXDigitalOption(LAMathObjectValue &trade) const;
	// setUpCallSpreadForDigitalOption
	void setUpCallSpreadForIRDigitalOption(LAMathObjectValue &trade) const;
	// setUpStubCoefficient
	//void setUpStubCoefficient(LAObjectPool &objPool,  LAMathObjectValue &trade) const;
	// setUpRenotionalFXInfo
	void setUpRenotionalFXInfo(LAObjectPool &objPool,  LAMathObjectValue &trade) const;
	// setUpConvexityAdjustInfo
	void setUpConvexityAdjustInfo(LAObjectPool &objPool,  LAMathObjectValue &trade) const;

	// setUpLiborRateMap
	void setUpLiborRateMap(LAObjectPool &objPool) const;

	// set up properties of SpotRateRatioMethod for stub rates calculation
	void setUpSpotRateRatioMethod(LAObjectPool &objPool,  LAMathObjectValue &trade) const;

	bool setUpMarkovFunctionalParameter(LAObjectPool &objPool, LAMathObjectValue &trade) const;
	
	mutable std::map<LAString, LAInterpolationBase*> mLiborRateMap;
	mutable std::map<LAString, DoubleVector> mLiborGridTermMap;
};
#endif
