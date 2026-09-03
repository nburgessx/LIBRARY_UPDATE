#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathPathEntity.h"
#include "AQLMathHWFuncTool.h"
#include <map>


// ID for AQLRatesHWIntegral
#define FN_HWINTEGRAL	10029
// Function name for AQLRatesHWIntegral
#define FN_HWINTEGRAL_STR	"fn_hwintegral"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of Euler-Maruyama sde integral

*/
class AQLRatesHWIntegral : public AQLRatesSDEIntegralBase
{
public:
//  LIFECYCLE
                                //======================================
	                            // Default constructor
	explicit AQLRatesHWIntegral(SDEINTEGRAL_TYPE type);
	                            // Default constructor
	explicit AQLRatesHWIntegral(SDEINTEGRAL_TYPE type, const AQLString& sdeAttrName);
								//======================================
	                            //	Copy constructor
	AQLRatesHWIntegral(const AQLRatesHWIntegral& v);
                                //======================================
	                            // Destructor
	virtual ~AQLRatesHWIntegral();
                                //======================================

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// excecute integral
	inline 	virtual void		integral(double ts, double te, 
										std::vector<AQLFunctionBase*>::const_iterator drift,										
										std::vector<std::vector<AQLFunctionBase*> >::const_iterator vol,
										DoubleArray::const_iterator	bm,
										SCALARARRAY::iterator	x_in_out,	
										unsigned int varnum
								) const;
								//======================================
				//				//set up initial value 
    //virtual void                setUpInitialVal(const AQLRatesPathElementBase& curve0, const SCALARARRAY& rate0);
    virtual void                setUpInitialVal(const SCALARARRAY& rate0);
                                //======================================
    virtual void                setUp(const AQLMathPathEntity& path);

protected:
	double						E( double T ) const;
	// tempolary variable for calculation
	mutable DoubleArray			                mVar;
    //
    mutable double                              mCumulatedVal;
    //
	const AQLMathHWFuncTool*						mpHWtool;
	//
	mutable std::map<double, double>*			mE_cache;
    //
    bool                                        is_cloned;

//protected:
};
