#include "AQLMathCapFloor.h"

#include <algorithm>
#include "AQLModelDynamicsCurve.h"
#include "AQLMathCorrelationLMM.h"
#include "AQLModelUtilities.h"
#include "AQLMathVolatilityLMMDiscModel.h"
#include "AQLMathAnalyticalFormula2.h"
#include "AQLBlackScholesBaseFunc.h"

const double minVol = 0.000001;
//
//--------------------------------------------------------------------------------
//

/*!
	@brief constructor
*/
AQLMathCapFloor::AQLMathCapFloor()
:
mCurve0(0),
mCloned(false)
{
}

/*!
	@brief constructor

	@param[in] curve0_ 
	@param[in] T_fix_
	@param[in] T_pay_
    @param[in] tau_L
    @param[in] tau_
*/
AQLMathCapFloor::AQLMathCapFloor(AQLRatesPathElementCurve& curve0_,
						   const DoubleVector& T_fix_,
						   const DoubleVector& T_pay_,
						   const DoubleVector& tau_L,
						   const DoubleVector& tau_
		)
:
mCurve0(&curve0_),
mT_fix(T_fix_),
mT_pay(T_pay_),
mTau(tau_),
mn(mT_fix.size()),
mStrike(mn, getSwapRate( mT_fix, mT_pay, mTau )),
mCapletVar(mn),
mCloned(false)
{
    size_t n = mT_fix.size();
    if( n != mT_pay.size() ) throw AQLCoreInvalidData("T_fix.size != T_pay.size",__FILE__,__LINE__);
    if( n != mTau.size() ) throw AQLCoreInvalidData("T_fix.size != tau.size",__FILE__,__LINE__);

    // Libor
    mL.resize( n );
    for( size_t i = 0; i < n; ++i )
	{
        mL[i] = getForwardLIBOR(mT_fix[i], tau_L[i], mT_pay[i]);
    }
}

/*!
	@brief constructor

	@param[in] curve0_ 
	@param[in] T_fix_
	@param[in] T_pay_
    @param[in] tau_L
    @param[in] tau_
    @param[in] K_
*/
AQLMathCapFloor::AQLMathCapFloor(AQLRatesPathElementCurve& curve0_,
						   const DoubleVector& T_fix_,
						   const DoubleVector& T_pay_,
						   const DoubleVector& tau_L,
						   const DoubleVector& tau_,
						   const DoubleVector& strike_
		)
:
mCurve0(&curve0_),
mT_fix(T_fix_),
mT_pay(T_pay_),
mTau(tau_),
mn(mT_fix.size()),
mStrike(strike_),
mCapletVar(mn),
mCloned(false)
{
    size_t n = mT_fix.size();
    if( n != mT_pay.size() ) throw AQLCoreInvalidData("mT_fix.size != mT_pay.size",__FILE__,__LINE__);
    if( n != mTau.size() ) throw AQLCoreInvalidData("mT_fix.size != mTau.size",__FILE__,__LINE__);
    if( n != mStrike.size() ) throw AQLCoreInvalidData("mT_fix.size != mTau.size",__FILE__,__LINE__);

    // Libor
    mL.resize( n );
    for( size_t i = 0; i < n; ++i )
	{
        mL[i] = getForwardLIBOR(mT_fix[i], tau_L[i], mT_pay[i]);
    }
}

/*!
	@brief copy constructor
*/
AQLMathCapFloor::AQLMathCapFloor(const AQLMathCapFloor& rhs)
:
mCurve0( rhs.mCurve0 != 0 ? dynamic_cast<AQLRatesPathElementCurve*>(rhs.mCurve0->clone()) : 0 ),
mT_fix(rhs.mT_fix),
mT_pay(rhs.mT_pay),
mTau(rhs.mTau),
mn( mT_fix.size() ),
mL(rhs.mL),
mStrike(rhs.mStrike),
mCapletVar(rhs.mCapletVar),
mCloned(false)
{
}

/*!
	@brief destructor
*/
AQLMathCapFloor::~AQLMathCapFloor()
{
    if(mCloned)
    {
        delete mCurve0;
        mCurve0 = 0;
    }
}

/*!
	@brief Equal operator 
*/
AQLMathCapFloor& AQLMathCapFloor::operator=(const AQLMathCapFloor& rhs)
{
    if( this != &rhs)
    {
        delete mCurve0;
        mCurve0 = rhs.mCurve0 != 0 ? dynamic_cast<AQLRatesPathElementCurve*>(rhs.mCurve0->clone()) : 0;
        mCloned = true;

        mT_fix = rhs.mT_fix;
        mT_pay = rhs.mT_pay;
        mTau = rhs.mTau;
        mn = rhs.mn;

        mL = rhs.mL;
        mStrike = rhs.mStrike;
        mCapletVar = rhs.mCapletVar;
    }
    return *this;
}

/*!
	@brief Get Implide volatility
*/
double AQLMathCapFloor::getStdDev( int sgn )
{
    DoubleVector P0(mn);
    for( size_t i = 0; i < mn; ++i )
    {
        P0[i] = mCurve0->getP(mT_pay[i]);
    }
    double prem = getPV();
	double minPrem = getMinimumPV();

	if (AQLMath::abs(prem) <= AQLMath::abs(minPrem)) return minVol;

	return local::CapFloorImplVol( prem, P0, mL, mTau, mT_fix, mStrike, sgn );
    //return 1.0;
}

/*!
	@brief Get CapFloor PV
*/
double AQLMathCapFloor::getPV( int callfloor_flg )
{
    if( callfloor_flg != 1 && callfloor_flg != -1) throw AQLCoreInvalidData("callfloor_flg != 1 && callfloor_flg != -1",__FILE__,__LINE__);

    getCapletVariance();

    double tmp = 0.0;
    for( size_t i = 0; i < mT_fix.size(); ++i )
    {
		tmp += mTau[i] * mCurve0->getP(mT_pay[i]) * local::BlackFormula( mL[i], sqrt( mCapletVar[i] ), mStrike[i], callfloor_flg );
    }
    return tmp;
}

/*!
	@brief Get CapFloor minimum PV
*/
double AQLMathCapFloor::getMinimumPV( int callfloor_flg )
{
    if( callfloor_flg != 1 && callfloor_flg != -1) throw AQLCoreInvalidData("callfloor_flg != 1 && callfloor_flg != -1",__FILE__,__LINE__);

    double tmp = 0.0;
    for( size_t i = 0; i < mT_fix.size(); ++i )
    {
		tmp += mTau[i] * mCurve0->getP(mT_pay[i]) * local::BlackFormula( mL[i], minVol, mStrike[i], callfloor_flg );
    }
    return tmp;
}

//
//--------------------------------------------------------------------------------
//

/*!
	@brief function to get Forward LIBOR 
	@param[in] T_fix
	@param[in] term_rate
	@param[in] T_pay
*/
double AQLMathCapFloor::getForwardLIBOR( double T_fix, double term_rate, double T_pay ) const
{
    if( T_pay < T_fix )
    {
        AQLString msg("T_pay < T_fix");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    if( T_fix < mCurve0->get_t() )
    {
        AQLString msg("T_fix < curve0->get_t()");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    if ( term_rate < 0.0 )
    {
        AQLString msg("term_rate < 0.0");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    if ( T_fix < 0.0 )
    {
        AQLString msg("T_fix < 0.0");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    if ( T_pay < 0.0 )
    {
        AQLString msg("T_pay < 0.0");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    if ( AQLModelUtilities::eq(term_rate, 0.0, eps_1d) && AQLModelUtilities::eq(T_fix, T_pay, eps_1d) )
    {
        AQLString msg("eq(term_rate, 0.0, eps_1d) && eq(T_fix, T_pay, eps_1d)");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    if (( AQLModelUtilities::eq(term_rate, 0.0, eps_1d) && !AQLModelUtilities::eq(T_fix, T_pay, eps_1d) ) ||
		   	( !AQLModelUtilities::eq(term_rate, 0., eps_1d) && AQLModelUtilities::eq(T_fix, T_pay, eps_1d) ))
    {
        AQLString msg("( eq(term_rate, 0.0, eps_1d) && !eq(T_fix, T_pay, eps_1d) ) || ( !eq(term_rate, 0., eps_1d) && eq(T_fix, T_pay, eps_1d) )");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

	double rate = ( (mCurve0->getP(T_fix) / mCurve0->getP(T_pay)) - 1 ) / term_rate;
#ifdef ZEROFLOOR
	rate = AQLMath::max(rate, MIN_RATE);
#endif
    return rate;
}

/*!
	@brief function to get Forward Rate Value
	@param[in] T_fix
	@param[in] T_pay
*/
double AQLMathCapFloor::getForwardRateValue( double T_fix, double T_pay ) const
{
    if ( T_fix < 0.0 )
    {
        AQLString msg("T_fix < 0.0");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    if ( T_pay < 0.0 )
    {
        AQLString msg("T_pay < 0.0");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    if( T_pay < T_fix )
    {
        AQLString msg("T_pay < T_fix");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    if ( T_fix < mCurve0->get_t() ) return 0.0;

    return mCurve0->getP(T_fix) - mCurve0->getP(T_pay);
}

/*!
	@brief function to get FRA
	@param[in] T_fix
	@param[in] T_pay
    @param[in] term_acc
    @param[in] fix_rate
*/
double AQLMathCapFloor::getFRA( double T_fix, double T_pay, double term_acc, double fix_rate ) const
{
    return fix_rate * term_acc * mCurve0->getP( T_pay ) - getForwardRateValue( T_fix, T_pay );
}

/*!
	@brief function to get SwapValue
	@param[in] T_fix
	@param[in] T_pay
    @param[in] term_acc
    @param[in] swap_rate
*/
double AQLMathCapFloor::getSwapValue( const DoubleVector& T_fix, const DoubleVector& T_pay, const DoubleVector& term, double swap_rate ) const
{
    return getSwapFloat( T_fix, T_pay ) - swap_rate * getAnnuity( T_pay, term );
}

/*!
	@brief function to get SwapRate
	@param[in] T_fix
	@param[in] T_pay
    @param[in] term_acc
*/
double AQLMathCapFloor::getSwapRate( const DoubleVector& T_fix, const DoubleVector& T_pay, const DoubleVector& term_acc ) const
{
    if( T_fix[0] < mCurve0->get_t() )
    {
        AQLString msg("T_fix < curve0->get_t()");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    size_t n = T_fix.size();
    if ( T_pay.size() != n )
    {
        AQLString msg("T_pay.size() != T_fix.size()");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    double annuity = getAnnuity( T_pay, term_acc );
    if ( annuity < 0.0 || AQLModelUtilities::eq(annuity,0.0) )
    {
        AQLString msg("annuity < 0 || eq(annuity,0.)");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

	double rate = getSwapFloat( T_fix, T_pay ) / annuity;
#ifdef ZEROFLOOR
	rate = AQLMath::max(rate, MIN_RATE);
#endif
    return rate;
}

/*!
	@brief function to get Swap Float
	@param[in] T_fix
	@param[in] T_pay
*/
double AQLMathCapFloor::getSwapFloat( const DoubleVector& T_fix, const DoubleVector& T_pay) const
{
    size_t n = T_fix.size();
    if ( T_pay.size() != n )
    {
        AQLString msg("T_pay.size() != T_fix.size()");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    double swap_Float = 0.0;
    for( size_t i = 0; i < n; i++ )
    {
        if ( T_fix[i] < 0.0 )
        {
            AQLString msg("T_fix[i] < 0.0");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
        if ( T_pay[i] < 0.0 )
        {
            AQLString msg("T_pay[i] < 0.0");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
        if ( T_pay[i] < T_fix[i]  )
        {
            AQLString msg("T_pay[i] < T_fix[i] ");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        swap_Float += getForwardRateValue( T_fix[i], T_pay[i] );
    }
    return swap_Float;
}

/*!
	@brief function to get Annuity
	@param[in] T_pay
	@param[in] term_acc
*/
double AQLMathCapFloor::getAnnuity( const DoubleVector& T_pay, const DoubleVector& term_acc ) const
{
    if ( T_pay.front() < mCurve0->get_t() )
    {
        AQLString msg("T_pay.front() < mCurve0->get_t()");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    size_t n = T_pay.size();

    if ( term_acc.size() != n )
    {
        AQLString msg("term_acc.size() != n");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    double temp_Anuity = 0.0;
    for( size_t i = 0; i < n; i++ )
    {
        if ( term_acc[i] < 0.0 )
        {
            AQLString msg("term_acc[i] < 0.0");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
        if ( T_pay[i] < 0.0 )
        {
            AQLString msg("T_pay[i] < 0.0");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
        temp_Anuity += term_acc[i] * mCurve0->getP( T_pay[i] );
    }

    return temp_Anuity;
}

//
//--------------------------------------------------------------------------------
//

/*!
	@brief constructor
*/
AQLMathCapFloorBlackVol::AQLMathCapFloorBlackVol()
:
AQLMathCapFloor(),
mBlackVol(0.0),
mCloned(false)
{
}

/*!
	@brief constructor

	@param[in] curve0_
    @param[in] black vol
    @param[in] T_pay_
    @param[in] tau_L
    @param[in] tau_
*/
AQLMathCapFloorBlackVol::AQLMathCapFloorBlackVol(AQLRatesPathElementCurve& curve0_,
										   double black_vol_,
										   const DoubleVector& T_fix_,
										   const DoubleVector& T_pay_,
										   const DoubleVector& tau_L,
										   const DoubleVector& tau_
		)
:
AQLMathCapFloor(curve0_, T_fix_, T_pay_, tau_L, tau_),
mBlackVol(black_vol_),
mCloned(false)
{
}

/*!
	@brief constructor

	@param[in] curve0_
    @param[in] black vol
    @param[in] T_pay_
    @param[in] tau_L
    @param[in] tau_
    @param[in] strike_

*/
AQLMathCapFloorBlackVol::AQLMathCapFloorBlackVol(AQLRatesPathElementCurve& curve0_,
										   double black_vol_,
										   const DoubleVector& T_fix_,
										   const DoubleVector& T_pay_,
										   const DoubleVector& tau_L,
										   const DoubleVector& tau_,
										   const DoubleVector& strike_
		)
:
AQLMathCapFloor(curve0_, T_fix_, T_pay_, tau_L, tau_, strike_),
mBlackVol(black_vol_),
mCloned(false)
{
}

/*!
	@brief copy constructor
*/
AQLMathCapFloorBlackVol::AQLMathCapFloorBlackVol(const AQLMathCapFloorBlackVol& rhs)
:
AQLMathCapFloor(rhs),
mBlackVol(rhs.mBlackVol)
{
}

/*!
	@brief destructor
*/
AQLMathCapFloorBlackVol::~AQLMathCapFloorBlackVol()
{
}

//
//------------------
//

AQLMathCapFloorBlackVol& AQLMathCapFloorBlackVol::operator=(const AQLMathCapFloorBlackVol& rhs)
{
	if( this != &rhs )
	{
        (*this).AQLMathCapFloor::operator = (rhs);
        mBlackVol = rhs.mBlackVol;
	}
	return *this;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLMathCapFloorBlackVol* AQLMathCapFloorBlackVol::clone() const
{
    return new AQLMathCapFloorBlackVol(*this);
}

void AQLMathCapFloorBlackVol::getCapletVariance()
{ 
    for(size_t i = 0; i < mn; ++i)
    {
        mCapletVar[i] = mBlackVol * mBlackVol * mT_fix[i];
    }
}

/*!
    @brief Get vega of this cap/floor
    @return vega of cap/floor
*/
double AQLMathCapFloorBlackVol::getVega()
{
    if( mStrike.size() != mn ) throw AQLCoreInvalidData("strike.size() != T_fix.size() : CapFloor::Get_PV",__FILE__,__LINE__);

    double tmp = 0;
    for(size_t t_ = 0; t_ < mn; ++t_)
    {
		tmp += mCurve0->getP( mT_pay[t_] ) * local::FW_Vega( mL[t_], mBlackVol, mT_fix[t_], mStrike[t_] );
    }
    return tmp;
};

//
//--------------------------------------------------------------------------------
//

/*!
	@brief constructor
*/
AQLMathCapFloorVolLMMDiscModel::AQLMathCapFloorVolLMMDiscModel()
:
AQLMathCapFloor(),
mVolLMM(0),
mCloned(false)
{
}

/*!
	@brief constructor

	@param[in] curve0_
    @param[in] vol_LMM_
    @param[in] T_pay_
    @param[in] tau_L
    @param[in] tau_
*/
AQLMathCapFloorVolLMMDiscModel::AQLMathCapFloorVolLMMDiscModel(AQLRatesPathElementCurve& curve0_,
														 AQLMathVolatilityLMMDiscModel& vol_LMM_,
														 const DoubleVector& T_fix_,
														 const DoubleVector& T_pay_,
														 const DoubleVector& tau_L,
														 const DoubleVector& tau_
		)
:
AQLMathCapFloor(curve0_, T_fix_, T_pay_, tau_L, tau_),
mVolLMM(&vol_LMM_),
mCloned(false)
{
}

/*!
	@brief constructor

	@param[in] curve0_
    @param[in] vol_LMM_
    @param[in] T_pay_
    @param[in] tau_L
    @param[in] tau_
    @param[in] strike_
*/
AQLMathCapFloorVolLMMDiscModel::AQLMathCapFloorVolLMMDiscModel(AQLRatesPathElementCurve& curve0_,
														 AQLMathVolatilityLMMDiscModel& vol_LMM_,
														 const DoubleVector& T_fix_,
														 const DoubleVector& T_pay_,
														 const DoubleVector& tau_L,
														 const DoubleVector& tau_,
														 const DoubleVector& strike_
		)
:
AQLMathCapFloor(curve0_, T_fix_, T_pay_, tau_L, tau_, strike_),
mVolLMM(&vol_LMM_),
mCloned(false)
{
}

/*!
	@brief copy constructor
*/
AQLMathCapFloorVolLMMDiscModel::AQLMathCapFloorVolLMMDiscModel(const AQLMathCapFloorVolLMMDiscModel& rhs)
:
AQLMathCapFloor(rhs),
mVolLMM( rhs.mVolLMM != 0 ? rhs.mVolLMM->clone() : 0 ),
mCloned(true)
{
}

/*!
	@brief destructor
*/
AQLMathCapFloorVolLMMDiscModel::~AQLMathCapFloorVolLMMDiscModel()
{
    if(mCloned)
    {
        delete mVolLMM;
        mVolLMM = 0;
    }
}

//
//------------------
//

AQLMathCapFloorVolLMMDiscModel& AQLMathCapFloorVolLMMDiscModel::operator=(const AQLMathCapFloorVolLMMDiscModel& rhs)
{
	if( this != &rhs )
	{
        (*this).AQLMathCapFloor::operator = (rhs);

        delete mVolLMM;
        mVolLMM = rhs.mVolLMM != 0 ? rhs.mVolLMM->clone() : 0;

        mCloned = true;
	}
	return *this;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLMathCapFloorVolLMMDiscModel* AQLMathCapFloorVolLMMDiscModel::clone() const
{
    return new AQLMathCapFloorVolLMMDiscModel(*this);
}

//
//------------------
//

/*!
	@brief function to get Variance of CapFloor
*/
void AQLMathCapFloorVolLMMDiscModel::getCapletVariance()
{
    for(size_t t_ = 0; t_ < mn; ++t_)
    {
        double t = mT_fix[t_];
        DoubleVector T_vol = mVolLMM->getTGrid();

        size_t i = upper_bound(T_vol.begin(), T_vol.end(), t) - T_vol.begin();
        if( i != 0 && i < T_vol.size() ) i = AQLModelUtilities::eq(t, T_vol[i], eps_1W) ? i : --i;

        mCapletVar[t_] = mVolLMM->integrateSQ0(t, i);
    }
}

/*!
	@brief function to get Vega of CapFloor
*/
double AQLMathCapFloorVolLMMDiscModel::getVega()
{
    double tmp = 0;
    for(size_t t_ = 0; t_ < mn; ++t_)
    {
        double sigma = sqrt( mCapletVar[t_] / mT_fix[t_] );
		tmp += mCurve0->getP( mT_pay[t_] ) * local::FW_Vega( mL[t_], sigma, mT_fix[t_], mStrike[t_] );
    }
    return tmp;
}

//
//------------------
//
/*!
	@brief fuction to change parameters of volatility function.
	@param[in] paramV
	@param[in] paramF
	@param[in] G
*/
void AQLMathCapFloorVolLMMDiscModel::setParam(const double Q_,
										   const DoubleVector& paramV_,
										   const DoubleVector& paramF_,
										   const DoubleVector& G_
		)
{
    if( mVolLMM->getParamV() != paramV_ ||  mVolLMM->getParamF() != paramF_ || mVolLMM->getG() != G_)
    {
        mVolLMM->setParam(paramV_, paramF_, G_);
    }
}

/*!
	@brief fuction to change parameters of volatility function.
	@param[in] paramV
*/
void AQLMathCapFloorVolLMMDiscModel::setParamV(const DoubleVector& paramV_)
{
    if( mVolLMM->getParamV() != paramV_ )
    {
        mVolLMM->setParamV(paramV_);
    }
}

/*!
	@brief fuction to change parameters of volatility function.
	@param[in] paramF
*/
void AQLMathCapFloorVolLMMDiscModel::setParamF(const DoubleVector& paramF_)
{
    if( mVolLMM->getParamF() != paramF_)
    {
        mVolLMM->setParamF(paramF_);
    }
}

/*!
	@brief fuction to change parameters of volatility function.
	@param[in] G
*/
void AQLMathCapFloorVolLMMDiscModel::setParamG(const DoubleVector& G_)
{
    if( mVolLMM->getG() != G_)
    {
        mVolLMM->setParamG(G_);
    }
}
