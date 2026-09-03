#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLModelDynamicsLMMCurve.h"
#include "AQLBasic.h"


// ID for AQLRatesPathElementDDLMMCurve
#define PE_DDLMMCURVE	5 



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of path element class that represents curve object for libor market model
*/
class AQLRatesPathElementDDLMMCurve : public AQLRatesPathElementLMMCurve
{
public:
//  LIFECYCLE
	// Default constructor
	AQLRatesPathElementDDLMMCurve(double s, const double constShift, const DoubleArray& tenor, const DoubleArray& delta_tenor, double t = 0);
	// copy constructor
	AQLRatesPathElementDDLMMCurve(const AQLRatesPathElementDDLMMCurve& v);
	// Destructor
	virtual ~AQLRatesPathElementDDLMMCurve();

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
								//======================================
								// get value
	/*virtual const SCALARARRAY&	get(void) const 
								{
									mValue_with_Spread = mValue;
									unsigned int i, size = mValue.size();
									for (i = 0; i < size; i++) mValue_with_Spread[i] += mSpread;
									return mValue_with_Spread;
								} */
	// equal operator
    /*!
	    @param[in] a source object
        @return copy object
    */
    virtual	AQLRatesPathElementDDLMMCurve&
								operator = (const AQLRatesPathElementDDLMMCurve& a);

	/*!
		@param[in] a value to set
	*/	
	/*virtual void				set(const SCALARARRAY& a) 
								{	
									int i, j;
									for (i = a.size() - 1, j = mValue.size() - 1; j >= 0; j--, i--)
										mValue[j] = a[i] - mSpread;
								}*/
	// set value
	virtual void				set (const AQLRatesPathElementBase& a);
								//======================================
								// get P value
	virtual double				getP(double T) const;
								//======================================
								// set spread 
	void						setSpread(double s){mSpread = s;};
								//======================================
								// set constant shift 
	void						setConstShift(double constShift){mConstShift = constShift;};
								//======================================
								// get spreads 
	const DoubleVector&			getSpreads() const {return mSpreads;}
								//======================================
								// set TrancateVal 
	void						setTrancateVal(double val){ mTrancateVal = val;}
								//======================================
								// get spreads ID
	static unsigned int         getSpreadsID() {return mSpreadsID;}


private:


protected:
	DoubleArray					mSpreads;		//spread vector = mSpread * mL_i + mConstShift
	double						mConstShift;	// constant shift
	double						mTrancateVal;	//infimum of the calculating path 
	double						mSpread;		// - AQLMath::log(Q)/AQLMath::log(2.0);
	static unsigned int         mSpreadsID;     // spreads ID
	//mutable SCALARARRAY 		mValue_with_Spread;	// L + spread
};
