#include "LAString.h"
#include "LACurvePricingObject.h"
#include "InitializeAQETrading.h"
#include "meCurveDisplay.h"
#include "tryMeCurveDisplay.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for meCurveDisplay
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		curveIndex			Index of the curve set
*  @return			Matrix of terms and corresponding discount factors from the yield curve
*/
std::vector<std::vector<double> > meCurveDisplay(const std::string& curveCollection, const std::string& curveIndex)
{
	AQ_API_START
    DoubleMatrix ret;
	
    // marshall all inputs		
	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
		
	DoubleArray ans = validation::tryMeCurveDisplay(tmp_curveCollection, tmp_curveIndex);

    LADataHolder* dh = &(etrading::InitializeAQETrading::instance().dataInstance()->getObjectPool().getObject(tmp_curveCollection,ENCHKTYPE_ISDEFINED).getData(IR_CALIBRATION_DATA_DFS2));
	if(dh->isDefined() && !dh->isNull() && tmp_curveIndex == STD)
	{
		DoubleVector terms;
		DoubleVector dfs;
		DoubleVector df2s;

		double N = floor(double(ans.size()/3));//because N is terms.size +df.size()+ df2.size();
		for(size_t i=0; i<size_t(N); ++i)
		{
			terms.push_back(ans[3 * i]);
			dfs.push_back(ans[3 * i + 1]);
			df2s.push_back(ans[3 * i + 2]);
		}

		ret.push_back(terms);
		ret.push_back(dfs);
		ret.push_back(df2s);
	}
	else
	{
		DoubleVector terms;
		DoubleVector dfs;

		double N = floor(double(ans.size()/2));//because N is terms.size +df.size();
			
		for(size_t i=0; i<size_t(N); ++i)
		{
			terms.push_back(ans[2 * i]);
			dfs.push_back(ans[2 * i + 1]);
		}

		ret.push_back(terms);
		ret.push_back(dfs);
	}

    return ret;
    AQ_API_END
}