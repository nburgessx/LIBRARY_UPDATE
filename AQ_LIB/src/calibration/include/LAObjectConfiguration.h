/*! @file
    @brief  object setup class
*/
//  2007, Mizuho International London.
#ifndef LAObjectConfiguration_h
#define LAObjectConfiguration_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAObjectConfiguration.h
//
//  DESCRIPTION :        Object setup class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreTemplateType.h"
#include "LADataMultiReference.h"


class LADataInstance;
class LAObjectPool;
class LAMathObjectValue;
class LAStaticData;
class LAObject;
//===================== Class Declare LAObjectConfiguration==================================
/*! 
    @brief Object setup class
	
	this class is abstract

*/
class LAObjectConfiguration
{
public:
	// constructor
	explicit LAObjectConfiguration(void);
	// destructor
	virtual ~LAObjectConfiguration(void);
	// copy constructor
	LAObjectConfiguration(const LAObjectConfiguration &rhs);
	LAObjectConfiguration &operator=(const LAObjectConfiguration &rhs);

	//==============================================
	// setup object
	virtual void setUpEntityes(LADataInstance &dataInstance) const;
	//==============================================
	// setup  pathentity
	virtual void setUpPathEntity(LAObjectPool &objPool, const LAString *pPathName = 0) const;

	//==============================================
	// setUpExtraLibor
	void setUpExtraLibor(LAObjectPool &objPool, LAMathObjectValue &trade) const;
	//==============================================
	// setup future fixing object
	virtual void setUpFutureFixingEntity(LAObjectPool &objPool) const;
	//==============================================
	// setup future fixing function
	void setUpFutureFixingFunc(LAObjectPool &objPool,  LAMathObjectValue &trade) const;
	//==============================================
	// get fixed rate from property
	double getFixedRate(const LAString &key, const LAString &index) const;
	//==============================================
	// setUpDigitalCallSpreadCoupon
	void setUpDigitalCallSpreadCoupon(LAObjectPool &objPool, LAMathObjectValue &trade) const;
	//==============================================
	// setUpDiscountCurve
	void setUpDiscountCurve(LAObjectPool &objPool, LAMathObjectValue &trade) const;
	//==============================================
	// setUpFundingSpread
	void setUpFundingSpread(LAObjectPool &objPool, LAMathObjectValue &trade) const;
	//==============================================
	// setup  fx object
	virtual void setUpFXEntity(LAObjectPool &objPool) const;
	//==============================================
	// setup  risk info
	void setUpRiskInfo(LADataInstance &dataInstance) const;
	//==============================================
	// setUpCoTermOfLSMCIndex
	void setUpCoTermOfLSMCIndex(LAObjectPool &objPool, LAMathObjectValue &trade) const;
	//==============================================
	// setUpDirtyPrice
	void setUpDirtyPrice(LAObjectPool &objPool, LAMathObjectValue &trade) const;
	//==============================================
	// setUpScaleShftOfLSMC
	void setUpScaleShftOfLSMC(LAObjectPool &objPool, LAMathObjectValue &trade) const;

protected :
	//==============================================
	// setup  valuable object
	virtual void setUpValuableEntity(LAObjectPool &objPool) const;
	//==============================================
	// setup  extra object
	virtual void setUpExtraEntity(LAObjectPool &objPool) const;
	//==============================================
	// get model name
	virtual LAString getModelName() const = 0;
	//==============================================
	// get sde timegrid
	virtual void getSDETimeGrid(DoubleArray &timeGrid) const = 0;
		//==============================================
	// get sde timegrid
	virtual void getSDEIntegralTimeGrid(DoubleArray &timeGrid) const = 0;
	//==============================================
	// setUpRecalcTrade
	virtual void setUpRecalcTrade(LAObjectPool &objPool, LAMathObjectValue &trade) const;
	//==============================================
	// setup forward interpolation
	virtual void setUpFwdInterpolation(LAObjectPool &objPool) const;
	//==============================================
	// setUpAsOfDateForTradeEntity
	void setUpAsOfDateForTradeEntity(LAObjectPool &objPool, LAMathObjectValue &trade) const;
	//==============================================
	// setup  valuable object
	void setUpRiskInfo(std::vector<const LAObject *> &riskEVec, LAObject &e) const;
	//==============================================
	// setUpPastIndexRate
	void setUpPastIndexRate(LAObjectPool &objPool, LAMathObjectValue &trade) const;
	//==============================================
	// setUpCompoundingFixedRates
	void setUpCompoundingFixedRates(const LADate& asOfDate, LAObjectHolder& ehTrade) const;
	//==============================================
	// setUpScaleShftOfLSMC
	void setUpSVDToleranceForLSMC(LAObjectPool &objPool, LAMathObjectValue &trade) const;
	//==============================================
	// setUpBondFundingLeg
	void setUpBondFundingLeg(LAObjectPool &objPool, LAMathObjectValue &trade) const;

	LAStaticData *mpStaticData; // property accessor
	mutable std::map<LAString, bool> mIsFwdInterMap;
	mutable std::map<LAString, LAString> mFwdInterMap;
	mutable std::map<LAString, double> mFdSpdMap;
	mutable std::map<LAString, double> mDirtyPriceMap;

    LAString createCoTermTenorString(const LADate& today, const LADate& matdate, const LAString& freq) const;

private :
	//==============================================
	// setup forward interpolation
	void setUpFwdInterpolation(LAMathObjectValue &trade) const;
	//==============================================
	// setUpPastCouponPayOffs
	void setUpPastCouponPayOffs(LAObjectPool &objPool, LAMathObjectValue &trade) const;
	//==============================================
	// setUpRangeAccrueFixedRates
	void setUpRangeAccrueFixedRates(const LADate& asOfDate, LAObjectHolder& ehTrade) const;
	// setUpIndexFixedRates
	void setUpIndexFixedRates(const LAStringMatrix& ratesMatrix, LADataMultiReference& indexs) const;
	// setUpIndexFixedRates for ovservation operator
	void setUpIndexFixedRates_ObsvOpr(const LAStringMatrix& ratesMatrix, LADataMultiReference& indexs) const;
	// setUpBoundaryIndexFixedRates
	void setUpBoundaryIndexFixedRate(const LAStringMatrix& ratesMatrix, LADataMultiReference& indexs) const;
};
#endif
