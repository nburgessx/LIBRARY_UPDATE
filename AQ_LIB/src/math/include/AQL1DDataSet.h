#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// Funciton ID of AQL1DDataSet
#define FN_1DDATASET	1203
// Function Name of AQL1DDataSet
#define FN_1DDATASET_STR	"fn_1ddataset"
// Funciton ID of No interpolation
#define NULL_INTERPOLATION	0

class AQLInterpolationBase;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to express functions with data points and interpolation

    This class derives from AQLFunctionBase

*/
class AQL1DDataSet : public AQLFunctionBase
{
public:
	// constructor
	explicit AQL1DDataSet(bool checkflag = false);
	// destructor
	virtual ~AQL1DDataSet();
	//	Copy constructor
	AQL1DDataSet(const AQL1DDataSet& v);
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
								//======================================
								// Return interpolation type
    virtual function_t          getInterpolationType() const;
	                            //==========================================
	                            // return interpolated value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
	                            // return interpolated value
	virtual double				operator()(double x) const;
	
	// 20060929 override a virtual function of the base class 
	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								};

	                            //==========================================
	                            // return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x,
										const AQLIntegralBase* pIntegral) const;
	                            //==========================================
	                            // return integral result
	virtual double				integral(double x1, double x2,
										const AQLIntegralBase* pIntegral) const;
	                            //==========================================
	                            // return integral result
	virtual double				integral(double x1, double x2) const;

	// 20060929 override a virtual function of the base class
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const
								{
									return AQLFunctionBase::integral(x);
								};
	virtual double				integral(double xl, double xu,
										const AQL1DIntegral* pIntegral) const
								{
									return AQLFunctionBase::integral(xl, xu, pIntegral);
								};

	                            //==========================================
	                            // return partial derivative result
	virtual double				partialDerivative(const DoubleArray& x, unsigned int pos,
										CALC_TYPE calctype = DEFAULT, DIFF_TYPE difftype = BOTHSIDE,double delta = 0.0001) const;

	                            //==========================================
	                            // return second partial derivative result
	virtual double				partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype = DEFAULT, double delta = 0.0001) const;
								//==========================================
								// check x is in domain of this function or not.
	virtual	bool				isInDomain(const DoubleArray& x)const{ 
												(void)x;
												return true;};
								//==========================================
	                            // check derivable or not.
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int pos)const;
							
								//==========================================
	                            // check double derivable or not.	
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int posi, unsigned int posj)const;
								

								//==========================================
	                            // return grid points
	virtual const DoubleArray&
								getGrids() const{return mGrids;};


	                            //==========================================
	                            // set data points and their values
	virtual void				set(const DoubleArray& x, const DoubleArray& y);
						
	                            //==========================================
	                            // set interpolation method
	virtual void				setInterpolation(const AQLInterpolationBase& v);


								//==========================================
	                            // set first and second derivability	
	void						setGridDerivableFlag(bool flag1 , bool flag2)
								{	
									mGridDifferentiableFlag1 = flag1;
									mGridDifferentiableFlag2 = flag2;
								};

	AQL1DDataSet & operator=( const AQL1DDataSet & ) { return *this; }

private:
	DoubleArray mGrids;					// grid points
	mutable AQLInterpolationBase* mpInter;	// pointer to interpolation function
	bool mGridDifferentiableFlag1;			// first derivative is possible or not
	bool mGridDifferentiableFlag2;			// second derivative is possible or not
protected:
	
};

