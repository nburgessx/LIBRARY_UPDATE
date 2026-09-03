#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAFunctionUtilities.h"

////////////////////////////MLib-Userfunc
//change str delete space and upper 
void 
LAFunctionUtilities::upperdelspace(LAString& input)
{
	//remove space 
	LAString tmp;
	std::vector<LAString> tmpvec = input.toToken(' ');
	for(unsigned int c=0;c < tmpvec.size(); c++)
		tmp += tmpvec[c];
	input = tmp.toUpper();
}

//change str delete space and lwoer 
void 
LAFunctionUtilities::lowerdelspace(LAString& input)
{
	//remove space 
	LAString tmp;
	std::vector<LAString> tmpvec = input.toToken(' ');
	for(unsigned int c=0;c < tmpvec.size(); c++)
		tmp += tmpvec[c];
	input = tmp.toLower();
}

//change str-vector delete space and upper 
void 
LAFunctionUtilities::upperdelspace(LAStringVector& vec)
{
	int N=vec.size();
	for(int i=0; i<N ;i++)
		LAFunctionUtilities::upperdelspace(vec[i]);
}

//change str-vector delete space and lower 
void 
LAFunctionUtilities::lowerdelspace(LAStringVector& vec)
{
	int N=vec.size();
	for(int i=0; i<N ;i++)
		LAFunctionUtilities::lowerdelspace(vec[i]);
}

//change str-matrix delete space and upper 
void 
LAFunctionUtilities::upperdelspace(LAStringMatrix& mat)
{
	int N=mat.size();
	int M= (N>0) ? mat[0].size() : 0;

	for(int i=0; i<N ;i++)
		for(int j=0; j<M ;j++)
			LAFunctionUtilities::upperdelspace(mat[i][j]);
}

LAString
LAFunctionUtilities::getEntityPoolName(LADataInstance* dataInstance)
{
	LAString ret("");
	const LAObjectPool & objPool = dataInstance->getObjectPool();
	EntityConstIter it;
	if(objPool.getSize())
	{
		for(it = objPool.begin(); it != objPool.end(); it++)
		{		
			ret += it->first;
			if(it!=objPool.end())
				ret+= ":";
		}		
	}
	else
	{
		ret+= "No Object!!";
	}
	
	return ret;
}

LAString 
LAFunctionUtilities::findElement(const LAStringMatrix& mat, const LAString& input, const int row, const int col, bool ischangeroworcol, bool isemptyerror)
{
	
	LAString ret;
	if(ischangeroworcol)
	{	
		const int tmprow= LAFunctionUtilities::findRowsNumber(mat,input);
		if(tmprow < 0)
		{
			LAString empty("");
			if(isemptyerror)
			{
				LAString msg = "Key " + input + " not found in input matrix";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else
				return empty;
		}
		ret = mat[tmprow][col];
	}
	else	
	{
		const int tmpcol= LAFunctionUtilities::findColsNumber(mat,input);
		if(tmpcol < 0)
		{
			LAString empty("");
			if(isemptyerror)
			{
				LAString msg = "Key " + input + " not found in input matrix";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else
				return empty;
		}

		ret = mat[row][tmpcol];
	}
	return ret;
}

const int  
LAFunctionUtilities::findRowsNumber(const LAStringMatrix& defmat, const LAString& input)
{
	int i=0;
	int N = defmat.size();
	for(; i<N;i++)
	{
		if( input== defmat[i][0])
			break;
		else if(i!=N-1)
			continue;
		else if(i==N-1)
			return -1;	
	}
	
	i = (0 != i || N != 0) ? i : -1;

	return i;
}

const int  
LAFunctionUtilities::findColsNumber(const LAStringMatrix& defmat, const LAString& input)
{
	int j=0;
	int M = defmat[0].size();
	for(; j<M;j++)
	{
		if( input== defmat[0][j])
			break;
		else if(j!=M-1)
			continue;
		else if(j==M-1)
			return -1;	
	}

	j = (0 != j || M != 0) ? j : -1;
	
	return j;
}

LAString LAFunctionUtilities::findElement(const LAStringMatrix& matrix, const LAString& key)
{
    try
    {
        return LAFunctionUtilities::findElement(matrix, key, 0, 1, true);
    }
    catch (LACoreError &e)
    {
        LAString cpKey(key);
        upper(cpKey);
        return LAFunctionUtilities::findElement(matrix, cpKey, 0, 1, true);
    }
}

bool LAFunctionUtilities::StringToBool(LAString s)
{
    upper(s);
    return (s == "TRUE");
}

/// Search for exact string in vector of string
bool TryFind(const LAString& searchedItem, const vector<LAString>& inVector, size_t& foundIdx)
{
    foundIdx = std::distance(inVector.begin(), std::find(inVector.begin(), inVector.end(), searchedItem));
    return !(foundIdx < 0 || foundIdx >= inVector.size());
}

/// Search for substring within string
bool TryFind(const LAString& searchedItem, const LAString& searchedString, size_t& foundIdx)
{
    string refString(searchedString.getCString());
    string srcString(searchedItem.getCString());
    foundIdx = refString.find(srcString);
    return !(foundIdx == string::npos);
}
