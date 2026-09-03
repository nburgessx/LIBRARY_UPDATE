/* 
 * @brief			Collection of utility methods to do with data types
 * @Created:		14 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "stdafx.h"
#include "TypeUtilities.h"

namespace swig
{
	/* @brief		build StringMatrix from a vector of string vectors
	*  @param [out]		sMatrix			the string matrix object being built
	*  @param [in]		rhs				a vector of string vectors
	*/
	void buildStringMatrix(StringMatrix& sMatrix, const std::vector<std::vector <std::string> >& rhs)
	{
		sMatrix.clear();

		for(size_t i=0; i<rhs.size(); ++i)
		{
			std::vector<LAString> mbStr;
			std::vector<std::string> vec = rhs[i];
			for(size_t j=0; j<vec.size(); ++j)
			{
				mbStr.push_back(LAString(vec[j].c_str()));
			}

			sMatrix.push_back(mbStr);
		}
	}

	/* @brief			build DateVector from a vector of strings 
	*  @param [out]		dVector			A vector of dates
	*  @param [in]		rhs				a vector of strings
	*/
	void buildDateVector(DateVector& dVector, const std::vector<std::string>& rhs)
	{
		dVector.clear();

		for(size_t i=0; i<rhs.size(); ++i)
		{
			LADate tempDate(rhs[i].c_str(),"YYYYMMDD");
			dVector.push_back(tempDate);
		}
	}

	/* @brief			build StringVector from a vector of strings 
	*  @param [out]		sVector			A StringVector object
	*  @param [in]		rhs				a vector of strings
	*/
	void buildStringVector(StringVector& sVector, const std::vector<std::string>& rhs)
	{
		sVector.clear();

		for(size_t i=0; i<rhs.size(); ++i)
		{
			LAString temp(rhs[i].c_str());
			sVector.push_back(temp);
		}
	}

	/* @brief			build a vector of strings from a DoubleVector object
	*  @param [in]		inVal	input value as a string
	*  @output			output value as a LAString
	*/
	LAString fromStringToLAString(const std::string& inVal) 
	{
		LAString outVal(inVal.c_str()); 
		return outVal; 
	}

	/* @brief			build a matrix of strings from a StringMatrix object
	*  @param [in]		inVal	a StringMatrix object
	*  @output			output a matrix of strings
	*/
	std::vector<std::vector<std::string>> fromStringMatrixToMatrixOfString(const StringMatrix& inVal)
	{
		std::vector<std::vector<std::string> > ret;
		for (size_t i=0; i<inVal.size(); ++i)
		{
			std::vector<std::string> tempVec;
			for (size_t j=0; j<inVal[0].size(); ++j)
			{
				tempVec.push_back(inVal[i][j].getCString());
			}
			ret.push_back(tempVec);
		}
		return ret;
	}

				
}
