#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesNumeraireBase.h"
#include "AQL1DDataSet.h"

// ID for LARatesNumeraireBankAccountHW
#define FN_NUMERAIREBANKACCOUNTHW	10039 
// Function name for LARatesNumeraireBankAccountHW
#define FN_NUMERAIREBANKACCOUNTHW_STR	"fn_numeraire_bankaccounthw"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of bank account numeraire class
*/
class LARatesNumeraireBankAccountHW : public LARatesNumeraireBase
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesNumeraireBankAccountHW();
	// constructor
	LARatesNumeraireBankAccountHW(const AQLString& basisName, const DoubleArray& timeGrid, const DoubleArray& basis);
	//	Copy constructor
//	LARatesNumeraireBankAccountHW(const LARatesNumeraireBankAccountHW& v);
	// Destructor
	virtual ~LARatesNumeraireBankAccountHW();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// get numeraire value at t
	virtual double				operator()(double t) const;
								//======================================
								// set curve
	virtual void				setCurve(double t, const LARatesPathElementCurve* pcurve);
								//======================================
								// set basis spread
	virtual void				setBasisSpread(const AQLString& basisName, const DoubleArray& timeGrid, const DoubleArray& basis);
								//======================================
								// set basis name
	virtual void				setBasisName(const AQLString& basisName);
								//======================================
								// set SDE that holds this Numerarire
	virtual void				setSDE(LARatesSDEBase* pSDE);
								//======================================
								// reset 
	virtual void				reset();

protected:
	class LARatesCurveWithBasisHW : public LARatesCurveWithBasis
	{
		friend class LARatesNumeraireBankAccountHW;
	public:
		// Default constructor
		LARatesCurveWithBasisHW(){;}
		// Destructor
		virtual ~LARatesCurveWithBasisHW(){;}
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
		virtual	LARatesCurveWithBasisHW&
									operator = (const LARatesCurveWithBasisHW& a){a; return *this;}
		// get forward spot rate
		virtual double				getF (double T, double dt = 0.001) const;
		// setup basis forward curve
		void						setBasisForwardCurve(const DoubleArray& timeGrid);
		
	private:
		//mutable std::map<double, double>	cache;
		//mutable std::map<double, bool>		is_cache;
		AQL1DDataSet							mBasisForwardCurve;
	};	

//	OPERATION
private:
		// setUp Adjust Value of Numeraire
	void							setAdjVal() const;
	virtual LARatesCurveWithBasis* getCurveWithBasis() const;

								//======================================
								// calc nuemraire	
	void						calcNumeraire(void) const;
	//mutable bool	mUpdateFlag;		// update flag
	mutable	DoubleArray	mNumeraireArray;// numerarie values
	mutable	DoubleArray	mSpotArray;// spot r values
	mutable	DoubleArray	mTimeGrid;		// time grid
	mutable std::map<double, double>	cache;
	mutable std::map<double, bool> is_cache;
	mutable bool	mUpdateAdjValFlag;	// update flag for AdjVal
	mutable DoubleArray mAdjVal;		// Adjust Value of Numeraire
	mutable DoubleArray mInitialSpotArray;// mInitialSpotArray
	const LARatesPathElementCurve*	mpInitialCurve;// intitial curve
	mutable double mCalcStartTime;
};
