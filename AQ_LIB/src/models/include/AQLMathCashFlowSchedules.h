#ifndef AQLMathCashFlowSchedules_h
#define AQLMathCashFlowSchedules_h

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "AQLString.h"
#include "AQLDate.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include <vector>

using namespace std;

struct CashFlowTiming
{
    AQLDate fixing;
    AQLDate start;
    AQLDate end;
    AQLDate payment;
    AQLDate indexSettlement;
    double accrual;
};

class AQLMathScheduleUtility
{
public:
//================ Single Flow ===================================
static CashFlowTiming CashFlowSchedule(AQLDate valDate, AQLString mtyTerm, AQLStringMatrix legScheduler, AQLStringMatrix indexScheduler);

//================ Multiple Flows ===================================
static vector<CashFlowTiming> LegSchedule(AQLDate valDate, AQLString mtyTerm, AQLStringMatrix legScheduler, AQLStringMatrix indexScheduler);

static DateVector BaseDates(AQLString frequency, AQLString calendar, AQLString slidingRule,
                     AQLDate startDate, AQLDate endDate, int* day);

static DateVector StartDates(DateVector baseDates);

static DateVector EndDates(DateVector baseDates);

static DateVector PayDates(DateVector endDates, AQLString term, AQLPriceDataSlidingRule slidingRule, AQLPriceDataCalendar calendar);

static DateVector FixingDates(DateVector startDates, DateVector payDates, bool isAdvance, AQLString fixingLag, AQLString calendar);

static DateVector IndexSettlementDates(DateVector fixingDates, AQLString settlLag, AQLString calendar);

static DoubleVector Accruals(AQLPriceDataDayCount daycount, DateVector startDates, DateVector endDates);

static AQLPriceDataSlidingRule ModelSlidingRule();

static AQLPriceDataCalendar ModelCalendar();
};

#endif