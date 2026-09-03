#pragma once

#ifdef __GNUG__
#pragma interface
#endif

//+++++ INCLUDE +++++
#include "LAModelDynamicsCurve.h"
#include "LAMathYieldCurve.h"

//+++++ DEFINE +++++
// ID for LARatesPathElementSRCurve
#define PE_SRCURVE	10030 


//
//------------------------------ LARatesPathElementSRCurve ------------------------------
//

/*! 
    @brief Declaration of path element class that represents Short Rate curve object
*/
class LARatesPathElementSRCurve : public LARatesPathElementCurve
{
public:
//  LIFECYCLE
                                //======================================
                            	// constructor
    explicit
    LARatesPathElementSRCurve(double t = 0.0);

                                //======================================
                                // Destructor
    virtual ~LARatesPathElementSRCurve();
                                //======================================

//  QUERY
								//======================================
								// Check pathelement for this class ID
	virtual bool                isTypeOf(pathelement_t id) const;
								//======================================
								// Return this class ID
	virtual pathelement_t		getType() const;
                                //======================================
                                // get discount bond price
                                /*!
	                                @param[in] T maturity
                                    @return discount bond price
                                */
	virtual double				getP (double T) const = 0;
								//======================================
								// set r
	virtual void				set(SCALAR a){ mValue[0] = a;}
								//======================================
								// set r
	virtual void				set(const SCALARARRAY& a) { mValue[0] = a[0];} 
                        		//======================================
								// get r
    double						get_r () const { return mValue[0]; }
                                //======================================
protected:
private:

};

