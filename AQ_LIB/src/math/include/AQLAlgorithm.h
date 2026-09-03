#pragma once

#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 ) // cancel warning for specific exception
#endif



//===================== AQLAlgorithm =============================================
/*! 
    @brief Class to provide algorithms
*/
class AQLAlgorithm 
{
public:
	// find s (=t[pos]) between t[start] and t[end].
    /*!
		@param[in] t	sorted array that support access operator[] 
		@param[in] s	element value to search 
		@param[in] start	start position of search resion.
		@param[in] end	end position of search resion.
		@param[out] pos	output 
		@return find or not
		@note t must be sorted.
	*/
	template<class T, class S>	
	static bool	find(const T& t, const S& s, unsigned int start, unsigned int end,  unsigned int& pos)
	{
		unsigned int ju, jm, jl;
		jl = start;
		ju = end + 2;
		while (ju - jl > 1)
		{
			jm = (ju + jl) >> 1;
			if (s == t[jm - 1])
			{
				pos = jm-1;
				return true;
			}
			else if (s > t[jm - 1])
				jl = jm;
			else
				ju = jm;
		}
		if (jl == start || jl == end + 1)return false;
		else if (s == t[jl - 1])
		{
			pos = jl - 1;
			return true;
		}
		else if (s == t[jl])
		{
			pos = jl;
			return true;
		}		
		return false;	
	}
	
	// find pos s.t. s<=t[pos] and s>t[pos-1]. if s>t[pos] then pos=size.if s<=t[0] then pos=0. 
    /*!
		@param[in] searchArray		sorted search array that support access operator[] 
		@param[in] target			element value to search 
		@param[in] arraySize		size of sortedArray
		@param[out] position		output: result position 
	*/
	template<class T, class S>	
	static void	locate(const T& searchArray, const S& target, unsigned int searchArraySize, unsigned int& position )
	{
		unsigned int ju, jm, jl;
		jl = 0;
		ju = searchArraySize + 1;
		while(ju - jl > 1)
		{
			jm = (ju + jl) >> 1;
			if (target > searchArray[jm - 1])
				jl = jm;
			else
				ju = jm;
		}
		if (jl == searchArraySize) position = jl;
		else if (jl == 0) position = 0;
		else if (searchArray[jl - 1] == target) position = jl - 1;
		else if (searchArray[jl] == target) position = jl;
		else position = jl;
	}
};

