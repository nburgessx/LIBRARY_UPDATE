#include "TestHelperUtilities.h"

namespace google_test
{
    /* @brief			Find a value by a given key from a ReadDataFile object
    * @param [out]		val				Value of the given key
	* @param [in]		collection		ReadDataFile object
    * @param [in]		keys			All keys in the ReadDataFile object
    * @param [in]		key				Key that gives value
	* @param [in]		optional		Is optional key?
    */
	void findValByKey(AQLString& val, const etrading::ReadDataFile& collection, const std::set<AQLString>& keys, const AQLString& key, bool optional)
	{
		AQLString temp(key);
		temp.toUpper();
		if (keys.find(AQLString(temp).toUpper()) != keys.end())
		{
			val = collection(temp, "col1");
		}
		else
		{
			if (!optional)
			{
				AQLString err = "#Err: Failed to find key '" + key + "'. Please correct input file.";
				throw err.getCString();
			}
		}
	}

	/* @brief			Return the short name of interpolation types
	* @param [in]		interpolation	Interpolation in long name
    * @output			interpolation short name
    */
	AQLString interpolationShortName(const AQLString& interpolation)
	{
		AQLString temp(interpolation);
		temp.toUpper();
		AQLString ret;
		if (temp == "FN_SPLINEINTERPOLATION")
		{
			ret = "SPLINE";
		}
		else if (temp == "FN_MONOTONESPLINEINTERPOLATION")
		{
			ret = "MONOTONESPLINE";
		}
		else if (temp == "FN_MONOTONEPARABOLICINTERPOLATION")
		{
			ret = "MONOTONEPARABOLIC";
		}
		else if (temp == "FN_MONOTONECONVEXINTERPOLATION")
		{
			ret = "MONOTONECONVEX";
		}
		else if (temp == "FN_LINEARINTERPOLATION")
		{
			ret = "LINEAR";
		}
        else if (temp == "FN_NATURALSPLINEINTERPOLATION")
		{
			ret = "NATURALSPLINE";
		}
        else if (temp == "FN_CLAMPEDSPLINEINTERPOLATION")
		{
			ret = "CLAMPEDSPLINE";
		}
        else if (temp == "FN_PARABOLICINTERPOLATION")
		{
			ret = "PARABOLIC";
		}
        else if (temp == "FN_LINEARSPLINEINTERPOLATION")
		{
			ret = "LINEARSPLINE";
		}
		else if (temp == "FN_LINEARMONOTONESPLINEINTERPOLATION")
		{
			ret = "LINEARMONOTONESPLINE";
		}
		else if (temp == "FN_LINEARMONOTONEPARABOLICINTERPOLATION")
		{
			ret = "LINEARMONOTONEPARABOLIC";
		}
		else if (temp == "FN_CONSTRAINEDSPLINEINTERPOLATION")
		{
			ret = "CONSTRAINEDSPLINE";
		}
		return ret;
	}

	double getBondSpreadTolerance()
	{
		const double spreadTolerance = 1e-6;        // Spread is reported in BPs
		const double spreadTolerance_64 = 1e-5;     // Spread is reported in BPs

		#if defined(GTEST32)  
				return spreadTolerance;
		#else
				return spreadTolerance_64;
		#endif
	}

}