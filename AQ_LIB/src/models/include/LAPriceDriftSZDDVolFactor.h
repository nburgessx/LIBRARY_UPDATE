#pragma once

#ifdef __GNUG__
#pragma interface
#endif

//+++++ include +++++
#include "LAMathDriftFuncBase.h"
#include "LACoreAppError.h"
#include "LARatesCovariance.h"
#include "LACoreTemplateType.h"
#include "LAFunction.h"

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

    LAPriceDriftSZDDVolFactor( const LAString& sdeAttrName);
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
    LACoreFunctionBase*		        clone() const;// %%% COVARIANT RETURN %%%
                                //======================================
                                // Return this class ID
	function_t			        getType() const;
								//======================================
								// return string representaion
    LAString			        convertToString(void) const;
                                //======================================
                                // operator()
    virtual double              operator()(const DoubleArray& x) const;
                                //======================================
 //Å@OPERATION
								//======================================
								// transform from string representaion
    void				        convertFromString(const LAString& str);
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
	const LA1DDataSet*			mpThetaFunc;
	mutable std::map<double, double> mCacheFuncExpInt_a_theta;

private:
	LAMathFunction<LAPriceDriftSZDDVolFactor> mFuncExpInt_a_theta;

};

