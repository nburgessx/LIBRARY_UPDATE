#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "LARatesCovariance.h"
#include "AQLCoreTemplateType.h"
#include "AQLFunction.h"

#include "LAMathHWFuncMR.h"
#include "LAModelDynamicsHW1FCurve.h"
#include "LAMathHWFuncTool.h"
#include <vector>
#include <map>

//
//------------------------------ LAPriceDriftHW ------------------------------
//
//+++++ definition +++++
// ID for LAPriceDriftHW
#define FN_DRIFTHW 10035
// Function name for LAPriceDriftLMMSpot
#define FN_DRIFTIR_STR	"fn_drifthw"

//+++++ PROTOTYPE +++++
class LAMathHWFuncMR;
class LAMathHWFuncSigma;

/*! 
    @brief Declaration of drift function of SR

*/

class LAPriceDriftHW : public LAMathDriftFuncBase
{
public:
//  LIFECYCLE
                                //======================================
	                            // constructor
    explicit 
    LAPriceDriftHW();

    LAPriceDriftHW( const AQLString& sdeAttrName);
                                //======================================
    	                        // copy constructor
	LAPriceDriftHW(const LAPriceDriftHW& rhs);
                                //======================================
                                // Destructor
	virtual ~LAPriceDriftHW();
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
								// get function class of LAMathHWFunc 
	const LAMathHWFuncMR&			getHWMR(void) const {return *mpHWtool->getHWMR();}
								// get function class of LAMathHWFunSigma
	const LAMathHWFuncSigma&		getHWSigma(void) const {return  *mpHWtool->getHWSigma();}
	
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
    const LARatesPathElementCurve* mpInitialCurve;
    bool is_cloned;
    //
	LAMathHWFuncTool* mpHWtool;
	LAMathHWFuncTool* mpHWtool2;
	LAMathHWFuncTool* mpHWtool3;
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

	AQLMathFunction<LAPriceDriftHW> mFuncExpInt_a_inv;
    AQLMathFunction<LAPriceDriftHW> mFuncExpInt_a_sigma_SQ;
    AQLMathFunction<LAPriceDriftHW> mFuncExpInt_a_sigma_SQ_B;

};
