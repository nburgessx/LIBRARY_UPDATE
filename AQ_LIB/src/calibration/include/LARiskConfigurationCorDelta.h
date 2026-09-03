/*! @file
    @brief Correlation Delta setup class
*/
#ifndef LARiskConfigurationCorDelta_h
#define LARiskConfigurationCorDelta_h

#include <map>

#include "LARiskConfiguration.h"

class LARiskConfigurationCorDelta : public LARiskConfiguration
{
public:
	// constructor
	explicit LARiskConfigurationCorDelta(void);
	// destructor
	virtual ~LARiskConfigurationCorDelta(void);

private:
	// copy constructor
	LARiskConfigurationCorDelta(const LARiskConfigurationCorDelta &rhs);
	LARiskConfigurationCorDelta &operator=(const LARiskConfigurationCorDelta &rhs);

protected:
	//==============================================
	// create risk object
	virtual std::vector<std::pair<LAString, std::vector<LAObject *> > > createRiskEntity(LAObjectPool &objPool) const;
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& key) const;
	// setup
	virtual  void setUpRiskOutputCurrency(const LAString &ccy, LAObject &e) const;
	//==============================================
	// create scenario1 object 
	virtual  std::vector<LAObject *> createScenario1Entity(const LAString &key, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<LAObject *> createScenario2Entity(const LAString &key, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get target names 
	virtual  LAString getTargetNames(const LAString &key, LADataInstance &dataInstance)  const;
	//==============================================
	// get extra target names1
	virtual  LAStringVector getExtraTargetNames1(const LAString &key, LADataInstance &dataInstance) const;
	//==============================================
	// create extraScenario1 object 
	virtual  std::vector<std::vector<LAObject *> > createExtraScenario1Entity(const LAString &key, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &key) const;
	//==============================================
	// isgridsensitivity
	virtual  bool isGridSensitivity(const LAString &key) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &key) const;
	//==============================================
	// return risk name
	virtual  LAString  getRiskName(void) const;
	//==============================================
	// get grid term
	virtual std::vector<LAString> getGridTerm(const LAString &key) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<LAString> getBucketGridTerm(const LAString &key) const;
	//==============================================
	// get bump direction
	virtual  LAString getBumpDirection(const LAString &key) const;
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const LAString &ccy) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const LAString &key, SCENARIONUM scenarioNum) const;

private:
	//==============================================
	// @Description: Put a BumpedCor object for the specified key
	//                into mBumpedCors
	//  @param key [in] The key like "JPY-USD" or "AUD-USD/JPY"
	//  @param objPool [in] The object pool
	void putBumpedCor(const LAString &key, LAObjectPool &objPool) const;

	//==============================================
	// @Description: A struct whose object represents a bumped correlation matrix
	struct BumpedCor
	{
		//==============================================
		// @Description: A bumped correlation matrix whose rows and columns are
		//                arranged in the same order as simulation SDE currencies
		DoubleMatrix corMtxForPathEntity;

		//==============================================
		// @Description: true if corMtxForPathEntity is produced with upshift,
		//                false otherwise (that is, downshift)
		bool upshift;
	};

	//==============================================
	// @Description: A associative aaray which maps a key (like "JPY-USD") to
	//                its corresponding bumped correlation matrix
	mutable std::map<LAString, BumpedCor> mBumpedCors;
};

#endif
