#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathCashFlowSchedules.h"
#include "AQLFunctionUtilities.h"
#include "AQLDateSchedule.h"

//================ Single Flow ===================================
CashFlowTiming AQLMathScheduleUtility::CashFlowSchedule(AQLDate valDate, AQLString mtyTerm, AQLStringMatrix legScheduler, AQLStringMatrix indexScheduler)
{
    ////// Input properties ////
    AQLString spotLag = AQLFunctionUtilities::findElement(legScheduler, "SpotLag");
    AQLString payCalendar_ = AQLFunctionUtilities::findElement(legScheduler, "PaymentCalendar");
    AQLString paySlidingRule_ = AQLFunctionUtilities::findElement(legScheduler, "PaymentSlidingRule");
    //AQLString payDelay = AQLFunctionUtilities::findElement(legScheduler, "PaymentDelay");
    AQLString fixingTiming = AQLFunctionUtilities::findElement(legScheduler, "FixingTiming");
    AQLString fixingLag = AQLFunctionUtilities::findElement(legScheduler, "FixingLag");
    AQLString fixingCalendar = AQLFunctionUtilities::findElement(legScheduler, "FixingCalendar");
    AQLString daycountConvention = AQLFunctionUtilities::findElement(legScheduler, "Daycount");
    upper(fixingTiming);
    bool isAdvance = (fixingTiming == "ADVANCE");
    bool overrideToArrear = true;
    if (overrideToArrear)
        isAdvance = false;
    AQLString indexFixingLag = AQLFunctionUtilities::findElement(indexScheduler, "FixingLag");
    AQLString indexFixingCalendar = AQLFunctionUtilities::findElement(indexScheduler, "FixingCalendar");

    // Conversions
    AQLPriceDataSlidingRule paySlidingRule;
    paySlidingRule.convertFromString(paySlidingRule_);
    AQLPriceDataCalendar payCalendar;
    upper(payCalendar_);
    payCalendar.convertFromString(payCalendar_);

    // No change
    AQLPriceDataSlidingRule noChangeSlidingRule;
    noChangeSlidingRule.convertFromString("no_change");
    AQLPriceDataCalendar noChangeCalendar;
    noChangeCalendar.convertFromString("");

    //// Calculation ////
    //AQLDate startDate = etrading::CalendarAdvance(valDate, spotLag, paySlidingRule, payCalendar);
    AQLDate swapStartDate = etrading::CalendarAdvance(valDate, spotLag, paySlidingRule, payCalendar);
    AQLDate endDate = etrading::CalendarAdvance(swapStartDate, mtyTerm, noChangeSlidingRule, noChangeCalendar);
    AQLDate payDate = endDate;
    AQLDate startDate = endDate;
    //AQLDate payDate = etrading::CalendarAdvance(endDate, payDelay, paySlidingRule, payCalendar);
    // Fixing Date
    AQLDate refDate = (isAdvance ? startDate : payDate);
    AQLPriceDataSlidingRule fixingSr;
    fixingSr.convertFromString("Preceding");
    AQLPriceDataCalendar fixingCdr;
    fixingCdr.convertFromString(fixingCalendar);
    AQLDate fixingDate = etrading::CalendarAdvance(refDate, "-" + fixingLag, fixingSr, fixingCdr);
    // Index settlement
    AQLPriceDataSlidingRule idxSr;
    idxSr.convertFromString("Following");
    AQLPriceDataCalendar idxFixingCdr;
    idxFixingCdr.convertFromString(indexFixingCalendar);
    AQLDate idxSettlDate = etrading::CalendarAdvance(fixingDate, indexFixingLag, idxSr, idxFixingCdr);
    // Accrual
    AQLPriceDataDayCount daycount = etrading::Daycount(daycountConvention);
    double accrual = etrading::YearFraction(daycount, startDate, endDate);

    return CashFlowTiming { fixingDate, startDate, endDate, payDate, idxSettlDate, accrual };
}

//================ Multiple Flows ===================================
vector<CashFlowTiming> AQLMathScheduleUtility::LegSchedule(AQLDate valDate, AQLString mtyTerm, AQLStringMatrix legScheduler, AQLStringMatrix indexScheduler)
{
    // Find input properties
    AQLString spotLag = AQLFunctionUtilities::findElement(legScheduler, "SpotLag");
    AQLString frequency = AQLFunctionUtilities::findElement(legScheduler, "Frequency");
    AQLString accCalendar = AQLFunctionUtilities::findElement(legScheduler, "AccrualCalendar");
    AQLString accSlidingRule = AQLFunctionUtilities::findElement(legScheduler, "AccrualSlidingRule");
    AQLString payDelay = AQLFunctionUtilities::findElement(legScheduler, "PaymentDelay");
    AQLString payCalendar_ = AQLFunctionUtilities::findElement(legScheduler, "PaymentCalendar");
    AQLString paySlidingRule_ = AQLFunctionUtilities::findElement(legScheduler, "PaymentSlidingRule");
    AQLString fixingLag = AQLFunctionUtilities::findElement(legScheduler, "FixingLag");
    AQLString fixingCalendar = AQLFunctionUtilities::findElement(legScheduler, "FixingCalendar");
    AQLString fixingTiming = AQLFunctionUtilities::findElement(legScheduler, "FixingTiming");
    AQLString daycountConvention = AQLFunctionUtilities::findElement(legScheduler, "Daycount");
    upper(fixingTiming);
    bool isAdvance = (fixingTiming == "ADVANCE");
    AQLString indexFixingLag = AQLFunctionUtilities::findElement(indexScheduler, "FixingLag");
    AQLString indexFixingCalendar = AQLFunctionUtilities::findElement(indexScheduler, "FixingCalendar");

    // Conversions
    AQLPriceDataSlidingRule paySlidingRule;
    paySlidingRule.convertFromString(paySlidingRule_);
    AQLPriceDataCalendar payCalendar;
    upper(payCalendar_);
    payCalendar.convertFromString(payCalendar_);
    AQLPriceDataSlidingRule noChangeSlidingRule;
    noChangeSlidingRule.convertFromString("no_change");
    AQLPriceDataCalendar noChangeCalendar;
    noChangeCalendar.convertFromString("");

    // Preliminary
    AQLDate startDate = etrading::CalendarAdvance(valDate, spotLag, paySlidingRule, payCalendar);
    AQLDate endDate = etrading::CalendarAdvance(startDate, mtyTerm, noChangeSlidingRule, noChangeCalendar);
    int day = endDate.dayOfMonth();

    // Calculate schedule
    DateVector baseDates = BaseDates(frequency, accCalendar, accSlidingRule, startDate, endDate, &day);
    DateVector startDates = StartDates(baseDates);
    DateVector endDates = EndDates(baseDates);
    DateVector payDates = PayDates(endDates, payDelay, paySlidingRule, payCalendar);
    DateVector fixingDates = FixingDates(startDates, payDates, isAdvance, fixingLag, fixingCalendar);
    DateVector idxSettlDates = IndexSettlementDates(fixingDates, indexFixingLag, indexFixingCalendar);
    AQLPriceDataDayCount daycount = etrading::Daycount(daycountConvention);
    vector<double> accruals = Accruals(daycount, startDates, endDates);

    size_t n = payDates.size();
    vector<CashFlowTiming> schedule(n);
    for (size_t i = 0; i < n; i++)
        schedule[i] = CashFlowTiming{ fixingDates[i], startDates[i], endDates[i], payDates[i], idxSettlDates[i], accruals[i] };

    return schedule;
}

DateVector AQLMathScheduleUtility::BaseDates(AQLString frequency, AQLString calendar, AQLString slidingRule,
                     AQLDate startDate, AQLDate endDate, int* day)
{
    return etrading::AQLDateSchedule::generateSchedule(startDate, endDate, frequency, slidingRule, calendar, 0, 0, day, true, 0);
}

DateVector AQLMathScheduleUtility::StartDates(DateVector baseDates)
{
    size_t n = baseDates.size() - 1;
    DateVector startDates(n);
    for (size_t i = 0; i < n; i++)
        startDates[i] = baseDates[i];

    return startDates;
}

DateVector AQLMathScheduleUtility::EndDates(DateVector baseDates)
{
    size_t n = baseDates.size() - 1;
    DateVector endDates(n);
    for (size_t i = 0; i < n; i++)
        endDates[i] = baseDates[i + 1];

    return endDates;
}

DateVector AQLMathScheduleUtility::PayDates(DateVector endDates, AQLString term, AQLPriceDataSlidingRule slidingRule, AQLPriceDataCalendar calendar)
{
    size_t n = endDates.size();
    DateVector payDates(n);
    for (size_t i = 0; i < n; i++)
        payDates[i] = etrading::CalendarAdvance(endDates[i], term, slidingRule, calendar);

    return payDates;
}

DateVector AQLMathScheduleUtility::FixingDates(DateVector startDates, DateVector payDates, bool isAdvance, AQLString fixingLag, AQLString calendar)
{
    DateVector refDates = (isAdvance ? startDates : payDates);
    size_t n = refDates.size();
    DateVector fixingDates(n);
    AQLPriceDataSlidingRule slidingRule;
    slidingRule.convertFromString("Preceding");
    AQLPriceDataCalendar cdr;
    cdr.convertFromString(calendar);
    for (size_t i = 0; i < n; i++)
        fixingDates[i] = etrading::CalendarAdvance(refDates[i], "-" + fixingLag, slidingRule, cdr);

    return fixingDates;
}

DateVector AQLMathScheduleUtility::IndexSettlementDates(DateVector fixingDates, AQLString settlLag, AQLString calendar)
{
    size_t n = fixingDates.size();
    DateVector idxSettlDates(n);
    AQLPriceDataSlidingRule slidingRule;
    slidingRule.convertFromString("Following");
    AQLPriceDataCalendar cdr;
    cdr.convertFromString(calendar);
    for (size_t i = 0; i < n; i++)
        idxSettlDates[i] = etrading::CalendarAdvance(fixingDates[i], settlLag, slidingRule, cdr);

    return idxSettlDates;
}

DoubleVector AQLMathScheduleUtility::Accruals(AQLPriceDataDayCount daycount, DateVector startDates, DateVector endDates)
{
    size_t n = startDates.size();
    DoubleVector acc(n);
    for (size_t i = 0; i < n; i++)
        acc[i] = etrading::YearFraction(daycount, startDates[i], endDates[i]);

    return acc;
}

AQLPriceDataSlidingRule AQLMathScheduleUtility::ModelSlidingRule()
{
    AQLPriceDataSlidingRule slidingRule;
    slidingRule.convertFromString("MOD_FOLLOWING");
    return slidingRule;
}

AQLPriceDataCalendar AQLMathScheduleUtility::ModelCalendar()
{
    AQLPriceDataCalendar calendar;
    calendar.convertFromString("TKB:LNB");
    return calendar;
}
