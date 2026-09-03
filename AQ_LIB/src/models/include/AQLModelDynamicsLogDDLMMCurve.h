#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLModelDynamicsLMMCurve.h"



// ID for AQLRatesPathElementLogDDLMMCurve
#define PE_LOGDDLMMCURVE	7 



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of path element class that represents curve object for libor market model (log type)
*/
class AQLRatesPathElementLogDDLMMCurve : public AQLRatesPathElementLMMCurve
{
public:
//  LIFECYCLE
	// Default constructor
	AQLRatesPathElementLogDDLMMCurve(double s, const DoubleArray& tenor, const DoubleArray& delta_tenor, double t = 0);
	// copy constructor
	AQLRatesPathElementLogDDLMMCurve(const AQLRatesPathElementLogDDLMMCurve& v);
	// Destructor
	virtual ~AQLRatesPathElementLogDDLMMCurve();

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
	// equal operator
    /*!
	    @param[in] a source object
        @return copy object
    */
    virtual	AQLRatesPathElementLogDDLMMCurve&
								operator = (const AQLRatesPathElementLogDDLMMCurve& a);
	// get discount bond price
	virtual double				getP (double T) const;
	// set value
	virtual void				set (const AQLRatesPathElementBase& a);

private:

protected:
	double						mSpread;		// spread
};
