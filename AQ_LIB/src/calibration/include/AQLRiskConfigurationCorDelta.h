/*! @file
    @brief Correlation Delta setup class
*/
#ifndef AQLRiskConfigurationCorDelta_h
#define AQLRiskConfigurationCorDelta_h

#include <map>

#include "AQLRiskConfiguration.h"

class AQLRiskConfigurationCorDelta : public AQLRiskConfiguration
{
public:
	// constructor
	explicit AQLRiskConfigurationCorDelta(void);
	// destructor
	virtual ~AQLRiskConfigurationCorDelta(void);

private:
	// copy constructor
	AQLRiskConfigurationCorDelta(const AQLRiskConfigurationCorDelta &rhs);
	AQLRiskConfigurationCorDelta &operator=(const AQLRiskConfigurationCorDelta &rhs);

protected:
	//==============================================
	// create risk object
	virtual std::vector<std::pair<AQLString, std::vector<AQLObject *> > > createRiskEntity(AQLObjectPool &objPool) const;
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& key) const;
	// setup
	virtual  void setUpRiskOutputCurrency(const AQLString &ccy, AQLObject &e) const;
	//==============================================
	// create scenario1 object 
	virtual  std::vector<AQLObject *> createScenario1Entity(const AQLString &key, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<AQLObject *> createScenario2Entity(const AQLString &key, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get target names 
	virtual  AQLString getTargetNames(const AQLString &key, AQLDataInstance &dataInstance)  const;
	//==============================================
	// get extra target names1
	virtual  AQLStringVector getExtraTargetNames1(const AQLString &key, AQLDataInstance &dataInstance) const;
	//==============================================
	// create extraScenario1 object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenario1Entity(const AQLString &key, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &key) const;
	//==============================================
	// isgridsensitivity
	virtual  bool isGridSensitivity(const AQLString &key) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &key) const;
	//==============================================
	// return risk name
	virtual  AQLString  getRiskName(void) const;
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getGridTerm(const AQLString &key) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<AQLString> getBucketGridTerm(const AQLString &key) const;
	//==============================================
	// get bump direction
	virtual  AQLString getBumpDirection(const AQLString &key) const;
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const AQLString &ccy) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const AQLString &key, SCENARIONUM scenarioNum) const;

private:
	//==============================================
	// @Description: Put a BumpedCor object for the specified key
	//                into mBumpedCors
	//  @param key [in] The key like "JPY-USD" or "AUD-USD/JPY"
	//  @param objPool [in] The object pool
	void putBumpedCor(const AQLString &key, AQLObjectPool &objPool) const;

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
	mutable std::map<AQLString, BumpedCor> mBumpedCors;
};

#endif
