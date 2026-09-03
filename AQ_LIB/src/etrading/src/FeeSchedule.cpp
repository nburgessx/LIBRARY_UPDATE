#include <boost/date_time.hpp>

#include "FeeSchedule.h"
#include "FeeCashflow.h"
#include "DateUtilities.h"

namespace etrading
{
	
	FeeSchedule::FeeSchedule(const std::string& instanceName) : Schedule(instanceName)
	{
  	    scheduleType_ = FEE_SCHEDULE_TYPE;
    }

    FeeSchedule::FeeSchedule(const std::string& instanceName, const std::vector<LabelValueBlock>& cashflowLVBs) : Schedule(instanceName) 
	{
		//inputParameters_ = cashflowLVBs;

	    scheduleType_ = FEE_SCHEDULE_TYPE;
        createBespokeCashflows(cashflowLVBs);
	}

    SchedulePtr FeeSchedule::clone()
    {
        SchedulePtr sch = SchedulePtr(new FeeSchedule(*this));
        return sch;
    }

	FeeSchedule::FeeSchedule(const FeeSchedule& rhs) : Schedule(rhs)
	{}

    void FeeSchedule::createBespokeCashflows(const std::vector<LabelValueBlock>& cashflowLVBs) 
	{

        for (size_t i=0; i < cashflowLVBs.size(); ++i)
		{
			LabelValueBlock lvb = cashflowLVBs[i];
            
			const std::string inputLVB = "cashflowLVB";
            
			LADate paymentDate = lvb.getOptionalValueAsDate( CASHFLOW_KEY::PAYMENT_DATE);
			double amount = lvb.getOptionalValueAsDouble( FEE_KEY::AMOUNT);
			PayReceiveEnum payReceive = toPayReceiveEnum(lvb.getOptionalValueAsLAString( IRS_KEY::PAY_RECEIVE).getCString());

            //Ignore the blank line
            if (paymentDate != LADate())
            {
			    CashflowPtr cf = CashflowPtr(new FeeCashflow(payReceive, paymentDate, amount));
			    cashflows_.push_back(cf);
			    paymentDates_.push_back(paymentDate);
            }
            
		}
	}

    const std::string FeeSchedule::getCashflowSchemaName() const
    {
        return toString(FEE_SCHEDULE);
    }

    const DataSchema FeeSchedule::generateDataSchema(const std::string& schemaName) const
    {
	    const DataSchema  dynamicSchema; // Dummy schema
        return dynamicSchema;
    }

    const DataSchema FeeSchedule::generateCashflowSchema(const std::string& schemaName) const
    {
        std::string schName = (schemaName.size()!=0) ? schemaName : getDataSchemaName();
	    const DataSchema  dynamicSchema(    schName,3,
                                            boost::assign::list_of(STRING_VALUE)(STRING_VALUE)(STRING_VALUE),
		                                    boost::assign::list_of(CASHFLOW_KEY::PAYMENT_DATE)(FEE_KEY::AMOUNT)(IRS_KEY::PAY_RECEIVE));
        return dynamicSchema;
    }

    std::map<std::string, std::vector<std::string>> FeeSchedule::getCashflowDataMap() const
    {
        std::map<std::string, std::vector<std::string>> dataMap;
		
        std::vector<std::string> paymentDates;
        std::vector<std::string> paymentAmounts;
        std::vector<std::string> payReceiveIndicators;
        for (size_t i=0; i < cashflows_.size(); ++i)
        {
            auto feecf = cashflows_[i];	
            paymentDates.emplace_back(feecf->getPaymentDate().stringWithFormat().getCString());
            paymentAmounts.emplace_back(boost::lexical_cast<std::string>(feecf->getAmount()));
            payReceiveIndicators.emplace_back(  toString(feecf->getPayReceive()) );
        }

        dataMap[CASHFLOW_KEY::PAYMENT_DATE] = paymentDates;
        dataMap[FEE_KEY::AMOUNT] = paymentAmounts;
        dataMap[IRS_KEY::PAY_RECEIVE] = payReceiveIndicators;

        return dataMap;
    }

	std::unordered_set<CashflowHeaderEnum,EnumClassHash> FeeSchedule::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
		{
			PAYMENT_DATE_HEADER
			,AMOUNT_HEADER
			,PAY_RECEIVE_HEADER
		};

		return expectedList;
	}


}
