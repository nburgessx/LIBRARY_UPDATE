using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace MLIB_API_CSHARP_TEST
{
    public class InstrumentRates 
    {

        private List<String[]> rates;

        private InstrumentRates(List<String[]> rates) 
        {
            this.rates = rates;
        }

        public VecVecString toMlib() 
        {
            VecVecString ret = new VecVecString();
            foreach (String[] entry in rates)
            {
                int size = entry.Length;
                VecString vecString = new VecString();
                for(int i=0; i<size; ++i)
                {
                    String temp = entry[i];
                    if (temp == "0")
                    {
                        vecString.Add("");
                    }
                    else
                    {
                        vecString.Add(entry[i]);
                    }                    
                }

                ret.Add(vecString);
            }

            return ret;
        }

        public class Builder 
        {
            private List<String[]> rates = new List<String[]>();

            public Builder addDateRate(String date, double rate) 
            {
                addRate(date, Convert.ToString(rate));

                return this;
            }

            public Builder addTermRate(String term, double rate) 
            {
                addRate(term, Convert.ToString(rate));

                return this;
            }

            public Builder addTermRateUse(String term, double rate, bool isUsed) 
            {
                addRate(term, Convert.ToString(rate), Convert.ToString(isUsed));

                return this;
            }

            public Builder addTermRateStartDateEndDateUse(String term, double rate, int startDate, int endDate, bool isUsed) 
            {

                addRate(term, Convert.ToString(rate), Convert.ToString(startDate), Convert.ToString(endDate), Convert.ToString(isUsed));

                return this;
            }

            public Builder addTermStartDateEndDateRateVolatility(String term, String startDate, String endDate, double rate, double volatility) 
            {
                addRate(term, startDate, endDate, Convert.ToString(rate), Convert.ToString(volatility));

                return this;
            }

            public InstrumentRates build() 
            {
                return new InstrumentRates(rates);
            }

            private void addRate(params String[] rate)
            {
                rates.Add(rate);
            }

            //private String fromDate(Date date) 
            //{
            //    if (date == null) {
            //        return "";
            //    }

            //    return Integer.toString(toExcelDate(date));
            //}
        }
    }

}
