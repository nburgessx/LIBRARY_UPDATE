#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLModelDynamicsLMMCurve.h"



// ID for AQLRatesPathElementLogLMMCurve
#define PE_LOGLMMCURVE	6 



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of path element class that represents curve object for libor market model (log type)
*/
class AQLRatesPathElementLogLMMCurve : public AQLRatesPathElementLMMCurve
{
public:
//  LIFECYCLE
	// Default constructor
	AQLRatesPathElementLogLMMCurve(const DoubleArray& tenor, const DoubleArray& delta_tenor, double t = 0);
	// copy constructor
	AQLRatesPathElementLogLMMCurve(const AQLRatesPathElementLogLMMCurve& v);
	// Destructor
	virtual ~AQLRatesPathElementLogLMMCurve();
	// copy constructor
//	AQLRatesPathElementLogLMMCurve(const AQLRatesPathElementLogLMMCurve& v);

//  QUERY
								//======================================
								// Check pathelement for this class ID
	virtual bool                isTypeOf(pathelement_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLRatesPathElementBase*	
								clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual pathelement_t		getType() const;
	// get discount bond price
	virtual double				getP (double T) const;
	// set value
	virtual void				set (const AQLRatesPathElementBase& a);


private:

protected:

};

