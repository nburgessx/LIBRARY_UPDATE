#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceDriftLMMBase.h"


// ID for LAPriceDriftLMMTerminal
#define FN_DRIFTLMMTERMINAL	2204
// Function name for LAPriceDriftLMMTerminal
#define FN_DRIFTLMMTERMINAL_STR	"fn_drift_lmmterminal"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of LMM sde (terminal measure)

*/
class LAPriceDriftLMMTerminal : public LAPriceDriftLMMBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAPriceDriftLMMTerminal(double Q = 1.0);
	// constructor
	LAPriceDriftLMMTerminal(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q = 1.0);	
	//	Copy constructor
	LAPriceDriftLMMTerminal(const LAPriceDriftLMMTerminal& v);
	// Destructor
	virtual ~LAPriceDriftLMMTerminal();

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
	virtual void				setUp(LAMathPathEntity& path);

private:

protected:

};
