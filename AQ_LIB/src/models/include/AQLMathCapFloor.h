#ifndef AQLMathCapFloor_h
#define AQLMathCapFloor_h

//#include "AQLFunctionBase.h>
#include <AQLCoreTemplateType.h>

#include <vector>

class AQLMathPathEntity;
class AQLMathVolatilityLMMDiscModel;
class AQLRatesPathElementCurve;

using namespace std;

//
//--------------------------------------------------------------------------------
//

class AQLMathCapFloor
{
public:

    // constructor
    AQLMathCapFloor();

    // constructor
	AQLMathCapFloor(AQLRatesPathElementCurve& curve0_,
				 const DoubleVector& T_fix_,
				 const DoubleVector& T_pay_,
				 const DoubleVector& tau_L,
				 const DoubleVector& tau_
			);

    // constructor
	AQLMathCapFloor(AQLRatesPathElementCurve& curve0_,
				 const DoubleVector& T_fix_,
				 const DoubleVector& T_pay_,
				 const DoubleVector& tau_L,
				 const DoubleVector& tau_,
				 const DoubleVector& strike_
			);

	//	copy constructor
	AQLMathCapFloor(const AQLMathCapFloor& rhs);

    // Destructor
	virtual ~AQLMathCapFloor();

    // Make copy(clone) of this class
    virtual AQLMathCapFloor* clone() const = 0;
//
//------------------
//
    // Equal operator
	virtual AQLMathCapFloor& operator=(const AQLMathCapFloor& rhs);

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
    AQLRatesPathElementCurve* mCurve0;

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

class AQLMathCapFloorBlackVol : public AQLMathCapFloor
{
public:

    // constructo
    AQLMathCapFloorBlackVol();

    // constructor
    AQLMathCapFloorBlackVol(AQLRatesPathElementCurve& curve0_,
						 double black_vol_,
						 const DoubleVector& T_fix_,
						 const DoubleVector& T_pay_,
						 const DoubleVector& tau_L,
						 const DoubleVector& tau_
			);

	// constructor
	AQLMathCapFloorBlackVol(AQLRatesPathElementCurve& curve0_,
						 double black_vol_,
						 const DoubleVector& T_fix_,
						 const DoubleVector& T_pay_,
						 const DoubleVector& tau_L,
						 const DoubleVector& tau_,
						 const DoubleVector& strike_
			);


	//	copy constructor
	AQLMathCapFloorBlackVol(const AQLMathCapFloorBlackVol& rhs);

    // Destructor
	virtual ~AQLMathCapFloorBlackVol();

    // Make copy(clone) of this class
    virtual AQLMathCapFloorBlackVol* clone() const;
//
//------------------
//
    virtual void getCapletVariance();

    virtual double getVega();
//
//------------------
//
    // Equal operator
	virtual AQLMathCapFloorBlackVol& operator=(const AQLMathCapFloorBlackVol& rhs);

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

class AQLMathCapFloorVolLMMDiscModel : public AQLMathCapFloor
{
public:

    // constructo
    AQLMathCapFloorVolLMMDiscModel();

    // constructor
	AQLMathCapFloorVolLMMDiscModel(AQLRatesPathElementCurve& curve0_,
									   AQLMathVolatilityLMMDiscModel& CapFloor_LMM_,
									   const DoubleVector& T_fix_,
									   const DoubleVector& T_pay_,
									   const DoubleVector& tau_L,
									   const DoubleVector& tau_
			);

	// constructor
	AQLMathCapFloorVolLMMDiscModel(AQLRatesPathElementCurve& curve0_,
									   AQLMathVolatilityLMMDiscModel& CapFloor_LMM_,
									   const DoubleVector& T_fix_,
									   const DoubleVector& T_pay_,
									   const DoubleVector& tau_L,
									   const DoubleVector& tau_,
									   const DoubleVector& strike_
			);

    //	copy constructor
	AQLMathCapFloorVolLMMDiscModel(const AQLMathCapFloorVolLMMDiscModel& rhs);

    // Destructor
	virtual ~AQLMathCapFloorVolLMMDiscModel();

    // Make copy(clone) of this class
    virtual AQLMathCapFloorVolLMMDiscModel* clone() const;
//
//------------------
//
    virtual void getCapletVariance();

    virtual double getVega();

//
//------------------
//
    // Equal operator
	virtual AQLMathCapFloorVolLMMDiscModel& operator=(const AQLMathCapFloorVolLMMDiscModel& rhs);

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
    AQLMathVolatilityLMMDiscModel* mVolLMM;

    //
    bool mCloned;

};

#endif
