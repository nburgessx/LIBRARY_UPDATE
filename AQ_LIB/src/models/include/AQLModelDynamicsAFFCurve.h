#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLModelDynamicsSRCurve.h"

// ID for AQLRatesPathElementAFFCurve
#define PE_AFFCURVE	10031 

//
//------------------------------ AQLRatesPathElementAFFCurve ------------------------------
//

/*! 
    @brief Declaration of path element class that represents curve object
*/
class AQLRatesPathElementAFFCurve : public AQLRatesPathElementSRCurve
{
public:
//  LIFECYCLE
                                //======================================
	                            // constructor
	explicit 
    AQLRatesPathElementAFFCurve(double t = 0);
								// Destructor
	virtual ~AQLRatesPathElementAFFCurve();
                                //======================================

//  QUERY
								//======================================
								// Check pathelement for this class ID
	virtual bool                isTypeOf(pathelement_t id) const;
								//======================================
								// Return this class ID
	virtual pathelement_t		getType() const;
    							//======================================
	virtual double				getP (double T) const;
                                //======================================	

protected:

    virtual double A( double T ) const = 0;
	virtual double B( double T ) const = 0;

private:

};
