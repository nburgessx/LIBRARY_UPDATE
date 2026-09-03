#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAModelDynamicsCurve.h"
#include "LABasic.h"
#include <map>


// ID for LARatesNumeraireBase
#define FN_NUMERAIREBASE	2101 
// initial value of mBasisName
#define NOBASIS "NOBASIS"

class LARatesPathElementCurve;
class LARatesPEInterpolationBase;
class LARatesSDEBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of numeraire class

*/
class LARatesNumeraireBase : public LACoreFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesNumeraireBase(bool isStochasticIR = true);
	// constructor
	LARatesNumeraireBase(const LAString& basisName, const DoubleArray& timeGrid, const DoubleArray& basis, bool isStochasticIR = true);
	//	Copy constructor
	LARatesNumeraireBase(const LARatesNumeraireBase& v);
	// Destructor
	virtual ~LARatesNumeraireBase();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const = 0;// %%% COVARIANT RETURN %%%
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
	const LARatesPathElementCurve&	getCurve(double t) const;
								//======================================
								// get curve at t
	const LARatesPathElementCurve&	getInitialCurve() const;
//	OPERATION
								//======================================
								// set curve
	virtual void				setCurve(double t, const LARatesPathElementCurve* pcurve);
								//======================================
								// set basis spread
	virtual void				setBasisSpread(const LAString& basisName, const DoubleArray& timeGrid, const DoubleArray& basis);
								//======================================
								// set basis name
	virtual void				setBasisName(const LAString& basisName);
								//======================================
								// get basis name
	virtual LAString			getBasisName();
								//======================================
								// reset 
	virtual void				reset();
								//==========================================
	                            // set interpolation class	
	void						setInterpolationMethod(LARatesPEInterpolationBase* pinter);

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
	virtual void				setSDE(LARatesSDEBase* pSDE);
								//======================================
	//							// is cancel spread for simulation 
	//void						isCancelSpread(bool iscancel);

protected:
	class LARatesCurveWithBasis : public LARatesPathElementCurve
	{
	public:
		// Default constructor
		LARatesCurveWithBasis():mpCurve(0), mpBasisCurve(0) {;}
		// Destructor
		virtual ~LARatesCurveWithBasis(){;}
									//======================================
									// Make copy(clone) of this class
									/*!
										@return Deep copy of this class
									*/
		virtual LARatesPathElementBase*
									clone() const;// %%% COVARIANT RETURN %%%
		// equal operator
		/*!
			@param[in] a source object
			@return copy object
		*/
		virtual	LARatesCurveWithBasis&
									operator = (const LARatesCurveWithBasis& a){a; return *this;}
		// get discount bond price
		virtual double				getP (double T) const
									{return mpCurve->getP(T)
										* LAMath::exp((*mpBasisCurve)(m_t) * m_t - (*mpBasisCurve)(T) * T);} 	

		// set base curve(without basis)
		/*!
			@param[in] pCurve base curve(with out basis)
		*/
		void						setBaseCurve(const LARatesPathElementCurve* pCurve)
									{mpCurve = pCurve; m_t = pCurve->get_t();}
		// set basis curve
		/*!
			@param[in] pBasisCurve basis curve
		*/
		void						setBasisCurve(const LAFunctionBase* pBasisCurve)
									{mpBasisCurve = pBasisCurve;}
		
	protected:
		const LARatesPathElementCurve* mpCurve;// base curve(without basis)
        const LAFunctionBase*	mpBasisCurve;//basis curve
	};	

	class LARatesCurveForNumeraire: public LARatesPathElementCurve
	{
	public:
		// Default constructor
		LARatesCurveForNumeraire(){;}
		// Destructor
		~LARatesCurveForNumeraire(){;}
									//======================================
									// Make copy(clone) of this class
									/*!
										@return Deep copy of this class
									*/
		virtual LARatesPathElementBase*
									clone() const {return NULL;}// %%% COVARIANT RETURN %%%
		// equal operator
		/*!
			@param[in] a source object
			@return copy object
		*/
		virtual	LARatesCurveForNumeraire&
									operator = (const LARatesCurveForNumeraire& a){a; return *this;}
		// get discount bond price
		virtual double				getP (double T) const; 	

	private:
		
	};

	std::map<double, const LARatesPathElementCurve*>		mCurves;// curve objects
	LARatesPEInterpolationBase*							mpInter;// interpolation method
	bool												mIsStochastic;// IR is stochastic or not
	LARatesCurveForNumeraire								mLastCurve;// curve for last curve element
	bool												mIsLongJump;// LongJump or Not
	LAFunctionBase*										mpBasisCurve;//basis curve
	LARatesCurveWithBasis*									mpCurveWithBasis;// curve with Basis
	LARatesSDEBase*										mpSDE;// pointer to SDE that holds this Numerarire
	//bool												mCancelSpread;//this is set when LMM and funding change
	mutable bool										mUpdateFlag;		// update flag
	LAString											mBasisName;
	std::map<LAString, LAFunctionBase*>					mBasisCurveMap;

private:
	virtual LARatesCurveWithBasis* getCurveWithBasis() const;
	//======================================
	// get curve without basis at t
	const LARatesPathElementCurve&	getCurveWithoutBasis(double t) const;

};
