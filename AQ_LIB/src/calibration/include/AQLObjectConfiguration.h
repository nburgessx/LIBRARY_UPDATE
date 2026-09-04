/*! @file
    @brief  object setup class
*/
#ifndef AQLObjectConfiguration_h
#define AQLObjectConfiguration_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLDataMultiReference.h"


class AQLDataInstance;
class AQLObjectPool;
class AQLMathObjectValue;
class AQLStaticData;
class AQLObject;
//===================== Class Declare AQLObjectConfiguration==================================
/*! 
    @brief Object setup class
	
	this class is abstract

*/
class AQLObjectConfiguration
{
public:
	// constructor
	explicit AQLObjectConfiguration(void);
	// destructor
	virtual ~AQLObjectConfiguration(void);
	// copy constructor
	AQLObjectConfiguration(const AQLObjectConfiguration &rhs);
	AQLObjectConfiguration &operator=(const AQLObjectConfiguration &rhs);

	//==============================================
	// setup object
	virtual void setUpEntityes(AQLDataInstance &dataInstance) const;
	//==============================================
	// setup  pathentity
	virtual void setUpPathEntity(AQLObjectPool &objPool, const AQLString *pPathName = 0) const;

	//==============================================
	// setUpExtraLibor
	void setUpExtraLibor(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;
	//==============================================
	// setup future fixing object
	virtual void setUpFutureFixingEntity(AQLObjectPool &objPool) const;
	//==============================================
	// setup future fixing function
	void setUpFutureFixingFunc(AQLObjectPool &objPool,  AQLMathObjectValue &trade) const;
	//==============================================
	// get fixed rate from property
	double getFixedRate(const AQLString &key, const AQLString &index) const;
	//==============================================
	// setUpDigitalCallSpreadCoupon
	void setUpDigitalCallSpreadCoupon(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;
	//==============================================
	// setUpDiscountCurve
	void setUpDiscountCurve(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;
	//==============================================
	// setUpFundingSpread
	void setUpFundingSpread(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;
	//==============================================
	// setup  fx object
	virtual void setUpFXEntity(AQLObjectPool &objPool) const;
	//==============================================
	// setup  risk info
	void setUpRiskInfo(AQLDataInstance &dataInstance) const;
	//==============================================
	// setUpCoTermOfLSMCIndex
	void setUpCoTermOfLSMCIndex(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;
	//==============================================
	// setUpDirtyPrice
	void setUpDirtyPrice(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;
	//==============================================
	// setUpScaleShftOfLSMC
	void setUpScaleShftOfLSMC(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;

protected :
	//==============================================
	// setup  valuable object
	virtual void setUpValuableEntity(AQLObjectPool &objPool) const;
	//==============================================
	// setup  extra object
	virtual void setUpExtraEntity(AQLObjectPool &objPool) const;
	//==============================================
	// get model name
	virtual AQLString getModelName() const = 0;
	//==============================================
	// get sde timegrid
	virtual void getSDETimeGrid(DoubleArray &timeGrid) const = 0;
		//==============================================
	// get sde timegrid
	virtual void getSDEIntegralTimeGrid(DoubleArray &timeGrid) const = 0;
	//==============================================
	// setUpRecalcTrade
	virtual void setUpRecalcTrade(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;
	//==============================================
	// setup forward interpolation
	virtual void setUpFwdInterpolation(AQLObjectPool &objPool) const;
	//==============================================
	// setUpAsOfDateForTradeEntity
	void setUpAsOfDateForTradeEntity(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;
	//==============================================
	// setup  valuable object
	void setUpRiskInfo(std::vector<const AQLObject *> &riskEVec, AQLObject &e) const;
	//==============================================
	// setUpPastIndexRate
	void setUpPastIndexRate(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;
	//==============================================
	// setUpCompoundingFixedRates
	void setUpCompoundingFixedRates(const AQLDate& asOfDate, AQLObjectHolder& ehTrade) const;
	//==============================================
	// setUpScaleShftOfLSMC
	void setUpSVDToleranceForLSMC(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;
	//==============================================
	// setUpBondFundingLeg
	void setUpBondFundingLeg(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;

	AQLStaticData *mpStaticData; // property accessor
	mutable std::map<AQLString, bool> mIsFwdInterMap;
	mutable std::map<AQLString, AQLString> mFwdInterMap;
	mutable std::map<AQLString, double> mFdSpdMap;
	mutable std::map<AQLString, double> mDirtyPriceMap;

    AQLString createCoTermTenorString(const AQLDate& today, const AQLDate& matdate, const AQLString& freq) const;

private :
	//==============================================
	// setup forward interpolation
	void setUpFwdInterpolation(AQLMathObjectValue &trade) const;
	//==============================================
	// setUpPastCouponPayOffs
	void setUpPastCouponPayOffs(AQLObjectPool &objPool, AQLMathObjectValue &trade) const;
	//==============================================
	// setUpRangeAccrueFixedRates
	void setUpRangeAccrueFixedRates(const AQLDate& asOfDate, AQLObjectHolder& ehTrade) const;
	// setUpIndexFixedRates
	void setUpIndexFixedRates(const AQLStringMatrix& ratesMatrix, AQLDataMultiReference& indexs) const;
	// setUpIndexFixedRates for ovservation operator
	void setUpIndexFixedRates_ObsvOpr(const AQLStringMatrix& ratesMatrix, AQLDataMultiReference& indexs) const;
	// setUpBoundaryIndexFixedRates
	void setUpBoundaryIndexFixedRate(const AQLStringMatrix& ratesMatrix, AQLDataMultiReference& indexs) const;
};
#endif
