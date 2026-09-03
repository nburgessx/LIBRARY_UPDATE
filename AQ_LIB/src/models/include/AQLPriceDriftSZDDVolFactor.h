#pragma once

#ifdef __GNUG__
#pragma interface
#endif

//+++++ include +++++
#include "AQLMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "AQLRatesCovariance.h"
#include "AQLCoreTemplateType.h"
#include "AQLFunction.h"

#include "AQLMathHWFuncMR.h"
#include "AQLModelDynamicsHW1FCurve.h"
#include "AQLMathHWFuncTool.h"
#include "AQLPriceDriftHW.h"
#include <vector>
#include <map>

//
//------------------------------ AQLPriceDriftSZDDVolFactor ------------------------------
//
//+++++ definition +++++
// ID for AQLPriceDriftSZDDVolFactor
#define FN_DRIFTSZDDVOLAFACTOR 10065
// Function name for AQLPriceDriftLMMSpot
#define FN_DRIFTSZDDVOLAFACTOR_STR	"fn_driftszddvolafactor"

//+++++ PROTOTYPE +++++
class AQLMathHWFuncMR;
class AQLMathHWFuncSigma;

/*! 
    @brief Declaration of drift function of SR

*/

class AQLPriceDriftSZDDVolFactor : public AQLPriceDriftHW
{
public:
//  LIFECYCLE
                                //======================================
	                            // constructor
    explicit 
    AQLPriceDriftSZDDVolFactor();

    AQLPriceDriftSZDDVolFactor( const AQLString& sdeAttrName);
                                //======================================
    	                        // copy constructor
	AQLPriceDriftSZDDVolFactor(const AQLPriceDriftSZDDVolFactor& rhs);
                                //======================================
                                // Destructor
	virtual ~AQLPriceDriftSZDDVolFactor();
                                //======================================
//  QUERY
								//======================================
								// Check function for this class ID
	bool                        isTypeOf(function_t id) const;
								//======================================
                                //
    AQLCoreFunctionBase*		        clone() const;// %%% COVARIANT RETURN %%%
                                //======================================
                                // Return this class ID
	function_t			        getType() const;
								//======================================
								// return string representaion
    AQLString			        convertToString(void) const;
                                //======================================
                                // operator()
    virtual double              operator()(const DoubleArray& x) const;
                                //======================================
 // OPERATION
								//======================================
								// transform from string representaion
    void				        convertFromString(const AQLString& str);
								//======================================
								// set up this class
	void				        setUp(AQLMathPathEntity& path);
								//======================================
								// get alpha
	virtual double				getAlpha(const double t) const;
                                //======================================
protected:
	double                      getExpInt_a_theta(const double t) const;

	double                      mInitialValue;
	const AQL1DDataSet*			mpThetaFunc;
	mutable std::map<double, double> mCacheFuncExpInt_a_theta;

private:
	AQLMathFunction<AQLPriceDriftSZDDVolFactor> mFuncExpInt_a_theta;

};

