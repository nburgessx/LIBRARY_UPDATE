/*! @file
    @brief Declaration of path element class that represents curve object for HW

*/
#ifndef LAMATHHWFUNCMR_H
#define LAMATHHWFUNCMR_H

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathHWFuncMR.h
//
//  SYNOPSIS    :       LAMathHWFuncMR
//  DESCRIPTION :       Declaration of path element class that represents curve object for HW
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma interface
#endif

//+++++ INCLUDE +++++
#include "AQLCoreTemplateType.h"
#include "AQLFunctionBase.h"
#include "LAModelDynamicsAFFCurve.h"
#include "AQLInterpolationBase.h"
#include <map>
#include "AQLGaussLegendre.h"
#include "AQLCoreSystemError.h"


const int  HWGAUSSLEGENDRENUM = 20;
const int  HWINTEGRATEDNUM = 2;



//
//------------------------------ LAMathHWFuncMR ------------------------------
//
//

//+++++ DEFINE +++++
// Funciton ID of LAMathHWFuncMR
#define FN_HWFUNCMR	10018
// Function Name of LAMathHWFuncMR
#define FN_HWFUNCMR_STR	"fn_hwfuncmr"

class LAMathHWFuncMR : public AQLFunctionBase
{
public :
//  LIFECYCLE
                                //======================================
	                            // constructor
	LAMathHWFuncMR();
    explicit 
    LAMathHWFuncMR(double a) : mMR(a) {};
                                //======================================
                                // copy constructor
	LAMathHWFuncMR(const LAMathHWFuncMR &rhs);
                                //======================================
                                // destructor
	virtual ~LAMathHWFuncMR(void);
                                //======================================
//  QUERY
								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual AQLCoreFunctionBase*    clone() const;
								//======================================
								// return function value
	virtual double				operator()(const DoubleArray& x) const
								{
									return mMR;
									//throw AQLCoreInvalidData("this operator is not supported",__FILE__,__LINE__);
								};
								// return this class type
    virtual function_t          getType() const;
	                            //==========================================
	                            // return a value
    virtual double              get_a(double t) const { return mMR; }
                                //==========================================
                                // integrate from 0 to t
    virtual double              integrate( double t ) const { return mMR * t; }
                                //==========================================
                                // integrate from t_s to t_e
    virtual double              integrate( double ts, double te ) const { return mMR * (te - ts); }
                                //==========================================

private :
	
    // a
    double mMR;		     	
	
};

//+++++ DEFINE +++++
// Funciton ID of LAMathHWFuncMRTMDPT
#define FN_HWFUNCMRTMDPT	10019
// Function Name of LAMathHWFuncMRTMDPT
#define FN_HWFUNCMRTMDPT_STR	"fn_hwfuncmrtmdpt"

class LAMathHWFuncMRTMDPT : public LAMathHWFuncMR
{
public :
//  LIFECYCLE
                                //======================================
	                            // constructor
   /* LAMathHWFuncMRTMDPT( const DoubleArray&    T_grid_,
                  const DoubleArray&    a_grid_,
                  Interpolation&        interpolation_,
                  int                   integrate_n_
                );*/
	 LAMathHWFuncMRTMDPT( const DoubleArray&    T_grid_,
                  const DoubleArray&    a_grid_,
				  AQLInterpolationBase&	interpolation_);
                                //======================================
                            	// copy constructor
	LAMathHWFuncMRTMDPT(const LAMathHWFuncMRTMDPT &rhs);
                                //======================================
                                // destructor
	virtual ~LAMathHWFuncMRTMDPT(void);
                                //======================================	
//  QUERY
								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual AQLCoreFunctionBase*   clone() const;
								//======================================
                            	// equal operator
                                /*!
	                                @param[in] a source object
                                    @return copy object
                                */
    //virtual LAMathHWFuncMRTMDPT&       operator =( const LAMathHWFuncMRTMDPT& rhs );
	
								// return function value
	virtual double				operator()(const DoubleArray& x) const
								{
									return mpInter->value(x[0]);
								};
								//======================================
								// return this class type
    virtual function_t          getType() const;
	                            //==========================================
	                            // return a value at t
    virtual double              get_a(double t) const;
                                //======================================
                                // integrate from 0 to t 
    virtual double              integrate( double t ) const;
                                //======================================
                                // integrate from t_s to t_e
    virtual double              integrate( double ts, double te ) const;
                                //======================================
  
private:

    
    DoubleArray a_grid;		// a grid
	DoubleArray T_grid;		 // time grid    
	AQLInterpolationBase* mpInter;
    bool is_cloned;
   	AQLGaussLegendre mGL;
	
	
};


#endif