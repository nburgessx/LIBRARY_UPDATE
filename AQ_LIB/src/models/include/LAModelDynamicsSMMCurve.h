#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "LAModelDynamicsCurve.h"



// ID for LARatesPathElementSMMCurve
#define PE_SMMCURVE	8 



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of path element class that represents curve object for libor market model
*/
class LARatesPathElementSMMCurve : public LARatesPathElementCurve
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesPathElementSMMCurve(const DoubleArray& tenor, const DoubleArray& delta_tenor, double t = 0);
	// Destructor
	virtual ~LARatesPathElementSMMCurve();
	// copy constructor
	LARatesPathElementSMMCurve(const LARatesPathElementSMMCurve& v);

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
    virtual	LARatesPathElementSMMCurve&
								operator = (const LARatesPathElementSMMCurve& a);

	// get discount bond price
	virtual double				getP (double T) const;
	// get value
	/*!
		@return value
	*/
//	virtual const void*			get(void) const {return &mData_L;}
	// set value
	/*!
		@param[in] pdata pointer to value to set
	*/
/*	virtual void				set(const void* pdata) 
								{	
									const SCALARARRAY* p_array = reinterpret_cast<const SCALARARRAY*>(pdata); 
				
									int i, j;
									for (i = p_array->size() - 1, j = mData_L.size() - 1; j >= 0; j--, i--)
										mData_L[j] = (*p_array)[i];
								}*/
	// set value
	/*!
		@param[in] a value to set
	*/	
	virtual void				set(DoubleArray a)
								{	
									unsigned int i, j;
									for (i = a.size() - 1, j = mValue.size() - 1; j >= 0; j--, i--)
										mValue[j] = a[i];
									DoubleArray q(mValue.size());
									double b = (*mpDeltaTenor).size() - mValue.size();
									//mData_L.resize(mValue.size());
									for (i = a.size() - 1, j = mValue.size() - 1; j >= 0; j--, i--)
									{
										unsigned int m, n;	double p = 1.0;
										for (m = j; m <= mValue.size() - 1; m++)
										{
											if (j + 1 <= m)
												for (n = j + 1; n <= m; n++)
													p *= q[n];
											q[j] += (*mpDeltaTenor)[m + b] * p;
											p = 1.0;
										}
										q[j] *= mValue[j];
										
										p = 1.0;
										if (j + 1 <= mValue.size() - 1)
											for (n = j + 1; n <= mValue.size() - 1; n++)
												p *= q[n];
										q[j] += p;
										q[j] = 1 / q[j];
										mData_L[j] = (1 / q[j] - 1.0) / (*mpDeltaTenor)[i + b];
									}
								}
	// set value
	virtual void				set (const LARatesPathElementBase& a);


	// set start time of this curve
	virtual void				set_t (double t); 
	// get libor rate
	double						getL (double ts, double te, double delta, const LARatesPathElementCurve& curve) const;
	// get swap rate
	//double					getS (double ts, double te, double annuity, const LARatesPathElementCurve& curve) const;
	double						getS (DoubleArray tenor, DoubleArray deltatenor, const LARatesPathElementCurve& curve) const;

private:
	// set start time of this curve
	void						set_t (void); 
	// get annuity
	double						getA (DoubleArray tenor, DoubleArray deltatenor, const LARatesPathElementCurve& curve) const;

	// clear data
	void						clear();

	int*						mpRefCount;		// reference counter of common data   

protected:
	SCALARARRAY					mData_L;		// libor
	DoubleArray*				mpTenor;		// tenor
	DoubleArray*				mpDeltaTenor;	// delta tenor
	SCALARARRAY*				mpInitialData_L;// initial libor
	SCALARARRAY*				mpInitialData_S;// initial libor
	const LARatesPathElementCurve*	mpInitialCurve;	// initial curve
};
