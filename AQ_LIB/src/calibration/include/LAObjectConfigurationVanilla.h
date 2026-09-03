/*! @file
    @brief  Vanilla object setup class
*/
//  2008, AlgoQuantHub.
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
#include "AQLInterpolationBase.h"
#include <map>


class AQLObjectPool;
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
	virtual void setUpPathEntity(AQLObjectPool &objPool, const AQLString *pPathName = 0) const;

protected:
	//==============================================
	// setup  valuable object
	virtual void setUpValuableEntity(AQLObjectPool &objPool) const; //{return;};
	////==============================================
	// setup  extra object
	virtual void setUpExtraEntity(AQLObjectPool &objPool) const ;
	//==============================================
	// get model name
	virtual AQLString getModelName() const { return MODEL_IRVANILLA; }
	//==============================================
	// get sde timegrid
	virtual void getSDETimeGrid(DoubleArray &timeGrid) const {timeGrid; return;};
		//==============================================
	// get sde timegrid
	virtual void getSDEIntegralTimeGrid(DoubleArray &timeGrid) const {timeGrid; return;};
	// setUpCallSpreadForFXDigitalOption
	void setUpCallSpreadForFXDigitalOption(AQLMathObjectValue &trade) const;
	// setUpCallSpreadForDigitalOption
	void setUpCallSpreadForIRDigitalOption(AQLMathObjectValue &trade) const;
	// setUpStubCoefficient
	//void setUpStubCoefficient(AQLObjectPool &objPool,  AQLMathObjectValue &trade) const;
	// setUpRenotionalFXInfo
	void setUpRenotionalFXInfo(AQLObjectPool &objPool,  AQLMathObjectValue &trade) const;
	// setUpConvexityAdjustInfo
	void setUpConvexityAdjustInfo(AQLObjectPool &objPool,  AQLMathObjectValue &trade) const;

	// setUpLiborRateMap
	void setUpLiborRateMap(AQLObjectPool &objPool) const;

	// set up properties of SpotRateRatioMethod for stub rates calculation
	void setUpSpotRateRatioMethod(AQLObjectPool &objPool,  AQLMathObjectValue &trade) const;

	bool setUpMarkovFunctionalParameter(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;
	
	mutable std::map<AQLString, AQLInterpolationBase*> mLiborRateMap;
	mutable std::map<AQLString, DoubleVector> mLiborGridTermMap;
};
#endif
