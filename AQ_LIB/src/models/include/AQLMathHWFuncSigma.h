/*! @file
    @brief Declaration of volatility tool class that represents for HW

*/
#ifndef AQLMATHHWFUNCSIGMA_H
#define AQLMATHHWFUNCSIGMA_H


#ifdef __GNUG__
#pragma interface
#endif




//
//------------------------------ AQLMathHWFuncSigma ------------------------------
//

//+++++ INCLUDE +++++
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLInterpolationBase.h"
#include "AQLAlgorithm.h"
#include "AQLFunctionBase.h"
#include "AQLCoreSystemError.h"


//+++++ DEFINE +++++
// Funciton ID of AQLMathHWFuncSigma
#define FN_HWFUNCSIGMA	10021
// Function Name of AQLMathHWFuncSigma
#define FN_HWFUNCSIGMA_STR	"fn_hwfuncsigma"

class AQLMathHWFuncSigma : public AQLFunctionBase
{
public :
//  LIFECYCLE
                                //======================================
	                            // constructor
	explicit
    AQLMathHWFuncSigma(double sigma);
                                //======================================
                                // copy constructor
	AQLMathHWFuncSigma(const AQLMathHWFuncSigma &rhs);
                                //======================================
                            	// destructor
	virtual ~AQLMathHWFuncSigma(void);
                                //======================================
//  QUERY
								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual AQLCoreFunctionBase*    clone() const;
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
//------------------------------ AQLMathHWFuncSigmaTMDPT ------------------------------
//

//+++++ DEFINE +++++
// Funciton ID of AQLMathHWFuncSigmaTMDPT
#define FN_HWFUNCSIGMATMDPT	10022
// Function Name of AQLMathHWFuncSigmaTMDPT
#define FN_HWFUNCSIGMATMDPT_STR	"fn_hwfuncsigmatmdpt"

class AQLMathHWFuncSigmaTMDPT : public AQLMathHWFuncSigma
{
public :
//  LIFECYCLE
                                //======================================
	                            // constructor
	explicit 
   	AQLMathHWFuncSigmaTMDPT(const DoubleArray& T_Grid_, const DoubleArray& s_grid_, AQLInterpolationBase& interpolation);
                                //======================================	
                                // destructor
	virtual ~AQLMathHWFuncSigmaTMDPT(void);
                                //======================================	
                            	// copy constructor
	AQLMathHWFuncSigmaTMDPT(const AQLMathHWFuncSigmaTMDPT &rhs);
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
    //virtual AQLMathHWFuncSigmaTMDPT&   operator =( const AQLMathHWFuncSigmaTMDPT& rhs );
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
	AQLInterpolationBase* mpInter;	// mpInter method
    bool                        is_cloned;
	
};

#endif