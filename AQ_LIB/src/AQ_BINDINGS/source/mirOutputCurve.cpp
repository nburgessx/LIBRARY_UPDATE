/* 
 * @brief			Swig interface to Java for function mirOutputCurve
 * @Created:		31 March 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "InitializeMLibETrading.h"
#include "mirOutputCurve.h"
#include "LAString.h"
#include "tryMirOutputCurve.h"
#include "LACurvePricingObject.h"

/* @brief			swig interface for mirOutputCurve
*  @param [in]		curveId			ID of the curve set
*  @param [in]		curveName		Name of the curve set
*  @return			Matrix of terms and corresponding discount factors from the yield curve
*/
std::vector<std::vector<double> > mirOutputCurve(const std::string& curveId,
								const std::string& curveName)
{
	DoubleMatrix ret;
	try 
	{
		// marshall all inputs		
		LAString curId			(curveId.c_str());
		LAString curName		(curveName.c_str());
		
		DoubleArray ans = validation_api::tryMirOutputCurve(etrading::InitializeMLibETrading::instance().dataInstance(),
												            curId,
												            curName);

		LADataHolder* dh = &(etrading::InitializeMLibETrading::instance().dataInstance()->getObjectPool().getObject(curId,ENCHKTYPE_ISDEFINED).getData(IR_CALIBRATION_DATA_DFS2));
		if(dh->isDefined() && !dh->isNull() && curveName == STD)
		{
			DoubleVector terms;
			DoubleVector dfs;
			DoubleVector df2s;

			double N = floor(double(ans.size()/3));//because N is terms.size +df.size()+ df2.size();
			for(size_t i=0; i<(size_t)N; ++i)
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
			
			for(size_t i=0; i<(size_t)N; ++i)
			{
				terms.push_back(ans[2 * i]);
				dfs.push_back(ans[2 * i + 1]);
			}

			ret.push_back(terms);
			ret.push_back(dfs);
		}


	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;

}