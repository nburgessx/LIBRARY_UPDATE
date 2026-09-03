/*! @file
    @brief Data class declaration to represent the data (type rate convention).
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LABasic.h"
#include "LAPriceDataConvention.h"
#include "LADate.h"
#include "LACoreAppError.h"
#include <cmath>
#include <vector>

using namespace std;

// rate convention
#define RATE_C_SIZE 6
// string representing rate convention
static const char_t* gRateConvention[RATE_C_SIZE] =
{
    "CONT", "MNT", "QTR", "SA", "ANN", "SIMPL"
};

/*!
    @brief default constructor

    Continuous Compound(continuous compound) Actual / Actual is default
*/
LAPriceDataConvention::LAPriceDataConvention(void) : LAPriceDataType(DATA_CONVENTION)
{
    mRateConvention = CONT;
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LAPriceDataConvention::LAPriceDataConvention(const LAPriceDataConvention& attr) 
    : LAPriceDataType(attr)
{
  	mDayCount = attr.mDayCount;
    mRateConvention = attr.mRateConvention;
}

/*!
    @brief constructor

    @param[in] dc day count convention
    @param[in] rc rate convention
*/
LAPriceDataConvention::LAPriceDataConvention(const DayCount dc, 
                                   const RateConvention rc)
    : LAPriceDataType(DATA_CONVENTION)
{
    mDayCount.setDayCount(dc);
    mRateConvention = rc;
    setNull(false);
}

/*!
    @brief destructor
*/
LAPriceDataConvention::~LAPriceDataConvention(void)
{
}

/*!
    @brief deep copy of the data object

    @return the copied object
*/
LAPriceDataType*    
LAPriceDataConvention::clone() const
{
    try {
        LAPriceDataConvention*   pAttr = new LAPriceDataConvention(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief interest rate calculation method in the string representation
    
    @return interest rate calculation method

    @sa enum RateConvention, enum DayCount
*/
LAString      
LAPriceDataConvention::convertToString(void) const
{
    if (isNull()) return NULL_STR;

    LAString ret(gRateConvention[mRateConvention]);
    ret += "(";
	ret += mDayCount.convertToString();
    ret += ")";
    return ret;
}

/*!
    @brief get terms in years between fromDate and toDate
    
	@param[in] fromDate start date
    @param[in] toDate   end date

    @return terms in years
*/
double
LAPriceDataConvention::getTerm(const LADate& fromDate, const LADate& toDate) const
{
	return mDayCount.getTerm(fromDate, toDate);
}

/*!
    @brief get terms in years (number of days per period) from the start date
 
    @param[in] fromDate start date
    @param[in] termDays number of days from start date

    @return terms in years
*/
double
LAPriceDataConvention::getTerm(const LADate& fromDate, const double& termDays) const
{
	return mDayCount.getTerm(fromDate, termDays);
}

/*!
    @brief get terms in days (number of years per period) from the start date

    @param[in] fromDate start date
    @param[in] termY number of years per period from the start date

    @return terms in days
*/
double
LAPriceDataConvention::getDayTerm(const LADate& fromDate, const double& termY) const
{
	return mDayCount.getDayTerm(fromDate, termY);

}

/*!
    @brief return discount factor based on the period and interest rate (annual interest)
   
    @param[in] rate interest rate(per year)
    @param[in] fromDate start date
    @param[in] toDate end date

    @return DF(Discount Factor)
*/
double
LAPriceDataConvention::getDF(const double& rate,
                        const LADate& fromDate,
                        const LADate& toDate) const
{
    return 1.0 / rateToRet(rate, getTerm(fromDate, toDate), *this);
}

/*!
    @brief return discount factor based on the period and interest rate (annual interest)

    @param[in] rate interest rate(per year)
    @param[in] fromDate start date
    @param[in] term number of days from start date

    @return DF(Discount Factor)
*/
double
LAPriceDataConvention::getDF(const double& rate,
                        const LADate& fromDate,
                        const double& term) const
{
    return 1.0 / rateToRet(rate, getTerm(fromDate,term), *this);
}

/*!
    @brief return yield to maturity based on the period and interest rate (annual interest)

    @param[in] rate interest rate(per year)
    @param[in] fromDate start date
    @param[in] toDate end date

    @return yield to maturity
*/
double
LAPriceDataConvention::getCompound(const double& rate,
                        const LADate& fromDate,
                        const LADate& toDate) const
{
    return rateToRet(rate, getTerm(fromDate, toDate), *this) - 1.0;
}

/*!
    @brief return yield to maturity based on the period and interest rate (annual interest)
    
    @param[in] rate interest rate(per year)
    @param[in] fromDate start date
    @param[in] term number of days from start date

    @return yield to maturity
*/
double
LAPriceDataConvention::getCompound(const double& rate,
                        const LADate& fromDate,
                        const double& term) const
{
    return rateToRet(rate, getTerm(fromDate,term), *this) - 1.0;
}

/*!
    @brief return interest rate (annual interest) based on the period and discount factor
    
    @param[in] df DF(Discount Factor)
    @param[in] fromDate start date
    @param[in] toDate enda date

    @return interest rate (annual interest)
*/
double
LAPriceDataConvention::getRate(const double& df,
                        const LADate& fromDate,
                        const LADate& toDate) const
{
    return retToRate(1.0 / df, getTerm(fromDate, toDate), *this);
}

/*!
    @brief return interest rate (annual interest) based on the period and discount factor

    @param[in] df DF(Discount Factor)
    @param[in] fromDate start date
    @param[in] term number of days from start date

    @return interest rate (annual interest)
*/
double
LAPriceDataConvention::getRate(const double& df,
                        const LADate& fromDate,
                        const double& term) const
{
    return retToRate(1.0 / df, getTerm(fromDate,term), *this);
}

/*!
    @brief convert interest rate
    
	convert the interest rate based on the rate convention that you specify

    @param[in] rate rate (annual interest)
    @param[in] fromDate start date
    @param[in] toDate end date
    @param[in] toConv rate convention

    @return converted interest rate (annual interest)
*/
double              
LAPriceDataConvention::convertTo(const double& rate, 
                        const LADate& fromDate,
                        const LADate& toDate,
                        const LAPriceDataConvention& toConv) const
{
    if (*this == toConv) return rate;
    double ret = rateToRet(rate, getTerm(fromDate, toDate), *this);
    return retToRate(ret, toConv.getTerm(fromDate, toDate), toConv);
}

/*!
    @brief set the rate convention information from a string of rate convention that you specify

	select string with rate convention format, and selectable string refers to enum RateConvention and enum DayCount.

    @param[in] str string representing rate convention (e.g. "CONT(ACT_ACT)")

    @sa enum RateConvention, enum DayCount
*/
void          
LAPriceDataConvention::convertFromString(const LAString& str)
{
    LAString data;
    mDayCount = ACT_ACT;
    mRateConvention = CONT;
    update();   // update
    setNull();  // NULL

    bool ret = strToData(str, data);
    if (! ret && data.size() != 0)
    {
        vector<LAString> items = data.toToken('(');
        if (items.size() != 2) 
        {
            LAString msg("Invalid String for convertFromString : ");
            msg += str;
            throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        }
        items[1].charUpdate(items[1].size() - 1, '\0');
    
        // set RATE CONVENTION
        int i;
        for (i = 0; i < RATE_C_SIZE; ++i)
        {
            if (items[0] == gRateConvention[i])
            {
                mRateConvention = RateConvention(i);
                break;
            }
        }
        if (i == RATE_C_SIZE)
        {
            LAString msg("Invalid String for convertFromString : ");
            msg += str;
            throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        }
        // DAY COUNT BASIS set
        mDayCount.convertFromString(items[1]);

        setNull(false);
    }
}

/*!
    @brief set the interest calculation rules

    @param[in] rc the interest calculation rules to be set
*/
void                  
LAPriceDataConvention::setConvention(const RateConvention rc)
{
    mRateConvention = rc;
    setNull(false);
    // update
    update();
}

/*!
    @brief the interest calculation rules

    @param[in] rc the interest calculation rules to be set with LAString type
*/
void                  
LAPriceDataConvention::setConvention(const LAString& rc)
{
    int i;
    for (i = 0; i < RATE_C_SIZE; ++i)
    {
        if (rc == gRateConvention[i])
        {
            mRateConvention = RateConvention(i);
            break;
        }
    }
    if (i == RATE_C_SIZE)
    {
        LAString msg("Invalid String for setConvention : ");
        msg += rc;
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    setNull(false);
    // update
    update();
}

/*!
    @brief set day count convention

    @param[in] dc day count to be set
*/
void                  
LAPriceDataConvention::setDayCount(const DayCount dc)
{
    mDayCount.setDayCount(dc);
    setNull(false);
    // update
    update();
}

/*!
    @brief set day cont convention

    @param[in] dc day count to be set with MSString type
*/
void                  
LAPriceDataConvention::setDayCount(const LAString& dc)
{
    mDayCount.setDayCount(dc);
    setNull(false);
    // update
    update();
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents
*/
LAPriceDataType& 
LAPriceDataConvention::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_CONVENTION) 
    {
        LAString err = "Assignement error for LAPriceDataConvention : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    mDayCount = dynamic_cast<const LAPriceDataConvention&>(a).mDayCount;
    mRateConvention = dynamic_cast<const LAPriceDataConvention&>(a).mRateConvention;
    setNull(a.isNull());
    // update
    update();
    return *this;
}

/*
    @brief compare the contents against the other objecet
    
	comparison is performed by rate convention, and next by day count convention
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared
    
    @retval 0     both rate convention and day count are the same
    @retval not 0 otherwise
*/
int          
LAPriceDataConvention::compare(const LAPriceDataType& a) const
{
    if (a.getType() != DATA_CONVENTION) 
    {
        LAString err = "Compare error for LAPriceDataConvention : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const LAPriceDataConvention& at = dynamic_cast<const LAPriceDataConvention&>(a);
    int ret = (at.mRateConvention - mRateConvention);
    if (ret != 0) return ret;
    ret = at.getDayCount() - getDayCount();
    return ret;
}



//================== PRIVATE METHODS ===============================
/*! 
    @brief calculate the rate of return from the period and interest rate (annual interest) based on the rate convention

    @param[in] rate interest rate (annual interest)
    @param[in] term period (in years)
    @param[in] conv rate convention data

    @return rate of return
*/
double
LAPriceDataConvention::rateToRet(const double& rate, 
                            const double& term,
                            const LAPriceDataConvention& conv)
{
    // 1/DF
	double ret = 0;
    switch (conv.mRateConvention)
    {
    case SIMPL:
        ret = (1.0 + rate * term);
        break;
    case CONT:
        ret = LAMath::exp(rate * term);
        break;
    case MNT:
        ret = LAMath::pow((1 + rate / 12.0), term * 12.0);
        break;
    case SA:
        ret = LAMath::pow((1 + rate / 2.0), term * 2.0);
        break;
    case ANN:
        ret = LAMath::pow((1 + rate), term);
        break;
    case QTR:
        ret = LAMath::pow((1+rate / 4.0), term * 4.0);
        break;
	default:
        throw LACoreInvalidData("#Error: Unknown rate-convention / compounding frequency.", __FILE__, __LINE__);
    }
	
    return ret;
}

/*! 
    @brief calculate interest rate (annual interest) from the period and the rate of return based on the rate convention

    @param[in] ret rate of return
    @param[in] term period (in years)
    @param[in] conv rate convention data

    @return interest rate (annual interest)
*/
double
LAPriceDataConvention::retToRate(const double& ret, 
                            const double& term,
                            const LAPriceDataConvention& conv)
{
    double rate= 0;
    switch (conv.mRateConvention)
    {
    case SIMPL:
        rate = (ret - 1.0) / term;
        break;
    case CONT:
        rate = LAMath::log(ret) / term;
        break;
    case MNT:
        rate = (LAMath::pow(ret, 1.0 / (term * 12.0)) - 1.0) * 12.0;
        break;
    case SA:
        rate = (LAMath::pow(ret, 1.0 / (term * 2.0)) - 1.0) * 2.0;
        break;
    case ANN:
        rate = LAMath::pow(ret, 1.0 / term) - 1.0;
        break;
    case QTR:
        rate = (LAMath::pow(ret, 1.0 / (term * 4.0)) - 1.0) * 4.0;
        break;
    }
    return rate;
}
