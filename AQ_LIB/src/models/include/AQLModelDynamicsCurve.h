#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLModelDynamicsBase.h"


// ID for AQLRatesPathElementCurve
#define PE_CURVE	3 



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of path element class that represents curve object
*/
class AQLRatesPathElementCurve : public AQLRatesPathElementBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLRatesPathElementCurve(double t = 0);
	// copy constructor
	AQLRatesPathElementCurve(const AQLRatesPathElementCurve& v);
	// Destructor
	virtual ~AQLRatesPathElementCurve();

//  QUERY
								//======================================
								// Check pathelement for this class ID
	virtual bool                isTypeOf(pathelement_t id) const;
								//======================================
								// Return this class ID
	virtual pathelement_t		getType() const;

	// equal operator
    /*!
	    @param[in] a source object
        @return copy object
    */
    virtual	AQLRatesPathElementCurve&
								operator = (const AQLRatesPathElementCurve& a);

	// get discount bond price
	/*!
	    @param[in] T maturity
        @return discount bond price
    */
	virtual double				getP (double T) const = 0;
	// get derivative of discount bond price
	virtual double				getP_D (double T, double dt = 0.001) const;
	// get 2nd derivative of discount bond price
	virtual double				getP_D2 (double T, double dt = 0.001) const;
	// get zero rate
	virtual double				getZero (double T) const; 
	// get forward spot rate
	virtual double				getF (double T, double dt = 0.0001) const;
	// get derivative of forward spot rate
	virtual double				getF_D (double T, double dt = 0.001) const;
	// get initial time of this curve
	/*!
	    @return initial time
    */	
	double						get_t (void) const {return m_t;}
	// set start time of this curve
	virtual void				set_t (double t);
								// set value
	virtual void				set(const AQLRatesPathElementBase& a); 


private:

protected:
	double m_t;	// start time of this curve
};

