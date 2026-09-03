#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LAMathPathEntity.h"
#include "LAMathHWFuncTool.h"
#include <map>


// ID for LARatesHWIntegral
#define FN_HWINTEGRAL	10029
// Function name for LARatesHWIntegral
#define FN_HWINTEGRAL_STR	"fn_hwintegral"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of Euler-Maruyama sde integral

*/
class LARatesHWIntegral : public LARatesSDEIntegralBase
{
public:
//  LIFECYCLE
                                //======================================
	                            // Default constructor
	explicit LARatesHWIntegral(SDEINTEGRAL_TYPE type);
	                            // Default constructor
	explicit LARatesHWIntegral(SDEINTEGRAL_TYPE type, const AQLString& sdeAttrName);
								//======================================
	                            //	Copy constructor
	LARatesHWIntegral(const LARatesHWIntegral& v);
                                //======================================
	                            // Destructor
	virtual ~LARatesHWIntegral();
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
    //virtual void                setUpInitialVal(const LARatesPathElementBase& curve0, const SCALARARRAY& rate0);
    virtual void                setUpInitialVal(const SCALARARRAY& rate0);
                                //======================================
    virtual void                setUp(const LAMathPathEntity& path);

protected:
	double						E( double T ) const;
	// tempolary variable for calculation
	mutable DoubleArray			                mVar;
    //
    mutable double                              mCumulatedVal;
    //
	const LAMathHWFuncTool*						mpHWtool;
	//
	mutable std::map<double, double>*			mE_cache;
    //
    bool                                        is_cloned;

//protected:
};
