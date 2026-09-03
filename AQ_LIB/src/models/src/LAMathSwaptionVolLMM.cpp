
//stl
#include <algorithm>
#include <numeric>

//
#include "LAModelDynamicsCurve.h"

//
#include "LAMathSwaptionVolLMM.h"
#include "LAMathCorrelationLMM.h"
//#include "util.h"
#include "LAMathVolatilityLMMDiscModel.h"
#include "LABlackScholesBaseFunc.h"
#include "LAMathAnalyticalFormula2.h"
#include "LAMathYieldCurve.h"

#define MIN_RATE_LMMCALIB 0.0000001
#define CONSTANT_SHIFT 0.01

//#define TIMER
//#include "Timer.h"
//extern Timer timer;
//
//--------------------------------------------------------------------------------
//

/*!
	@brief constructor
*/
LAMathSwaptionVolLMM::LAMathSwaptionVolLMM()
:
mCurve0_F(0),
mCurve0_D(0),
mCloned(false)
{
}

/*!
	@brief constructor

	@param[in] curve0_F_ :Forecast curve
	@param[in] curve0_D_  :Discount curve
	@param[in] T_fix_L_ :fixing time for Libor
	@param[in] T_pay_L_ :payment time for Libor
    @param[in] tau_L_ :tau for Libor
	@param[in] T_pay_ :payment time for Swap
    @param[in] tau_ :tau for Swap
    @param[in] T_OptMat_ :option maturity
    @param[in] T_Expire_ :underlying expiration time
    @param[in] volType :volatility type
*/
LAMathSwaptionVolLMM::LAMathSwaptionVolLMM( LARatesPathElementCurve& curve0_F_,
										LARatesPathElementCurve& curve0_D_,
										const DoubleVector& T_fix_L_,
										const DoubleVector& T_pay_L_,
										const DoubleVector& tau_L_,
										const DoubleVector& T_pay_,
										const DoubleVector& tau_,
										double T_OptMat_,
										double T_Expire_,
										volType volType_
										)
:
mCurve0_F(&curve0_F_),
mCurve0_D(&curve0_D_),
mT_fix_L(T_fix_L_),
mT_pay_L(T_pay_L_),
mTau_L(tau_L_),
mT_pay(T_pay_),
mTau(tau_),
mT_OptMat(T_OptMat_),
mT_Expire(T_Expire_),
mVolType(volType_),
mCloned(false)
{
    size_t n = mT_fix_L.size();
    if( n != mT_pay_L.size() ) throw LACoreInvalidData("mT_fix_L.size != mT_pay_L.size",__FILE__,__LINE__);
    if( n != mTau_L.size() ) throw LACoreInvalidData("mT_fix_L.size != mTau_L.size",__FILE__,__LINE__);
    size_t m = mT_pay.size();
    if( m != mTau.size() ) throw LACoreInvalidData("mT_pay.size != mTau.size",__FILE__,__LINE__);
	mL2SNum = n / m;

    if(mT_Expire <= mT_OptMat)
	{
        throw LACoreInvalidData("mT_Expire <= mT_OptMat",__FILE__,__LINE__);
	}

    //T itr for Libor
    mItr_OptMat_L = --upper_bound(mT_fix_L.begin(), mT_fix_L.end(), mT_OptMat) - mT_fix_L.begin();
    if( mItr_OptMat_L < n - 1 ) mItr_OptMat_L = LAModelUtilities::eq(mT_OptMat, mT_fix_L[mItr_OptMat_L + 1], eps_1W) ? mItr_OptMat_L + 1 : mItr_OptMat_L;
	
    mItr_Expire_L = --upper_bound(mT_pay_L.begin(), mT_pay_L.end(), mT_Expire) - mT_pay_L.begin();
    if( mItr_Expire_L < n - 1 ) mItr_Expire_L = LAModelUtilities::eq(mT_Expire, mT_pay_L[mItr_Expire_L + 1], eps_1W) ? mItr_Expire_L + 1 : mItr_Expire_L;
    if( mItr_Expire_L < mItr_OptMat_L ) mItr_Expire_L = mItr_OptMat_L;

    //T itr for Swap
    size_t itr_OptMat = upper_bound(mT_pay.begin(), mT_pay.end(), mT_OptMat) - mT_pay.begin();
    if( itr_OptMat < m - 1 ) itr_OptMat = LAModelUtilities::eq(mT_OptMat, mT_pay[itr_OptMat + 1], eps_1W) ? itr_OptMat + 1 : itr_OptMat;

	size_t itr_Expire = --upper_bound(mT_pay.begin(), mT_pay.end(), mT_Expire) - mT_pay.begin();
    if( itr_Expire < m - 1 ) itr_Expire = LAModelUtilities::eq(mT_Expire, mT_pay[itr_Expire + 1], eps_1W) ? itr_Expire + 1 : itr_Expire;
    if( itr_Expire < itr_OptMat ) itr_Expire = itr_OptMat;

	//Annuity
    mAnnuity = getAnnuity( DoubleVector( mT_pay.begin() + itr_OptMat, mT_pay.begin() + itr_Expire + 1 ),
						   DoubleVector( mTau.begin()   + itr_OptMat, mTau.begin()   + itr_Expire + 1 )
						 );

	//Swap rate
	mS0 = getSwapRate( DoubleVector( mT_fix_L.begin() + mItr_OptMat_L, mT_fix_L.begin() + mItr_Expire_L + 1 ),
					   DoubleVector( mT_pay_L.begin() + mItr_OptMat_L, mT_pay_L.begin() + mItr_Expire_L + 1 ),
					   DoubleVector( mT_pay.begin() + itr_OptMat, mT_pay.begin() + itr_Expire + 1 ),
					   DoubleVector( mTau.begin()   + itr_OptMat, mTau.begin()   + itr_Expire + 1 )
					 );

    // Libor & weight
    mL_F.resize( mItr_Expire_L + 1 - mItr_OptMat_L );
    mL_D.resize( mItr_Expire_L + 1 - mItr_OptMat_L );
    for( size_t i = mItr_OptMat_L; i < mItr_Expire_L + 1; ++i )
	{
        mL_F[i] = getForwardLIBOR( mCurve0_F, mT_fix_L[i], mT_pay_L[i], mTau_L[i]);
        mL_D[i] = getForwardLIBOR( mCurve0_D, mT_fix_L[i], mT_pay_L[i], mTau_L[i]);
    }
	if (dynamic_cast<const LAMathPathYieldCurve&>(*mCurve0_F) == dynamic_cast<const LAMathPathYieldCurve&>(*mCurve0_D) && mTau_L.size() == mTau.size())
	{
		// This is a temporary setting now(2016/06/13). Please remove it when EOD setting is changed to dual curve.
		mTau_L = mTau;
	}
    // calculate weight of Libors
    mWeight.resize( mItr_Expire_L + 1 - mItr_OptMat_L );
    for( size_t i = mItr_OptMat_L; i < mItr_Expire_L + 1; ++i )
	{
        mWeight[i] = mTau_L[i] * mCurve0_D->getP( mT_pay_L[i] ) / mAnnuity;
    }
}

/*!
	@brief copy constructor
*/
LAMathSwaptionVolLMM::LAMathSwaptionVolLMM(const LAMathSwaptionVolLMM& rhs)
:
mCurve0_F( rhs.mCurve0_F != 0 ? dynamic_cast<LARatesPathElementCurve*>(rhs.mCurve0_F->clone()) : 0 ),
mCurve0_D( rhs.mCurve0_D != 0 ? dynamic_cast<LARatesPathElementCurve*>(rhs.mCurve0_D->clone()) : 0 ),
mT_fix_L(rhs.mT_fix_L),
mT_pay_L(rhs.mT_pay_L),
mTau_L(rhs.mTau_L),
mTau(rhs.mTau),
mL2SNum(rhs.mL2SNum),
mT_OptMat(rhs.mT_OptMat),
mItr_OptMat_L(rhs.mItr_OptMat_L),
mT_Expire(rhs.mT_Expire),
mItr_Expire_L(rhs.mItr_Expire_L),
mL_F(rhs.mL_F),
mWeight(rhs.mWeight),
mAnnuity(rhs.mAnnuity),
mS0(rhs.mS0),
mVolType(rhs.mVolType),
mCloned(false)
{
}

/*!
	@brief destructor
*/
LAMathSwaptionVolLMM::~LAMathSwaptionVolLMM()
{
    if(mCloned)
    {
        delete mCurve0_F;
        delete mCurve0_D;
        mCurve0_F = 0;
        mCurve0_D = 0;
    }
}

/*!
	@brief Equal operator 
*/
LAMathSwaptionVolLMM& LAMathSwaptionVolLMM::operator=(const LAMathSwaptionVolLMM& rhs)
{
    if( this != &rhs)
    {
        delete mCurve0_F; 
        delete mCurve0_D; 
        mCurve0_F = rhs.mCurve0_F != 0 ? dynamic_cast<LARatesPathElementCurve*>(rhs.mCurve0_F->clone()) : 0;
        mCurve0_D = rhs.mCurve0_D != 0 ? dynamic_cast<LARatesPathElementCurve*>(rhs.mCurve0_D->clone()) : 0;

        mT_fix_L = rhs.mT_fix_L;
        mT_pay_L = rhs.mT_pay_L;
        mTau_L = rhs.mTau_L;
        mT_pay = rhs.mT_pay;
        mTau = rhs.mTau;
        mL2SNum = rhs.mL2SNum;

        mT_OptMat = rhs.mT_OptMat;
        mItr_OptMat_L = rhs.mItr_OptMat_L;
        mT_Expire = rhs.mT_Expire;
        mItr_Expire_L = rhs.mItr_Expire_L;

        mL_F = rhs.mL_F;
        mL_D = rhs.mL_D;
        mWeight = rhs.mWeight;
        mAnnuity = rhs.mAnnuity;
        mS0 = rhs.mS0;

		mVolType = rhs.mVolType;

        mCloned = true;
    }
    return *this;
}

/*!
	@brief function to get Swaption volatility 
	@param[in] T_fix
	@param[in] term_rate
	@param[in] T_pay
*/
double LAMathSwaptionVolLMM::getSwaptionVol()
{
    return sqrt( getVar() / mT_OptMat );
}

//
//--------------------------------------------------------------------------------
//

/*!
	@brief function to get Forward LIBOR 
	@param[in] T_fix
	@param[in] T_pay
	@param[in] tau
*/
double LAMathSwaptionVolLMM::getForwardLIBOR( const LARatesPathElementCurve* curve, double T_fix, double T_pay, double tau ) const
{
    if( T_pay < T_fix )
    {
        LAString msg("T_pay < T_fix");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);        
    }
    if( T_fix < mCurve0_F->get_t() )
    {
        LAString msg("T_fix < mCurve0_F->get_t()");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);        
    }
    if ( tau < 0.0 )
    {
        LAString msg("term_rate < 0.0");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);   
    }
    if ( T_fix < 0.0 )
    {
        LAString msg("T_fix < 0.0");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);   
    }
    if ( T_pay < 0.0 )
    {
        LAString msg("T_pay < 0.0");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);   
    }
    if ( LAModelUtilities::eq(tau, 0.0, eps_1d) && LAModelUtilities::eq(T_fix, T_pay, eps_1d) )
    {
        LAString msg("eq(term_rate, 0.0, eps_1d) && eq(T_fix, T_pay, eps_1d)");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);   
    }
    if (( LAModelUtilities::eq(tau, 0.0, eps_1d) && !LAModelUtilities::eq(T_fix, T_pay, eps_1d) ) ||
		   	( !LAModelUtilities::eq(tau, 0.0, eps_1d) && LAModelUtilities::eq(T_fix, T_pay, eps_1d) ))
    {
        LAString msg("( eq(term_rate, 0.0, eps_1d) && !eq(T_fix, T_pay, eps_1d) ) || ( !eq(term_rate, 0.0, eps_1d) && eq(T_fix, T_pay, eps_1d) )");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);   
    }

	double rate = ( (curve->getP(T_fix) / curve->getP(T_pay)) - 1.0 ) / tau;

//#ifdef ZEROFLOOR
	//rate = LAMath::max(rate, MIN_RATE);
//#endif
	//if ( MIN_RATE_LMMCALIB >= rate && rate >= 0. )
	//{
	//	rate = MIN_RATE_LMMCALIB;
	//}
	//else if ( 0. >= rate && rate >= - MIN_RATE_LMMCALIB )
	//{
	//	rate = - MIN_RATE_LMMCALIB;
	//}
    return rate;
}

/*!
	@brief function to get Forward Rate Value
	@param[in] T_fix
	@param[in] T_pay
*/
double LAMathSwaptionVolLMM::getForwardRateValue( const LARatesPathElementCurve* curve, double T_fix, double T_pay ) const
{
    if ( T_fix < 0.0 )
    {
        LAString msg("T_fix < 0.0");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);   
    }
    if ( T_pay < 0.0 )
    {
        LAString msg("T_pay < 0.0");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);   
    }
    if( T_pay < T_fix )
    {
        LAString msg("T_pay < T_fix");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);        
    }

    if ( T_fix < curve->get_t() ) return 0.0;

	return curve->getP(T_fix) - curve->getP(T_pay); 
}

/*!
	@brief function to get FRA
	@param[in] T_fix_L
	@param[in] T_pay_L
    @param[in] tau_L
    @param[in] fix_rate
*/
double LAMathSwaptionVolLMM::getFRA( double T_fix_L, double T_pay_L, double tau_L, double fix_rate ) const
{
    return fix_rate * tau_L * mCurve0_F->getP( T_pay_L ) - getForwardRateValue( mCurve0_F, T_fix_L, T_pay_L );
}

/*!
	@brief function to get Annuity
	@param[in] T_pay
	@param[in] tau
*/
double LAMathSwaptionVolLMM::getAnnuity( const DoubleVector& T_pay, const DoubleVector& tau ) const
{
    if ( T_pay.front() < mCurve0_D->get_t() )
    {
        LAString msg("T_pay.front() < mCurve0_D->get_t()");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__); 
    }

    size_t n = T_pay.size();

    if ( tau.size() != n )
    {
        LAString msg("term_acc.size() != n");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__); 
    }

    double temp_Anuity = 0.0;
    for( size_t i = 0; i < n; i++ ) 
    {
        if ( tau[i] < 0.0 )
        {
            LAString msg("term_acc[i] < 0.0");
	        throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__); 
        }
        if ( T_pay[i] < 0.0 )
        {
            LAString msg("T_pay[i] < 0.0");
	        throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__); 
        }
	    temp_Anuity += tau[i] * mCurve0_D->getP( T_pay[i] );
    }

    return temp_Anuity;
}

/*!
	@brief function to get Swap Float
	@param[in] T_fix_L
	@param[in] T_pay_L
	@param[in] T_pay
*/
double LAMathSwaptionVolLMM::getSwapFloat( const DoubleVector& T_fix_L, const DoubleVector& T_pay_L ) const
{
    size_t n = T_fix_L.size();
    if ( T_pay_L.size() != n )
    {
        LAString msg("T_pay_L.size() != T_fix_L.size()");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);  
    }

    double swap_Float = 0.0;
    for( size_t i = 0; i < n; i++ )
    {
        if ( T_fix_L[i] < 0.0 )
        {
            LAString msg("T_fix_L[i] < 0.0");
	        throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);  
        }
        if ( T_pay_L[i] < 0.0 )
        {
            LAString msg("T_pay_L[i] < 0.0");
	        throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);  
        }
        if ( T_pay_L[i] < T_fix_L[i]  )
        {
            LAString msg("T_pay_L[i] < T_fix_L[i] ");
	        throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);  
        }
        swap_Float += getForwardRateValue( mCurve0_F, T_fix_L[i], T_pay_L[i] ) * mCurve0_D->getP(T_pay_L[i]) / mCurve0_F->getP(T_pay_L[i]);
    }

    return swap_Float;
}

/*!
	@brief function to get SwapValue
	@param[in] T_fix_L
	@param[in] T_pay_L
	@param[in] T_pay
    @param[in] tau
    @param[in] swap_rate
*/
double LAMathSwaptionVolLMM::getSwapValue( const DoubleVector& T_fix_L, const DoubleVector& T_pay_L, const DoubleVector& T_pay, const DoubleVector& tau, double swap_rate ) const
{
    return getSwapFloat( T_fix_L, T_pay_L ) - swap_rate * getAnnuity( T_pay, tau );
}

/*!
	@brief function to get SwapRate
	@param[in] T_fix_L
	@param[in] T_pay_L
	@param[in] T_pay
    @param[in] tau
*/
double LAMathSwaptionVolLMM::getSwapRate( const DoubleVector& T_fix_L, const DoubleVector& T_pay_L, const DoubleVector& T_pay, const DoubleVector& tau ) const
{
    if( T_fix_L[0] < mCurve0_F->get_t() )
    {
        LAString msg("T_fix_L < mCurve0_F->get_t()");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);        
    }

    size_t n = T_fix_L.size();
    if ( T_pay_L.size() != n )
    {
        LAString msg("T_pay_L.size() != T_fix_L.size()");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);        
    }

    double annuity = getAnnuity( T_pay, tau );
    if ( annuity < 0.0 || LAModelUtilities::eq(annuity,0.0) )
    {
        LAString msg("annuity < 0.0 || eq(annuity,0.0)");
	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);  
    }

	double rate = getSwapFloat( T_fix_L, T_pay_L ) / annuity;

    return rate;
}


/*!
	@brief function to get swaption black premium
	@param[in] volatility
*/
double LAMathSwaptionVolLMM::getSwaptionBlackPrem( double vol, double forwardShift ) const
{
	return mAnnuity * local::BlackFormulaDD( mS0 + forwardShift, vol * LAMath::sqrt(mT_OptMat), mS0 + forwardShift, 1, 1 );
}

/*!
	@brief function to get swaption normal distribution premium
	@param[in] volatility
*/
double LAMathSwaptionVolLMM::getSwaptionNormalPrem( double vol ) const
{
	return mAnnuity * local::BlackFormulaDD( mS0, vol * LAMath::sqrt(mT_OptMat), mS0, 1, 0 );
}

//
//--------------------REBONATO_APPROXIMATION--------------------
//

/*!
	@brief constructor
*/
LAMathSwaptionVolLMMDiscModel::LAMathSwaptionVolLMMDiscModel()
:
LAMathSwaptionVolLMM(),
mCorrLMM(0),
mCorrCached(false),
mVolLMM(0),
mVolCached(false),
mT_VolCached(false),
mCloned(false)
{
}

/*!
	@brief constructor

	@param[in] curve0_F_ 
	@param[in] curve0_D_ 
	@param[in] corr_LMM_
	@param[in] vol_LMM_
	@param[in] T_fix_
	@param[in] T_pay_
    @param[in] tau_L_
    @param[in] tau_
*/
LAMathSwaptionVolLMMDiscModel::LAMathSwaptionVolLMMDiscModel( LARatesPathElementCurve& curve0_F_,
														LARatesPathElementCurve& curve0_D_,
														LAMathCorrelationLMM& corr_LMM_,
														LAMathVolatilityLMMDiscModel& vol_LMM_,
														const DoubleVector& T_fix_L_,
														const DoubleVector& T_pay_L_,
														const DoubleVector& tau_L_,
														const DoubleVector& T_pay_,
														const DoubleVector& tau_,
														double T_OptMat_,
														double T_Expire_,
														volType volType_
														)
:
LAMathSwaptionVolLMM(curve0_F_, curve0_D_, T_fix_L_, T_pay_L_, tau_L_, T_pay_, tau_, T_OptMat_, T_Expire_, volType_),
mCorrLMM(&corr_LMM_),
mCorrCached(false),
mVolLMM(&vol_LMM_),
mVolCached(false),
mT_VolCached(false),
mCloned(false)
{
}

/*!
	@brief copy constructor
*/
LAMathSwaptionVolLMMDiscModel::LAMathSwaptionVolLMMDiscModel(const LAMathSwaptionVolLMMDiscModel& rhs)
:
LAMathSwaptionVolLMM(rhs),
mItrS(rhs.mItrS),
mItrE(rhs.mItrE),
mCorrLMM( rhs.mCorrLMM != 0 ? rhs.mCorrLMM->clone() : 0 ),
mCorrCache(rhs.mCorrCache),
mCorrCached(rhs.mCorrCached),
mVolLMM( rhs.mVolLMM != 0 ? rhs.mVolLMM->clone() : 0 ),
mVolCache(rhs.mVolCache),
mVolCached(rhs.mVolCached),
mT_Vol(rhs.mT_Vol),
mT_VolItr(rhs.mT_VolItr),
mt_Vol(rhs.mt_Vol),
mT_VolCached(rhs.mT_VolCached),
mCloned(true)
{
}

/*!
	@brief destructor
*/
LAMathSwaptionVolLMMDiscModel::~LAMathSwaptionVolLMMDiscModel()
{
    if(mCloned)
    {
        delete mCorrLMM;
	    mCorrLMM = 0;

        delete mVolLMM;
	    mVolLMM = 0;
    }
}

//
//------------------
//

LAMathSwaptionVolLMMDiscModel& LAMathSwaptionVolLMMDiscModel::operator=(const LAMathSwaptionVolLMMDiscModel& rhs)
{
	if( this != &rhs )
	{
        (*this).LAMathSwaptionVolLMM::operator = (rhs);
        
        delete mCorrLMM; mCorrLMM = rhs.mCorrLMM != 0 ? rhs.mCorrLMM->clone() : 0;
        delete mVolLMM; mVolLMM = rhs.mVolLMM != 0 ? rhs.mVolLMM->clone() : 0;
        mCloned = true;

        mItrS = rhs.mItrS;
        mItrE = rhs.mItrE;

        mCorrCache = rhs.mCorrCache;
        mCorrCached = rhs.mCorrCached;

        mVolCache = rhs.mVolCache;
        mVolCached = rhs.mVolCached;
        mT_VolCached = rhs.mT_VolCached;
        mT_Vol = rhs.mT_Vol;
        mT_VolItr = rhs.mT_VolItr;

        mt_Vol = rhs.mt_Vol;
	}
	return *this;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LAMathSwaptionVolLMMDiscModel* LAMathSwaptionVolLMMDiscModel::clone() const
{
    return new LAMathSwaptionVolLMMDiscModel(*this);
}

//
//------------------
//


/*!
	@brief function to get Variance of SR 
	@param[in] T_OptMat   Option Maturity
	@param[in] T_Expire   Expire
*/
double LAMathSwaptionVolLMMDiscModel::getVar()
{
	if( mVolType != Black )
	{
		LAString msg = "Only black vol is supported! : LAMathSwaptionVolLMMDiscModel_DD::getVar";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	double tmp = 0.0;
    for( size_t i = mItr_OptMat_L; i < mItr_Expire_L + 1; ++i )
	{
        tmp += mWeight[i] * mL_F[i] * mWeight[i] * mL_F[i] * getIntegrateCov(i, i, 0.0, mT_OptMat);

		for( size_t j = mItr_OptMat_L; j < i; ++j )
		{
            tmp += 2.0 * mWeight[i] * mL_F[i] * mWeight[j] * mL_F[j] * getIntegrateCov(i, j, 0.0, mT_OptMat);
		}
	}

    return tmp / mS0 / mS0;
}

// Get premium of SR(Maturity, Expire)
double LAMathSwaptionVolLMMDiscModel::getSwaptionPrem()
{
	LAString msg = "TODO Implementation! : LAMathSwaptionVolLMMDiscModel::getSwaptionPrem";
	throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
}

/*!
	@brief fuction to change parameters of volatility function.
	@param[in] Q
	@param[in] paramV  
	@param[in] paramF  
	@param[in] G       
*/
void LAMathSwaptionVolLMMDiscModel::setParam(const DoubleVector& paramV_,
										   const DoubleVector& paramF_,
										   const DoubleVector& G_
		)
{
	if( mVolLMM->getParamV() != paramV_ ||  mVolLMM->getParamF() != paramF_ || mVolLMM->getG() != G_)
    {        
        mVolLMM->setParam(paramV_, paramF_, G_);
    }
    mVolCached = false;
}

/*!
	@brief fuction to change parameters of volatility function.
	@param[in] paramV  
*/
void LAMathSwaptionVolLMMDiscModel::setParamV(const DoubleVector& paramV_)
{
    if( mVolLMM->getParamV() != paramV_ )
    {
        mVolLMM->setParamV(paramV_);
    }
    mVolCached = false;
}

/*!
	@brief fuction to change parameters of volatility function.
	@param[in] paramF  
*/
void LAMathSwaptionVolLMMDiscModel::setParamF(const DoubleVector& paramF_)
{
    if( mVolLMM->getParamF() != paramF_)
    {
        mVolLMM->setParamF(paramF_);
    }
    mVolCached = false;
}

/*!
	@brief fuction to change parameters of volatility function.
	@param[in] G  
*/
void LAMathSwaptionVolLMMDiscModel::setParamG(const DoubleVector& G_)
{
    if( mVolLMM->getG() != G_)
    {
        mVolLMM->setParamG(G_);
    }
    mVolCached = false;
}

/*!
	@brief fuction to get integral of sigma_i & sigma_j
	@param[in] i       itr of T_fix[i]
	@param[in] j       itr of T_fix[j]
    @param[in] t_s     integral range
	@param[in] t_e     integral range

*/
double LAMathSwaptionVolLMMDiscModel::getIntegrateCov( size_t i, size_t  j, double t_s, double t_e)
{
    if( !mCorrCached )
    {
        getCorrCache();
        mCorrCached = true;
    }

    if( !mVolCached ) 
    {
        getVolCache();
        mVolCached = true;
    }

    if( t_e <= mt_Vol[1] )
    { 
        return mCorrCache[i][j] * mVolCache[0][i] * mVolCache[0][j] * (t_e - t_s);
    }

    double tmp = 0.0;
    size_t t;
	for(t = mItrS; t < mItrE; ++t)
	{
        tmp += mVolCache[t][i] * mVolCache[t][j] * ( mt_Vol[t + 1] - mt_Vol[t] );
	}
    if( t < mt_Vol.size() && mt_Vol[mItrE] < t_e )
    {
        tmp += mVolCache[t][i] * mVolCache[t][j] * ( t_e - mt_Vol[mItrE] );
    }

    return mCorrCache[i][j] * tmp;
}

/*!
	@brief fuction to get cache vol matrix
*/
void LAMathSwaptionVolLMMDiscModel::getVolCache()
{
	// set up itr_s, itr_e, T_Vol, T_Vol_itr & t_Vol
    if( !mT_VolCached )
    {
        // Tenor timing
        mT_Vol = mVolLMM->getTGrid();
        mT_VolItr.resize(mItr_Expire_L + 1 - mItr_OptMat_L);
        size_t itr_i;
        for( size_t i = mItr_OptMat_L; i < mItr_Expire_L + 1; ++i )
        {
            itr_i = upper_bound(mT_Vol.begin(), mT_Vol.end(), mT_fix_L[i]) - mT_Vol.begin();
            if( itr_i != 0 && itr_i < mT_Vol.size() ) itr_i = LAModelUtilities::eq(mT_fix_L[i], mT_Vol[itr_i], eps_1W) ? itr_i : --itr_i;
            mT_VolItr[i] = itr_i;
        }

        // Spot timing
        mt_Vol = mVolLMM->getDividedTGrid();
        if( mt_Vol.front() != 0.0 ) mt_Vol.insert(mt_Vol.begin(), 0.0);
    
        // get integral range index(t direction).
        mItrS = 0;
        mItrE = --upper_bound(mt_Vol.begin(), mt_Vol.end(), mT_OptMat) - mt_Vol.begin();
        if( mItrE < mt_Vol.size() - 1 ) mItrE = LAModelUtilities::eq(mT_OptMat, mt_Vol[mItrE + 1], eps_1W) ? mItrE + 1 : mItrE;

        mT_VolCached = true;
    }

	// set up vol_cache
    size_t itr = mItrE + 1 < mt_Vol.size() ? mItrE + 1 : mItrE;
    mVolCache.resize(itr);

    double t_;
    for(size_t t = 0; t < itr; ++t)
    {	
        t_ = mt_Vol[t]; 
        for( size_t i = mItr_OptMat_L; i < mItr_Expire_L + 1; ++i )
        {
            mVolCache[t].resize( mItr_Expire_L + 1 - mItr_OptMat_L );
            mVolCache[t][i] = mVolLMM->get( t_, mT_VolItr[i] );
        }
	}
}

/*!
	@brief fuction to get cache cor matrix
*/
void LAMathSwaptionVolLMMDiscModel::getCorrCache()
{
    DoubleVector T_Corr = mCorrLMM->getTgrid();
    mCorrCache.resize( mItr_Expire_L - mItr_OptMat_L + 1, DoubleVector(mItr_Expire_L - mItr_OptMat_L + 1) );
    
    for( size_t i = mItr_OptMat_L; i < mItr_Expire_L + 1; ++i )
    {
        size_t T_corr_i = --upper_bound(T_Corr.begin(), T_Corr.end(), mT_fix_L[i]) - T_Corr.begin();
        if( T_corr_i < T_Corr.size() - 1 ) T_corr_i = LAModelUtilities::eq(mT_fix_L[i], T_Corr[T_corr_i + 1], eps_1W) ? T_corr_i + 1 : T_corr_i;
        
        for( size_t j = mItr_OptMat_L; j < mItr_Expire_L + 1; ++j )
        {
            size_t T_corr_j = --upper_bound(T_Corr.begin(), T_Corr.end(), mT_fix_L[j]) - T_Corr.begin();
            if( T_corr_j < T_Corr.size() - 1 ) T_corr_j = LAModelUtilities::eq(mT_fix_L[j], T_Corr[T_corr_j + 1], eps_1W) ? T_corr_j + 1 : T_corr_j;

            mCorrCache[i][j] = T_corr_i == T_corr_j ? 1.0 : mCorrLMM->get(0.0, T_corr_i, T_corr_j);
        }
    }
}

//
//--------------------WIENER_ITO_CHAOS_EXPANTION_1D--------------------
//

// constructor
LAMathSwaptionVolLMMDiscModel_DD::LAMathSwaptionVolLMMDiscModel_DD()
{
}

// constructor
LAMathSwaptionVolLMMDiscModel_DD::LAMathSwaptionVolLMMDiscModel_DD(LARatesPathElementCurve& curve0_F_,
															LARatesPathElementCurve& curve0_D_,
														    LAMathCorrelationLMM& corr_LMM_,
															LAMathVolatilityLMMDiscModel& vol_LMM_,
															const DoubleVector& T_fix_L_,
															const DoubleVector& T_pay_L_,
															const DoubleVector& tau_L_,
															const DoubleVector& T_pay_,
															const DoubleVector& tau_,
															double T_OptMat,
															double T_Expire,
															volType volType_,
															double Q
															)
:
LAMathSwaptionVolLMMDiscModel(curve0_F_, curve0_D_, corr_LMM_,vol_LMM_,T_fix_L_,T_pay_L_,tau_L_,T_pay_,tau_,T_OptMat,T_Expire, volType_),
mQ(Q)
{
}

// Get Variance of SR()
double LAMathSwaptionVolLMMDiscModel_DD::getVar()
{
	if( mVolType != Black )
	{
		LAString msg = "Only black vol is supported! : LAMathSwaptionVolLMMDiscModel_DD::getVar";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	double log2_Q = log(mQ) / log(2.);
	double inv_log2_Q_SQ = (1. - log2_Q) * (1. - log2_Q);

	double tmp = 0.;
    for( size_t i = mItr_OptMat_L; i < mItr_Expire_L + 1; ++i )
	{
		double intg_cov = getIntegrateCov(i, i, 0., mT_OptMat);
		double Li_wi_SQ = mWeight[i] * mL_F[i]; Li_wi_SQ *= Li_wi_SQ;
		double WIC_2D = (intg_cov + 0.5 / inv_log2_Q_SQ * intg_cov * intg_cov);

		tmp += Li_wi_SQ * WIC_2D;

		for( size_t j = mItr_OptMat_L; j < i; ++j )
		{
			intg_cov = getIntegrateCov(i, j, 0., mT_OptMat);
			double Li_wi_Lj_wj = mWeight[i] * mWeight[j] * mL_F[i] * mL_F[j];
			double WIC_2D = (intg_cov + 0.5 / inv_log2_Q_SQ * intg_cov * intg_cov);

			tmp += 2. * Li_wi_Lj_wj * WIC_2D;
		}
	}

    return tmp / mS0 / mS0;
}

// Get premium of SR(Maturity, Expire)
double LAMathSwaptionVolLMMDiscModel_DD::getSwaptionPrem()
{
	LAString msg = "TODO Implementation! : LAMathSwaptionVolLMMDiscModel_DD::getSwaptionPrem";
	throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
}

//
//--------------------REBONATO_DD_APPROXIMATION--------------------
//

// constructor
LAMathSwaptionPremLMMDiscModel_ShiftedDD::LAMathSwaptionPremLMMDiscModel_ShiftedDD()
{
}

// constructor
LAMathSwaptionPremLMMDiscModel_ShiftedDD::LAMathSwaptionPremLMMDiscModel_ShiftedDD(LARatesPathElementCurve& curve0_F_,
																	LARatesPathElementCurve& curve0_D_,
																	LAMathCorrelationLMM& corr_LMM_,
																	LAMathVolatilityLMMDiscModel& vol_LMM_,
																	const DoubleVector& T_fix_L_,
																	const DoubleVector& T_pay_L_,
																	const DoubleVector& tau_L_,
																	const DoubleVector& T_pay_,
																	const DoubleVector& tau_,
																	double T_OptMat,
																	double T_Expire,
																	volType volType_,
																	double Q,
																	double constShift,
																	double forwardShift
																	)
:
LAMathSwaptionVolLMMDiscModel(curve0_F_,curve0_D_,corr_LMM_,vol_LMM_,T_fix_L_,T_pay_L_,tau_L_,T_pay_,tau_,T_OptMat,T_Expire,volType_),
mQ(Q),mConstShift(constShift),mForwardShift(forwardShift)
{
	getShiftedDDWeight();
}

/*!
	@brief function to get Variance of Sihfted SR 
*/
double LAMathSwaptionPremLMMDiscModel_ShiftedDD::getVar()
{
	double prem = getSwaptionPrem();
	
	double stdDev;
	if( mVolType == Black )
	{
		stdDev = local::BlackImplVolDD( prem / mAnnuity, mS0 + mForwardShift, mS0 + mForwardShift, 1, 1. );
	}
	else if( mVolType == Normal )
	{
		stdDev = local::BlackImplVolDD( prem / mAnnuity, mS0, mS0, 1, 0. );
	}
	else
	{
		LAString msg = "Only Black and Normal are supported in volType! : LAMathSwaptionPremLMMDiscModel_ShiftedDD::getVar()";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	return stdDev * stdDev;
}

/*!
	@brief function to get Variance of Sihfted SR 
*/
double LAMathSwaptionPremLMMDiscModel_ShiftedDD::getVar_ShiftedDD()
{
	double log2_Q = log(mQ) / log(2.);
	double inv_log2_Q_SQ = (1. - log2_Q) * (1. - log2_Q);

	double ret = 0.0;
    for( size_t i = mItr_OptMat_L; i < mItr_Expire_L + 1; ++i )
	{
        ret += mWeightCache_ShiftedDD[i] * mWeightCache_ShiftedDD[i] * getIntegrateCov(i, i, 0.0, mT_OptMat) / inv_log2_Q_SQ;

		for( size_t j = mItr_OptMat_L; j < i; ++j )
		{
            ret += 2.0 * mWeightCache_ShiftedDD[i] * mWeightCache_ShiftedDD[j] * getIntegrateCov(i, j, 0.0, mT_OptMat) / inv_log2_Q_SQ;
		}
	}

	return ret;
}

// Get premium of SR(Maturity, Expire)
double LAMathSwaptionPremLMMDiscModel_ShiftedDD::getSwaptionPrem()
{
	double log2_Q = log(mQ) / log(2.);
	double S0_shift = (mS0 + mConstShift) * (1. - log2_Q);
	double var = getVar_ShiftedDD();

	if ( S0_shift >0. )
	{
		return mAnnuity * local::BlackFormulaDD( S0_shift, LAMath::sqrt(var), S0_shift, 1, 1. );
	}
	else
	{
		LAString msg = "Shifted swap rate must be positive! : LAMathSwaptionPremLMMDiscModel_ShiftedDD::getSwaptionPrem";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
}

/*!
	@brief fuction to get ShiftedDD Weight

*/
void LAMathSwaptionPremLMMDiscModel_ShiftedDD::getShiftedDDWeight()
{
    mWeightCache_ShiftedDD.resize(mL_D.size());
	double log2_Q = log(mQ) / log(2.);

	DoubleMatrix dw_dL(mL_D.size(), DoubleVector(mL_D.size(), 0.));
	double sum_weight = 0;
	for( size_t h = mItr_Expire_L; h >= mItr_OptMat_L; --h )
	{
		if (h % mL2SNum == 0)
		{
			sum_weight += mTau[h / mL2SNum] * mCurve0_D->getP( mT_pay[h / mL2SNum] ) / mAnnuity;
		}
		for( size_t i = mItr_OptMat_L; i < mItr_Expire_L + 1; ++i )
		{
			double tmp = sum_weight;
			
			if( i >= h )
			{
				tmp -= 1.;
			}

			dw_dL[i][h] = mTau_L[h] * mWeight[i] / (1. + mTau_L[h] * mL_D[h]) * tmp;
		}

		if( h == mItr_OptMat_L ) break;
	}

	DoubleArray sum_L_dw_dL(mL_D.size());
	for( size_t h = mItr_OptMat_L; h < mItr_Expire_L + 1; ++h )
	{
		for( size_t i = mItr_OptMat_L; i < mItr_Expire_L + 1; ++i )
		{
			sum_L_dw_dL[h] += mL_F[i] * dw_dL[i][h];
		}
	}

    for( size_t h = mItr_OptMat_L; h < mItr_Expire_L + 1; ++h )
	{
		mWeightCache_ShiftedDD[h] = (mL_F[h] + mConstShift) / (mS0 + mConstShift) * (mWeight[h] + sum_L_dw_dL[h]);
	}
}
