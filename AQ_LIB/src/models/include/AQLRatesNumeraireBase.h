#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLBasic.h"
#include <map>


// ID for AQLRatesNumeraireBase
#define FN_NUMERAIREBASE	2101 
// initial value of mBasisName
#define NOBASIS "NOBASIS"

class AQLRatesPathElementCurve;
class AQLRatesPEInterpolationBase;
class AQLRatesSDEBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of numeraire class

*/
class AQLRatesNumeraireBase : public AQLCoreFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLRatesNumeraireBase(bool isStochasticIR = true);
	// constructor
	AQLRatesNumeraireBase(const AQLString& basisName, const DoubleArray& timeGrid, const DoubleArray& basis, bool isStochasticIR = true);
	//	Copy constructor
	AQLRatesNumeraireBase(const AQLRatesNumeraireBase& v);
	// Destructor
	virtual ~AQLRatesNumeraireBase();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const = 0;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// get numeraire value at t
								/*!
									@param[in] t time
									@return numeraire value
								*/
	virtual double				operator()(double t) const = 0;
								//======================================
								// get curve at t
	const AQLRatesPathElementCurve&	getCurve(double t) const;
								//======================================
								// get curve at t
	const AQLRatesPathElementCurve&	getInitialCurve() const;
//	OPERATION
								//======================================
								// set curve
	virtual void				setCurve(double t, const AQLRatesPathElementCurve* pcurve);
								//======================================
								// set basis spread
	virtual void				setBasisSpread(const AQLString& basisName, const DoubleArray& timeGrid, const DoubleArray& basis);
								//======================================
								// set basis name
	virtual void				setBasisName(const AQLString& basisName);
								//======================================
								// get basis name
	virtual AQLString			getBasisName();
								//======================================
								// reset 
	virtual void				reset();
								//==========================================
	                            // set interpolation class	
	void						setInterpolationMethod(AQLRatesPEInterpolationBase* pinter);

								//======================================
								// clear data
	void						clear();
								//======================================
								// set LongJump or not
								/*!
									@param[in] flag true:LongJump, false:not LongJump
								*/	
	void						setLongJump(bool flag) {mIsLongJump = flag;}
								//======================================
								// set SDE that holds this Numerarire
	virtual void				setSDE(AQLRatesSDEBase* pSDE);
								//======================================
	//							// is cancel spread for simulation 
	//void						isCancelSpread(bool iscancel);

protected:
	class AQLRatesCurveWithBasis : public AQLRatesPathElementCurve
	{
	public:
		// Default constructor
		AQLRatesCurveWithBasis():mpCurve(0), mpBasisCurve(0) {;}
		// Destructor
		virtual ~AQLRatesCurveWithBasis(){;}
									//======================================
									// Make copy(clone) of this class
									/*!
										@return Deep copy of this class
									*/
		virtual AQLRatesPathElementBase*
									clone() const;// %%% COVARIANT RETURN %%%
		// equal operator
		/*!
			@param[in] a source object
			@return copy object
		*/
		virtual	AQLRatesCurveWithBasis&
									operator = (const AQLRatesCurveWithBasis& a){a; return *this;}
		// get discount bond price
		virtual double				getP (double T) const
									{return mpCurve->getP(T)
										* AQLMath::exp((*mpBasisCurve)(m_t) * m_t - (*mpBasisCurve)(T) * T);} 	

		// set base curve(without basis)
		/*!
			@param[in] pCurve base curve(with out basis)
		*/
		void						setBaseCurve(const AQLRatesPathElementCurve* pCurve)
									{mpCurve = pCurve; m_t = pCurve->get_t();}
		// set basis curve
		/*!
			@param[in] pBasisCurve basis curve
		*/
		void						setBasisCurve(const AQLFunctionBase* pBasisCurve)
									{mpBasisCurve = pBasisCurve;}
		
	protected:
		const AQLRatesPathElementCurve* mpCurve;// base curve(without basis)
        const AQLFunctionBase*	mpBasisCurve;//basis curve
	};	

	class AQLRatesCurveForNumeraire: public AQLRatesPathElementCurve
	{
	public:
		// Default constructor
		AQLRatesCurveForNumeraire(){;}
		// Destructor
		~AQLRatesCurveForNumeraire(){;}
									//======================================
									// Make copy(clone) of this class
									/*!
										@return Deep copy of this class
									*/
		virtual AQLRatesPathElementBase*
									clone() const {return NULL;}// %%% COVARIANT RETURN %%%
		// equal operator
		/*!
			@param[in] a source object
			@return copy object
		*/
		virtual	AQLRatesCurveForNumeraire&
									operator = (const AQLRatesCurveForNumeraire& a){a; return *this;}
		// get discount bond price
		virtual double				getP (double T) const; 	

	private:
		
	};

	std::map<double, const AQLRatesPathElementCurve*>		mCurves;// curve objects
	AQLRatesPEInterpolationBase*							mpInter;// interpolation method
	bool												mIsStochastic;// IR is stochastic or not
	AQLRatesCurveForNumeraire								mLastCurve;// curve for last curve element
	bool												mIsLongJump;// LongJump or Not
	AQLFunctionBase*										mpBasisCurve;//basis curve
	AQLRatesCurveWithBasis*									mpCurveWithBasis;// curve with Basis
	AQLRatesSDEBase*										mpSDE;// pointer to SDE that holds this Numerarire
	//bool												mCancelSpread;//this is set when LMM and funding change
	mutable bool										mUpdateFlag;		// update flag
	AQLString											mBasisName;
	std::map<AQLString, AQLFunctionBase*>					mBasisCurveMap;

private:
	virtual AQLRatesCurveWithBasis* getCurveWithBasis() const;
	//======================================
	// get curve without basis at t
	const AQLRatesPathElementCurve&	getCurveWithoutBasis(double t) const;

};
