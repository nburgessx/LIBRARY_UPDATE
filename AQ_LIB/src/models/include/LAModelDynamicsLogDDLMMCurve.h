#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAModelDynamicsLMMCurve.h"



// ID for LARatesPathElementLogDDLMMCurve
#define PE_LOGDDLMMCURVE	7 



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of path element class that represents curve object for libor market model (log type)
*/
class LARatesPathElementLogDDLMMCurve : public LARatesPathElementLMMCurve
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesPathElementLogDDLMMCurve(double s, const DoubleArray& tenor, const DoubleArray& delta_tenor, double t = 0);
	// copy constructor
	LARatesPathElementLogDDLMMCurve(const LARatesPathElementLogDDLMMCurve& v);
	// Destructor
	virtual ~LARatesPathElementLogDDLMMCurve();

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
	// equal operator
    /*!
	    @param[in] a source object
        @return copy object
    */
    virtual	LARatesPathElementLogDDLMMCurve&
								operator = (const LARatesPathElementLogDDLMMCurve& a);
	// get discount bond price
	virtual double				getP (double T) const;
	// set value
	virtual void				set (const LARatesPathElementBase& a);

private:

protected:
	double						mSpread;		// spread
};
