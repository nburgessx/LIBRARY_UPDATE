/*! @file
    @brief Data class declaration to represent the data (type rate convention).
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLBasic.h"
#include "AQLPriceDataConvention.h"
#include "AQLDate.h"
#include "AQLCoreAppError.h"
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
AQLPriceDataConvention::AQLPriceDataConvention(void) : AQLPriceDataType(DATA_CONVENTION)
{
    mRateConvention = CONT;
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLPriceDataConvention::AQLPriceDataConvention(const AQLPriceDataConvention& attr) 
    : AQLPriceDataType(attr)
{
  	mDayCount = attr.mDayCount;
    mRateConvention = attr.mRateConvention;
}

/*!
    @brief constructor

    @param[in] dc day count convention
    @param[in] rc rate convention
*/
AQLPriceDataConvention::AQLPriceDataConvention(const DayCount dc, 
                                   const RateConvention rc)
    : AQLPriceDataType(DATA_CONVENTION)
{
    mDayCount.setDayCount(dc);
    mRateConvention = rc;
    setNull(false);
}

/*!
    @brief destructor
*/
AQLPriceDataConvention::~AQLPriceDataConvention(void)
{
}

/*!
    @brief deep copy of the data object

    @return the copied object
*/
AQLPriceDataType*    
AQLPriceDataConvention::clone() const
{
    try {
        AQLPriceDataConvention*   pAttr = new AQLPriceDataConvention(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief interest rate calculation method in the string representation
    
    @return interest rate calculation method

    @sa enum RateConvention, enum DayCount
*/
AQLString      
AQLPriceDataConvention::convertToString(void) const
{
    if (isNull()) return NULL_STR;

    AQLString ret(gRateConvention[mRateConvention]);
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
AQLPriceDataConvention::getTerm(const AQLDate& fromDate, const AQLDate& toDate) const
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
AQLPriceDataConvention::getTerm(const AQLDate& fromDate, const double& termDays) const
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
AQLPriceDataConvention::getDayTerm(const AQLDate& fromDate, const double& termY) const
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
AQLPriceDataConvention::getDF(const double& rate,
                        const AQLDate& fromDate,
                        const AQLDate& toDate) const
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
AQLPriceDataConvention::getDF(const double& rate,
                        const AQLDate& fromDate,
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
AQLPriceDataConvention::getCompound(const double& rate,
                        const AQLDate& fromDate,
                        const AQLDate& toDate) const
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
AQLPriceDataConvention::getCompound(const double& rate,
                        const AQLDate& fromDate,
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
AQLPriceDataConvention::getRate(const double& df,
                        const AQLDate& fromDate,
                        const AQLDate& toDate) const
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
AQLPriceDataConvention::getRate(const double& df,
                        const AQLDate& fromDate,
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
AQLPriceDataConvention::convertTo(const double& rate, 
                        const AQLDate& fromDate,
                        const AQLDate& toDate,
                        const AQLPriceDataConvention& toConv) const
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
AQLPriceDataConvention::convertFromString(const AQLString& str)
{
    AQLString data;
    mDayCount = ACT_ACT;
    mRateConvention = CONT;
    update();   // update
    setNull();  // NULL

    bool ret = strToData(str, data);
    if (! ret && data.size() != 0)
    {
        vector<AQLString> items = data.toToken('(');
        if (items.size() != 2) 
        {
            AQLString msg("Invalid String for convertFromString : ");
            msg += str;
            throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
            AQLString msg("Invalid String for convertFromString : ");
            msg += str;
            throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
AQLPriceDataConvention::setConvention(const RateConvention rc)
{
    mRateConvention = rc;
    setNull(false);
    // update
    update();
}

/*!
    @brief the interest calculation rules

    @param[in] rc the interest calculation rules to be set with AQLString type
*/
void                  
AQLPriceDataConvention::setConvention(const AQLString& rc)
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
        AQLString msg("Invalid String for setConvention : ");
        msg += rc;
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
AQLPriceDataConvention::setDayCount(const DayCount dc)
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
AQLPriceDataConvention::setDayCount(const AQLString& dc)
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
AQLPriceDataType& 
AQLPriceDataConvention::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_CONVENTION) 
    {
        AQLString err = "Assignement error for AQLPriceDataConvention : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    mDayCount = dynamic_cast<const AQLPriceDataConvention&>(a).mDayCount;
    mRateConvention = dynamic_cast<const AQLPriceDataConvention&>(a).mRateConvention;
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
AQLPriceDataConvention::compare(const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_CONVENTION) 
    {
        AQLString err = "Compare error for AQLPriceDataConvention : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const AQLPriceDataConvention& at = dynamic_cast<const AQLPriceDataConvention&>(a);
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
AQLPriceDataConvention::rateToRet(const double& rate, 
                            const double& term,
                            const AQLPriceDataConvention& conv)
{
    // 1/DF
	double ret = 0;
    switch (conv.mRateConvention)
    {
    case SIMPL:
        ret = (1.0 + rate * term);
        break;
    case CONT:
        ret = AQLMath::exp(rate * term);
        break;
    case MNT:
        ret = AQLMath::pow((1 + rate / 12.0), term * 12.0);
        break;
    case SA:
        ret = AQLMath::pow((1 + rate / 2.0), term * 2.0);
        break;
    case ANN:
        ret = AQLMath::pow((1 + rate), term);
        break;
    case QTR:
        ret = AQLMath::pow((1+rate / 4.0), term * 4.0);
        break;
	default:
        throw AQLCoreInvalidData("#Error: Unknown rate-convention / compounding frequency.", __FILE__, __LINE__);
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
AQLPriceDataConvention::retToRate(const double& ret, 
                            const double& term,
                            const AQLPriceDataConvention& conv)
{
    double rate= 0;
    switch (conv.mRateConvention)
    {
    case SIMPL:
        rate = (ret - 1.0) / term;
        break;
    case CONT:
        rate = AQLMath::log(ret) / term;
        break;
    case MNT:
        rate = (AQLMath::pow(ret, 1.0 / (term * 12.0)) - 1.0) * 12.0;
        break;
    case SA:
        rate = (AQLMath::pow(ret, 1.0 / (term * 2.0)) - 1.0) * 2.0;
        break;
    case ANN:
        rate = AQLMath::pow(ret, 1.0 / term) - 1.0;
        break;
    case QTR:
        rate = (AQLMath::pow(ret, 1.0 / (term * 4.0)) - 1.0) * 4.0;
        break;
    }
    return rate;
}
