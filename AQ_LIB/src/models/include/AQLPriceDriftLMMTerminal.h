#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceDriftLMMBase.h"


// ID for AQLPriceDriftLMMTerminal
#define FN_DRIFTLMMTERMINAL	2204
// Function name for AQLPriceDriftLMMTerminal
#define FN_DRIFTLMMTERMINAL_STR	"fn_drift_lmmterminal"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of LMM sde (terminal measure)

*/
class AQLPriceDriftLMMTerminal : public AQLPriceDriftLMMBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLPriceDriftLMMTerminal(double Q = 1.0);
	// constructor
	AQLPriceDriftLMMTerminal(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q = 1.0);	
	//	Copy constructor
	AQLPriceDriftLMMTerminal(const AQLPriceDriftLMMTerminal& v);
	// Destructor
	virtual ~AQLPriceDriftLMMTerminal();

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
	                            //==========================================
	                            // Return drift value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
	                            // Return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const;
//	OPERATION
								//======================================
								// set up this class
	virtual void				setUp(AQLMathPathEntity& path);

private:

protected:

};
