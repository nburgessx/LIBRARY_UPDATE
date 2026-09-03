#pragma once

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include "LABasic.h"
#include "LAAlgorithm.h"

#if !defined(WIN32) && !defined(WIN64)
#include <values.h>
#endif

//===================== Class Declare LACalibrationUtilities==================================
/*! 
    @brief  Template util class

   
*/
template <class T>
class LACalibrationUtilities
{
public:
	//==============================================================================
	// check unique data
	static void checkUnique(const std::vector<T> &vec, const T &data, int &pos);
	//==============================================================================
	// delete vector pointer
	static void delPContainer(std::vector<T *> &c); 
	//==============================================================================
	// search nearest position of vector for target value	
	static void searchNearestPos(const std::vector<T> &vec, const T &data, unsigned int &pos);
	//==============================================================================
	// convert string value to numvalue	
	static T conv(const std::vector<T> &vec, const T &data, unsigned int &pos);


};

/*!
    @brief return nearest pos
	
	search nearest position of vector for target value	

	@param[in]     vec
	@param[in]     data
	@param[out]    pos

*/
template <class T> void 
LACalibrationUtilities<T>::searchNearestPos(const std::vector<T> &vec, const T &data, unsigned int &pos)
{
	
	LAAlgorithm::locate<std::vector<T>, T>(vec, data, vec.size(), pos);

	if (pos == vec.size())
	{
		--pos;
	}
	else if (pos != 0)
	{
		double diff = LAMath::abs(vec[pos] - data); 
		if (diff > LAMath::abs(data - vec[pos - 1]))
		{
			--pos;
		}
	}
}


/*!
    @brief check vector unique data

	check vector has unique data for target data.
	if no data or multi data throw LACoreInvalidData.

	@param[in]     vec
	@param[in]     data
	@param[out]    pos

*/
template <class T> void 
LACalibrationUtilities<T>::checkUnique(const std::vector<T> &vec, const T &data, int &pos)
{
	
	pos = INT_MIN;

	const int size = vec.size();
	for (int i = 0; i < size; ++i)
	{
		if (vec[i] == data)
		{
			// duplicate error
			if (pos != INT_MIN)
			{
				throw LACoreInvalidData("The data must be unique !!" , __FILE__, __LINE__);
			}
			// set position
			pos = i;
		}

	}
	// no data error
	if (pos == INT_MIN)
	{
		throw LACoreInvalidData("The data must !!" , __FILE__, __LINE__);
	}
}


/*!
    @brief delete container pointer
	
	@param[in,out] c     

*/
template<class T> void 
LACalibrationUtilities<T>::delPContainer(std::vector<T *> &c)
{
	typename std::vector<T *>::iterator it = c.begin();

	while (it != c.end())
	{
		delete *it;
	}
	
}
