#ifndef LAMathCapFloor_h
#define LAMathCapFloor_h

//#include "AQLFunctionBase.h>
#include <AQLCoreTemplateType.h>

#include <vector>

class LAMathPathEntity;
class LAMathVolatilityLMMDiscModel;
class LARatesPathElementCurve;

using namespace std;

//
//--------------------------------------------------------------------------------
//

class LAMathCapFloor
{
public:

    // constructor
    LAMathCapFloor();

    // constructor
	LAMathCapFloor(LARatesPathElementCurve& curve0_,
				 const DoubleVector& T_fix_,
				 const DoubleVector& T_pay_,
				 const DoubleVector& tau_L,
				 const DoubleVector& tau_
			);

    // constructor
	LAMathCapFloor(LARatesPathElementCurve& curve0_,
				 const DoubleVector& T_fix_,
				 const DoubleVector& T_pay_,
				 const DoubleVector& tau_L,
				 const DoubleVector& tau_,
				 const DoubleVector& strike_
			);

	//	copy constructor
	LAMathCapFloor(const LAMathCapFloor& rhs);

    // Destructor
	virtual ~LAMathCapFloor();

    // Make copy(clone) of this class
    virtual LAMathCapFloor* clone() const = 0;
//
//------------------
//
    // Equal operator
	virtual LAMathCapFloor& operator=(const LAMathCapFloor& rhs);

    // Get Variance of SR(Maturity, Expire)
    virtual void getCapletVariance() = 0;

    // Get stdard deviation
    virtual double getStdDev( int sgn = 1 );

    // Get PV
    virtual double getPV( int callfloor_flg = 1);

    // Get minimum PV
    virtual double getMinimumPV( int callfloor_flg = 1);

    // Get Vega
    virtual double getVega() = 0;

//
//------------------
//

protected :

    // Get Forward LIBOR
    double getForwardLIBOR( double T_fix, double term_rate, double T_pay ) const;

    // Get Annuity
    double getAnnuity( const DoubleVector& T_pay, const DoubleVector& term_acc ) const;

    // Forward Rate Value
    double getForwardRateValue( double T_fix, double T_pay ) const;

    // Get FRA
    double getFRA(double T_fix, double T_pay, double term_acc, double fix_rate ) const;

    // Get Swap Value
    double getSwapValue(const DoubleVector& T_fix, const DoubleVector& T_pay, const DoubleVector& term, double swap_rate ) const;

    // Get Swap Rate
    double getSwapRate(const DoubleVector& T_fix, const DoubleVector& T_pay, const DoubleVector& term ) const;

    // Get Swap Float
    double getSwapFloat(const DoubleVector& T_fix, const DoubleVector& T_pay) const;

//
//------------------
//

    //
    LARatesPathElementCurve* mCurve0;

    //
    DoubleVector mT_fix;
    DoubleVector mT_pay;
    DoubleVector mTau;
    size_t mn;

    // Foward LIBOR
    DoubleVector mL;

    // Strike
    DoubleVector mStrike;

    // caplet variance
    DoubleVector mCapletVar;

private:

    bool mCloned;
};

//
//--------------------------------------------------------------------------------
//

class LAMathCapFloorBlackVol : public LAMathCapFloor
{
public:

    // constructo
    LAMathCapFloorBlackVol();

    // constructor
    LAMathCapFloorBlackVol(LARatesPathElementCurve& curve0_,
						 double black_vol_,
						 const DoubleVector& T_fix_,
						 const DoubleVector& T_pay_,
						 const DoubleVector& tau_L,
						 const DoubleVector& tau_
			);

	// constructor
	LAMathCapFloorBlackVol(LARatesPathElementCurve& curve0_,
						 double black_vol_,
						 const DoubleVector& T_fix_,
						 const DoubleVector& T_pay_,
						 const DoubleVector& tau_L,
						 const DoubleVector& tau_,
						 const DoubleVector& strike_
			);


	//	copy constructor
	LAMathCapFloorBlackVol(const LAMathCapFloorBlackVol& rhs);

    // Destructor
	virtual ~LAMathCapFloorBlackVol();

    // Make copy(clone) of this class
    virtual LAMathCapFloorBlackVol* clone() const;
//
//------------------
//
    virtual void getCapletVariance();

    virtual double getVega();
//
//------------------
//
    // Equal operator
	virtual LAMathCapFloorBlackVol& operator=(const LAMathCapFloorBlackVol& rhs);

protected :
private:

    //
    double mBlackVol;

    //
    bool mCloned;

};

//
//--------------------------------------------------------------------------------
//

class LAMathCapFloorVolLMMDiscModel : public LAMathCapFloor
{
public:

    // constructo
    LAMathCapFloorVolLMMDiscModel();

    // constructor
	LAMathCapFloorVolLMMDiscModel(LARatesPathElementCurve& curve0_,
									   LAMathVolatilityLMMDiscModel& CapFloor_LMM_,
									   const DoubleVector& T_fix_,
									   const DoubleVector& T_pay_,
									   const DoubleVector& tau_L,
									   const DoubleVector& tau_
			);

	// constructor
	LAMathCapFloorVolLMMDiscModel(LARatesPathElementCurve& curve0_,
									   LAMathVolatilityLMMDiscModel& CapFloor_LMM_,
									   const DoubleVector& T_fix_,
									   const DoubleVector& T_pay_,
									   const DoubleVector& tau_L,
									   const DoubleVector& tau_,
									   const DoubleVector& strike_
			);

    //	copy constructor
	LAMathCapFloorVolLMMDiscModel(const LAMathCapFloorVolLMMDiscModel& rhs);

    // Destructor
	virtual ~LAMathCapFloorVolLMMDiscModel();

    // Make copy(clone) of this class
    virtual LAMathCapFloorVolLMMDiscModel* clone() const;
//
//------------------
//
    virtual void getCapletVariance();

    virtual double getVega();

//
//------------------
//
    // Equal operator
	virtual LAMathCapFloorVolLMMDiscModel& operator=(const LAMathCapFloorVolLMMDiscModel& rhs);

//
//------------------
//
    // Change parameters of volatility function.
	void setParam(const double Q,
				  const DoubleVector& paramV,
				  const DoubleVector& paramF,
				  const DoubleVector& G
			);

    // Change parameters of volatility function.
    void setParamV(const DoubleVector& paramV);

    // Change parameters of volatility function.
    void setParamF(const DoubleVector& paramF);

    // Change parameters of volatility function.
    void setParamG(const DoubleVector& G);


//
//------------------
//

protected :
private:

    //
    LAMathVolatilityLMMDiscModel* mVolLMM;

    //
    bool mCloned;

};

#endif
