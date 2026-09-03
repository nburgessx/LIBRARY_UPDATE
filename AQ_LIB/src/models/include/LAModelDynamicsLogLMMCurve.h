#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAModelDynamicsLMMCurve.h"



// ID for LARatesPathElementLogLMMCurve
#define PE_LOGLMMCURVE	6 



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of path element class that represents curve object for libor market model (log type)
*/
class LARatesPathElementLogLMMCurve : public LARatesPathElementLMMCurve
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesPathElementLogLMMCurve(const DoubleArray& tenor, const DoubleArray& delta_tenor, double t = 0);
	// copy constructor
	LARatesPathElementLogLMMCurve(const LARatesPathElementLogLMMCurve& v);
	// Destructor
	virtual ~LARatesPathElementLogLMMCurve();
	// copy constructor
//	LARatesPathElementLogLMMCurve(const LARatesPathElementLogLMMCurve& v);

//  QUERY
								//======================================
								// Check pathelement for this class ID
	virtual bool                isTypeOf(pathelement_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LARatesPathElementBase*	
								clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual pathelement_t		getType() const;
	// get discount bond price
	virtual double				getP (double T) const;
	// set value
	virtual void				set (const LARatesPathElementBase& a);


private:

protected:

};

