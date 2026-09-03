/*! @file
    @brief Declaration of volatility tool class that represents for HW

*/
#ifndef LAMATHHWFUNCSIGMA_H
#define LAMATHHWFUNCSIGMA_H

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathHWFuncSigma.h
//
//  SYNOPSIS    :       LAMathHWFuncSigma
//  DESCRIPTION :       Declaration ofvolatility tool class that represents for HW
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma interface
#endif




//
//------------------------------ LAMathHWFuncSigma ------------------------------
//

//+++++ INCLUDE +++++
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAInterpolationBase.h"
#include "LAAlgorithm.h"
#include "LAFunctionBase.h"
#include "LACoreSystemError.h"


//+++++ DEFINE +++++
// Funciton ID of LAMathHWFuncSigma
#define FN_HWFUNCSIGMA	10021
// Function Name of LAMathHWFuncSigma
#define FN_HWFUNCSIGMA_STR	"fn_hwfuncsigma"

class LAMathHWFuncSigma : public LAFunctionBase
{
public :
//  LIFECYCLE
                                //======================================
	                            // constructor
	explicit
    LAMathHWFuncSigma(double sigma);
                                //======================================
                                // copy constructor
	LAMathHWFuncSigma(const LAMathHWFuncSigma &rhs);
                                //======================================
                            	// destructor
	virtual ~LAMathHWFuncSigma(void);
                                //======================================
//  QUERY
								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual LACoreFunctionBase*    clone() const;
								//======================================
	virtual double				operator()(const DoubleArray& x) const
								{
									 return mSigma; 
								};
	virtual double				operator()(double x) const
								{
									 return mSigma; 
								};
								// return this class type
    virtual function_t          getType() const;
	                            //==========================================
	virtual void                set_s(double sigma){ mSigma = sigma; }
                                //==========================================

private :
	
    // sigma
    double mSigma;		     	
	
};


//
//------------------------------ LAMathHWFuncSigmaTMDPT ------------------------------
//

//+++++ DEFINE +++++
// Funciton ID of LAMathHWFuncSigmaTMDPT
#define FN_HWFUNCSIGMATMDPT	10022
// Function Name of LAMathHWFuncSigmaTMDPT
#define FN_HWFUNCSIGMATMDPT_STR	"fn_hwfuncsigmatmdpt"

class LAMathHWFuncSigmaTMDPT : public LAMathHWFuncSigma
{
public :
//  LIFECYCLE
                                //======================================
	                            // constructor
	explicit 
   	LAMathHWFuncSigmaTMDPT(const DoubleArray& T_Grid_, const DoubleArray& s_grid_, LAInterpolationBase& interpolation);
                                //======================================	
                                // destructor
	virtual ~LAMathHWFuncSigmaTMDPT(void);
                                //======================================	
                            	// copy constructor
	LAMathHWFuncSigmaTMDPT(const LAMathHWFuncSigmaTMDPT &rhs);
                                //======================================	
//  QUERY
								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual LACoreFunctionBase*   clone() const;
								//======================================
                            	// equal operator
                                /*!
	                                @param[in] a source object
                                    @return copy object
                                */
    //virtual LAMathHWFuncSigmaTMDPT&   operator =( const LAMathHWFuncSigmaTMDPT& rhs );
								//======================================
	virtual double				operator()(const DoubleArray& x) const
								{
									return mpInter->value(x[0]);	
								};
	virtual double				operator()(double x) const
								{
									return mpInter->value(x);	
								};
								// return this class type
    virtual function_t          getType() const;
	                            //==========================================
	// set sigma values
	virtual void				set_s(const DoubleArray& sgrid);
								//==========================================
	// get sigma vector
	const DoubleArray&			get_svec() const {return s_grid;};
	// get sigma vector
	const DoubleArray&			get_Tvec() const {return T_grid;};
private :

    
    DoubleArray T_grid;				// time grid      
	DoubleArray s_grid;				// sigma grid	     
	LAInterpolationBase* mpInter;	// mpInter method
    bool                        is_cloned;
	
};

#endif