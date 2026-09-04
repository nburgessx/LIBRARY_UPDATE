#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "AQLRatesCovariance.h"
#include "AQLCoreTemplateType.h"
#include "AQLFunction.h"

#include "AQLMathHWFuncMR.h"
#include "AQLModelDynamicsHW1FCurve.h"
#include "AQLMathHWFuncTool.h"
#include <vector>
#include <map>

//
//------------------------------ AQLPriceDriftHW ------------------------------
//
//+++++ definition +++++
// ID for AQLPriceDriftHW
#define FN_DRIFTHW 10035
// Function name for AQLPriceDriftLMMSpot
#define FN_DRIFTIR_STR	"fn_drifthw"

//+++++ PROTOTYPE +++++
class AQLMathHWFuncMR;
class AQLMathHWFuncSigma;

/*! 
    @brief Declaration of drift function of SR

*/

class AQLPriceDriftHW : public AQLMathDriftFuncBase
{
public:
//  LIFECYCLE
                                //======================================
	                            // constructor
    explicit 
    AQLPriceDriftHW();

    AQLPriceDriftHW( const AQLString& sdeAttrName);
                                //======================================
    	                        // copy constructor
	AQLPriceDriftHW(const AQLPriceDriftHW& rhs);
                                //======================================
                                // Destructor
	virtual ~AQLPriceDriftHW();
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
								// get function class of AQLMathHWFunc 
	const AQLMathHWFuncMR&			getHWMR(void) const {return *mpHWtool->getHWMR();}
								// get function class of AQLMathHWFunSigma
	const AQLMathHWFuncSigma&		getHWSigma(void) const {return  *mpHWtool->getHWSigma();}
	
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
	//furuya
									// get Fwd
	double				        getFwd(const double t) const;
                                //======================================
protected:

	double getExpInt_a_inv(const  double t) const;
    double getExpInt_a_sigma_SQ(const double t) const;
    double getExpInt_a_sigma_SQ_B(const double t) const;

	// data name of ir sde
    AQLString    mSDEAttrName;	
    // tempolary variable
	mutable	unsigned int    mPos_old;			
    // tempolary variable
	mutable DoubleArray mX;				
    //
    const AQLRatesPathElementCurve* mpInitialCurve;
    bool is_cloned;
    //
	AQLMathHWFuncTool* mpHWtool;
	AQLMathHWFuncTool* mpHWtool2;
	AQLMathHWFuncTool* mpHWtool3;
	AQLGaussLegendre mGL;

	const DoubleVector *mpSimuTimeGrid;

	mutable DoubleVector *mpCache1;		// cache
	mutable DoubleVector *mpCache2;		// cache
	mutable DoubleVector *mpCache3;		// cache
	mutable DoubleVector *mpIntegrate_cache;
	mutable DoubleVector *mpTimeGrid;

	mutable std::map<double, double> mCacheAlpha;	    // cache
	mutable std::map<double, double> mCacheFuncExpInt_a_inv;		// cache
	mutable std::map<double, double> mCacheFuncExpInt_a_sigma_SQ;		// cache
	mutable std::map<double, double> mCacheFuncExpInt_a_sigma_SQ_B;		// cache

private:

	AQLMathFunction<AQLPriceDriftHW> mFuncExpInt_a_inv;
    AQLMathFunction<AQLPriceDriftHW> mFuncExpInt_a_sigma_SQ;
    AQLMathFunction<AQLPriceDriftHW> mFuncExpInt_a_sigma_SQ_B;

};
