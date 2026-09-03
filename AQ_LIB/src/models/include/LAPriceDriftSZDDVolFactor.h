#pragma once

#ifdef __GNUG__
#pragma interface
#endif

//+++++ include +++++
#include "LAMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "LARatesCovariance.h"
#include "AQLCoreTemplateType.h"
#include "AQLFunction.h"

#include "LAMathHWFuncMR.h"
#include "LAModelDynamicsHW1FCurve.h"
#include "LAMathHWFuncTool.h"
#include "LAPriceDriftHW.h"
#include <vector>
#include <map>

//
//------------------------------ LAPriceDriftSZDDVolFactor ------------------------------
//
//+++++ definition +++++
// ID for LAPriceDriftSZDDVolFactor
#define FN_DRIFTSZDDVOLAFACTOR 10065
// Function name for LAPriceDriftLMMSpot
#define FN_DRIFTSZDDVOLAFACTOR_STR	"fn_driftszddvolafactor"

//+++++ PROTOTYPE +++++
class LAMathHWFuncMR;
class LAMathHWFuncSigma;

/*! 
    @brief Declaration of drift function of SR

*/

class LAPriceDriftSZDDVolFactor : public LAPriceDriftHW
{
public:
//  LIFECYCLE
                                //======================================
	                            // constructor
    explicit 
    LAPriceDriftSZDDVolFactor();

    LAPriceDriftSZDDVolFactor( const AQLString& sdeAttrName);
                                //======================================
    	                        // copy constructor
	LAPriceDriftSZDDVolFactor(const LAPriceDriftSZDDVolFactor& rhs);
                                //======================================
                                // Destructor
	virtual ~LAPriceDriftSZDDVolFactor();
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
	void				        setUp(LAMathPathEntity& path);
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
	AQLMathFunction<LAPriceDriftSZDDVolFactor> mFuncExpInt_a_theta;

};

