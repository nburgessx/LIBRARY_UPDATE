/*! @file
    @brief Source code for class to represent index.

			Following dataValues are registered automatically to data master<BR>
			1.CALIBRATION_DATA_NAME(AQLDataString)<BR>			
			2.IR_MODEL_DATA_INDEXTYPE(AQLDataString)<BR>			
			3.IR_MODEL_DATA_ACCESSORY(AQLDataString)<BR>			
			4.IR_MODEL_DATA_CURRENCY(AQLDataString)<BR>			
			5.IR_MODEL_DATA_DAYCOUNT(AQLPriceDataDayCount)<BR>
			6.IR_MODEL_DATA_FREQUENCY(AQLDataString)<BR>			
			7.IR_MODEL_DATA_CALENDAR(AQLPriceDataCalendar)<BR>
			8.IR_MODEL_DATA_SLIDINGRULE(AQLPriceDataSlidingRule)<BR>
			9.IR_MODEL_DATA_PATHENTITY(AQLDataReference)<BR>
			10.IR_MODEL_DATA_FXRATE(AQLDataReference)<BR>
			11.IR_MODEL_DATA_CACHESIZE(AQLDataInt)<BR>

*/
//  2006, AlgoQuantHub..
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathIndexEntityInterpolation.h"
#include "AQLMathDateCalculations.h"
#include "AQLDataVector.h"
#include "AQLPriceDataDayCount.h"
#include "AQLMathPathEntity.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLMathFXEntity.h"
#include "AQLLinearInterpolation.h"
#include "AQLDataMultiReference.h"
#include "AQLObjectHolder.h"
#include "AQLMathYieldCurve.h"
#include "AQLAlgorithm.h"
#include <cmath>

//

//#include <ql/math/interpolations/loginterpolation.hpp>
//#include <ql/math/interpolations/linearinterpolation.hpp>

#define SIMPLE		"SIMPLE"
#define ANNUAL		"ANNUAL"
#define SEMIANNUAL	"SEMI-ANNUAL"
#define QUARTERLY	"QUARTERLY"
#define MONTHLY		"MONTHLY"
#define CONTINUOUS	"CONTINUOUS"
#define SPOTLAG		0

const double INFINITESIMAL = 1E-7;
/*!
    @brief default constructor

	@param[in] dataInstance pointer of AQLDataInstance object

*/
AQLMathIndexEntityInterpolation::AQLMathIndexEntityInterpolation(AQLDataInstance* dataInstance)
:
AQLMathIndexEntity(dataInstance)
{
}

/*!
    @brief copy constructor

	@param[in] index AQLMathIndexEntityInterpolation object
*/
AQLMathIndexEntityInterpolation::AQLMathIndexEntityInterpolation(const AQLMathIndexEntityInterpolation& rhs) : 
AQLMathIndexEntity(rhs),
mT_Canonic(rhs.mT_Canonic)
{
}

/*!
    @brief destructor
*/
AQLMathIndexEntityInterpolation::~AQLMathIndexEntityInterpolation()
{
}

/*!
    @brief calculate index
*/
void
AQLMathIndexEntityInterpolation::calcInterCache()
{
	if (mSDEType == IR)
	{
		if (mSpreadMat.empty())
		{
			for (unsigned int i = 0; i < mT_Canonic.size(); i++)
			{
				const AQLRatesPathElementCurve& curve = 
					dynamic_cast<const AQLRatesPathElementCurve&>(mpPath->getPath(mSDEPos, mT_Canonic[i]));
				// calc rate
				double ret = 0.0;
				for (unsigned int j = 1; j < mIndex_TenorGrid[i].size(); j++)
					ret += mIndex_TermGrid[i][j - 1] * curve.getP(mIndex_TenorGrid[i][j]);

				if (ret == 0.0)
				{
					mInterCache[i] = 0.0;
				}
				else
				{
					mInterCache[i] = (curve.getP(mIndex_TenorGrid[i][0]) - curve.getP(mIndex_TenorGrid[i].back())) / ret;
				}
			}
		}
		else //reflect basis spread
		{
			for (unsigned int i = 0; i < mT_Canonic.size(); i++)
			{
				const AQLRatesPathElementCurve& curve = 
					dynamic_cast<const AQLRatesPathElementCurve&>(mpPath->getPath(mSDEPos, mT_Canonic[i]));
			
				//calc P vector
				DoubleVector P_vec;
				for (unsigned int j = 0; j < mIndex_TenorGrid[i].size(); ++j)
					P_vec.push_back(curve.getP(mIndex_TenorGrid[i][j]));
				//calc index reflect basis spread
				double ret = 0.0;
				for (unsigned int j = 1; j < mIndex_TenorGrid[i].size(); j++)
					ret += mIndex_TermGrid[i][j - 1] * (P_vec[j] * mSpreadMat_DF[i][j]);

				if (ret == 0.0)
				{
					mInterCache[i] = 0.0;
				}
				else
				{
					double floatPV = 0.0;
					for (unsigned int k = 1;  k < mIndex_TenorGrid[i].size(); k++)
					{
						const double df1 = P_vec[k-1] * mSpreadMat[i][k - 1];
						const double df2 = P_vec[k] * mSpreadMat[i][k];
						const double rate = (df1 - df2) / df2 / mIndex_TermGrid[i][k - 1];
						const double d_df = P_vec[k] * mSpreadMat_DF[i][k];
						floatPV += rate * d_df * mIndex_TermGrid[i][k - 1];
					}
					mInterCache[i] = floatPV / ret;
				}
			}
		}
	}
	else if(mSDEType == FX)
	{
		for (unsigned int i = 0; i < mT_Canonic.size(); i++)
        {
			mInterCache[i] = mpFX->getForwardRate(mFromCurrency, mToCurrency, mT_Canonic[i], mT_Canonic[i], mpPath->getDayCount().getDayCount());
        }
	}
}

/*!
    @brief calculate index
*/
void
AQLMathIndexEntityInterpolation::calcIndex()
{
    calcInterCache();
    
	AQLLinearInterpolation interplation;
	interplation.set(mT_Canonic, mInterCache);
	for (unsigned int i = 0; i < mTimeGrid.size(); i++)
	{
		mIndexArray[i] = interplation.value(mTimeGrid[i]);
	}
}

/*!
	@brief copy AQLMathIndexEntity
	@param[in] e copy source
	@return reference to this object
*/
AQLObject&
AQLMathIndexEntityInterpolation::copy(const AQLObject& e)
{
    AQLMathIndexEntity::copy(e);
    
    if (this != &e)
    {
        mT_Canonic = dynamic_cast<const AQLMathIndexEntityInterpolation&>(e).mT_Canonic; 
        mDate_Canonic = dynamic_cast<const AQLMathIndexEntityInterpolation&>(e).mDate_Canonic; 
    }

	return *this;
}

/*!

    @brief Make copy(clone) of this Index Object object.
    @return pointer of Index Object object.
*/
AQLObject*  AQLMathIndexEntityInterpolation::clone() const
{
    return new AQLMathIndexEntityInterpolation(*this);
}

/*!
*/
void AQLMathIndexEntityInterpolation::convertTermtoDate(DateVector& dateVec, DoubleArray& termVec)
{
    // Get AsOfDate --------
    const AQLDate& asof = mpPath->getAsOfDate().get();

    //
    //AQLPriceDataDayCount DC_(DayCount::ACT_365_ISDA);
	AQLPriceDataDayCount DC_(ACT_365_ISDA);

    //
    size_t n = termVec.size();
    dateVec.resize(n);
    
    //
    AQLDate date_  = asof;
    AQLDate date_tmp; 
    for(size_t i = 0; i < n; ++i)
    {   
        if(termVec[i] == 0.)
        {
            dateVec[i] = asof;
        }
        else
        {
            double tmp = DC_.getTerm(asof, date_);
            while( 1. / 360. < fabs(tmp - termVec[i]) )
            {
                date_.addDays(1);
                tmp = DC_.getTerm(asof, date_);
                if(DC_.getTerm(asof,date_) > 100.) {throw AQLCoreInvalidData("DC_.getTerm(asof,date_) > 100.", __FILE__, __LINE__); }
            }
            
            date_tmp = date_; date_tmp.addDays(1);
            if(fabs(DC_.getTerm(asof, date_tmp) - termVec[i]) < fabs(tmp - termVec[i]))
            {
                date_.addDays(1);
                tmp = DC_.getTerm(asof, date_);
            }
            dateVec[i] = date_;
        }
    }
}

/*!

	@brief set up this class for index calculation
	
	@note this method is called from setforMC method
*/
void
AQLMathIndexEntityInterpolation::setUp(void)
{
    //-------- SetUp AQLMathIndexEntity --------
    AQLMathIndexEntity::setUp();

    //-------- Canonical Time Grid --------
    mT_Canonic = mpPath->getSDETimeGrid().get();
	unsigned int pos;
	AQLAlgorithm::locate<DoubleArray, double>(mT_Canonic, mTimeGrid.back(), mT_Canonic.size(), pos);
	mT_Canonic.resize(pos + 1);
    mInterCache.resize(mT_Canonic.size());

	if (mSDEType != IR)
	{
		return;
	}
    //-------- Date handling --------
    // Get Frequency --------
    AQLString freq_(getFrequency()); freq_.toUpper();
    
    // Get Accessory --------
	AQLString accessory(getAccessory().get());
    bool CoTermFlg = accessory.findString("Co-Term") != -1;
	if(CoTermFlg)
	{
		accessory.remove(accessory.findString("Co-Term"),7);
		accessory.trimLeft();
	}
	
    // Get Year, Month, Date --------
	int Y_; int M_; int D_; int W_;
    AQLMathDateCalculations::termStrtoYMDW(accessory, Y_, M_, D_, W_);
    
    // Error handle
    if(D_ != 0) { throw AQLCoreInvalidData("D_ != 0", __FILE__, __LINE__);	}
	if(Y_ == 0 && M_ == 0) { throw AQLCoreInvalidData("Y_ == 0 && M_ == 0",__FILE__,__LINE__); }

    // Get Calender --------
    const AQLPriceDataCalendar& cal = getCalendar();

    // Get SlidingRule --------
    const AQLPriceDataSlidingRule& srule = getSlidingRule();

    // Get DayCount --------
    const AQLPriceDataDayCount& Daycount = getDayCount();
	const AQLPriceDataDayCount& DaycountOfPath = mpPath->getDayCount();

    // Get AsOfDate --------
    const AQLDate& asof = mpPath->getAsOfDate().get();
    
	size_t num_IndexTenorGridOf = 0;
	size_t addmonth = 0;
	
    if (freq_ == SIMPLE)
	{
		num_IndexTenorGridOf = 2;
		addmonth = Y_ * 12 + M_;
	}
	else if (freq_ == MONTHLY)
	{
		num_IndexTenorGridOf = Y_ * 12 + M_ + 1;
		addmonth = 1;
	}
	else if (freq_ == QUARTERLY)
	{
		if (M_ % 3 != 0)
		{
			throw AQLCoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);	
		}
		num_IndexTenorGridOf = Y_ * 4 + M_ / 3 + 1;
		addmonth = 3;

	}
	else if (freq_ == SEMIANNUAL)
	{
		if (M_ % 6 != 0)
		{
			throw AQLCoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);	
		}	
		num_IndexTenorGridOf = Y_ * 2 + M_ / 6 + 1;
		addmonth = 6;

	}
	else if (freq_ == ANNUAL)
	{
		if (M_ % 12 != 0)
		{
			throw AQLCoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);
		}
		num_IndexTenorGridOf = Y_ + M_ / 12 + 1;
		addmonth = 12;
	}
	else
	{
            AQLString msg = freq_ + "is not support"; 
            throw AQLCoreInvalidData(msg.getCString() , __FILE__, __LINE__);
	}

    // Convert Term to Date
    convertTermtoDate(mDate_Canonic, mT_Canonic);

	AQLDate date, date_nonadjust, olddate;
    
    size_t n = mT_Canonic.size();
    mIndex_TenorGrid.resize(n);
    mIndex_TermGrid.resize(n);
    if(CoTermFlg)
	{
		AQLDate terminal_nonadj = cal.getBusinessDay(asof,SPOTLAG);
		terminal_nonadj.addMonths(addmonth * (num_IndexTenorGridOf-1));
		AQLDate terminal = srule.getDate(terminal_nonadj,cal);
		for (size_t i = 0; i < mT_Canonic.size(); i++)
		{
			mIndex_TermGrid[i].clear();
			mIndex_TenorGrid[i].clear();
			date = cal.getBusinessDay(mDate_Canonic[i], SPOTLAG);
			date_nonadjust = date;
			double grid0 = DaycountOfPath.getTerm(asof, date);
			mIndex_TenorGrid[i].push_back(grid0);//spot date

			if(terminal <= date)
			{
				//temporary term;
				mIndex_TermGrid[i].push_back(1.0);
				mIndex_TenorGrid[i].push_back(grid0);
				continue;
			}
			bool chk = false;
			for(unsigned int j = 1; j < num_IndexTenorGridOf && chk == false; j++)
			{
				olddate = date;
				date_nonadjust.addMonths(addmonth);
				date = srule.getDate(date_nonadjust, cal);
				chk = (terminal <= date);
				if(chk)
				{
					date_nonadjust = terminal_nonadj;
					date = terminal;
					mIndex_TenorGrid[i].push_back(DaycountOfPath.getTerm(asof, date));
					mIndex_TermGrid[i].push_back(Daycount.getTerm(olddate, date, false));
					continue;
				}
				else
				{
					mIndex_TenorGrid[i].push_back(DaycountOfPath.getTerm(asof, date));
					mIndex_TermGrid[i].push_back(Daycount.getTerm(olddate, date, false));
				}
			}
		}
	}
	else
	{
		for (size_t i = 0; i < n; i++)
		{
			mIndex_TenorGrid[i].resize(num_IndexTenorGridOf);
			mIndex_TermGrid[i].resize(num_IndexTenorGridOf - 1);
			date = cal.getBusinessDay(mDate_Canonic[i], SPOTLAG);
			date_nonadjust = date;
			mIndex_TenorGrid[i][0] = DaycountOfPath.getTerm(asof, date);//spot date
			if (mT_Canonic[i] > mIndex_TenorGrid[i][0] + INFINITESIMAL)
			{
				throw AQLCoreInvalidData("mIndex_TenorGrid[i][0] is before mT_Canonic[i]", __FILE__, __LINE__);
			}
			for (unsigned int j = 1; j < num_IndexTenorGridOf; j++)
			{
				olddate = date;
				date_nonadjust.addMonths(addmonth);
				date = srule.getDate(date_nonadjust, cal);
				mIndex_TenorGrid[i][j] = DaycountOfPath.getTerm(asof, date);		
				mIndex_TermGrid[i][j - 1] = Daycount.getTerm(olddate, date, false);
			}
		}
	}
	
	//calc basis spread
	if (mpBasis->isDefined() && !mpBasis->isNull())
	{
		const AQLStringVector &sde_attrnames_ = mpPath->getSDEAttrNames().get();
		for (size_t i = 0; i < sde_attrnames_.size(); i++)
		{
			const AQLMathAttrSDE& attrsde = 
				dynamic_cast<const AQLMathAttrSDE&>(mpPath->getData(sde_attrnames_[i], ISNOTNULL).get());
			if (attrsde.getSDEPathType() == mSDEType && attrsde.getCurrency() == getCurrency().get())
			{
				const AQLMathYieldCurve &curve = 
					dynamic_cast<AQLMathYieldCurve &>(mpPath->getInitialValues().get(i).get());
				calcSpreadMat(curve, mIndex_TenorGrid);
				break;
			}		
		}
	}

}