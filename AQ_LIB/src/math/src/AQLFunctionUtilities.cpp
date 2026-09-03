#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLFunctionUtilities.h"

////////////////////////////AlgoQuantLib-Userfunc
//change str delete space and upper 
void 
AQLFunctionUtilities::upperdelspace(AQLString& input)
{
	//remove space 
	AQLString tmp;
	std::vector<AQLString> tmpvec = input.toToken(' ');
	for(unsigned int c=0;c < tmpvec.size(); c++)
		tmp += tmpvec[c];
	input = tmp.toUpper();
}

//change str delete space and lwoer 
void 
AQLFunctionUtilities::lowerdelspace(AQLString& input)
{
	//remove space 
	AQLString tmp;
	std::vector<AQLString> tmpvec = input.toToken(' ');
	for(unsigned int c=0;c < tmpvec.size(); c++)
		tmp += tmpvec[c];
	input = tmp.toLower();
}

//change str-vector delete space and upper 
void 
AQLFunctionUtilities::upperdelspace(AQLStringVector& vec)
{
	int N=vec.size();
	for(int i=0; i<N ;i++)
		AQLFunctionUtilities::upperdelspace(vec[i]);
}

//change str-vector delete space and lower 
void 
AQLFunctionUtilities::lowerdelspace(AQLStringVector& vec)
{
	int N=vec.size();
	for(int i=0; i<N ;i++)
		AQLFunctionUtilities::lowerdelspace(vec[i]);
}

//change str-matrix delete space and upper 
void 
AQLFunctionUtilities::upperdelspace(AQLStringMatrix& mat)
{
	int N=mat.size();
	int M= (N>0) ? mat[0].size() : 0;

	for(int i=0; i<N ;i++)
		for(int j=0; j<M ;j++)
			AQLFunctionUtilities::upperdelspace(mat[i][j]);
}

AQLString
AQLFunctionUtilities::getEntityPoolName(AQLDataInstance* dataInstance)
{
	AQLString ret("");
	const AQLObjectPool & objPool = dataInstance->getObjectPool();
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

AQLString 
AQLFunctionUtilities::findElement(const AQLStringMatrix& mat, const AQLString& input, const int row, const int col, bool ischangeroworcol, bool isemptyerror)
{
	
	AQLString ret;
	if(ischangeroworcol)
	{	
		const int tmprow= AQLFunctionUtilities::findRowsNumber(mat,input);
		if(tmprow < 0)
		{
			AQLString empty("");
			if(isemptyerror)
			{
				AQLString msg = "Key " + input + " not found in input matrix";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else
				return empty;
		}
		ret = mat[tmprow][col];
	}
	else	
	{
		const int tmpcol= AQLFunctionUtilities::findColsNumber(mat,input);
		if(tmpcol < 0)
		{
			AQLString empty("");
			if(isemptyerror)
			{
				AQLString msg = "Key " + input + " not found in input matrix";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else
				return empty;
		}

		ret = mat[row][tmpcol];
	}
	return ret;
}

const int  
AQLFunctionUtilities::findRowsNumber(const AQLStringMatrix& defmat, const AQLString& input)
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
AQLFunctionUtilities::findColsNumber(const AQLStringMatrix& defmat, const AQLString& input)
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

AQLString AQLFunctionUtilities::findElement(const AQLStringMatrix& matrix, const AQLString& key)
{
    try
    {
        return AQLFunctionUtilities::findElement(matrix, key, 0, 1, true);
    }
    catch (AQLCoreError &e)
    {
        AQLString cpKey(key);
        upper(cpKey);
        return AQLFunctionUtilities::findElement(matrix, cpKey, 0, 1, true);
    }
}

bool AQLFunctionUtilities::StringToBool(AQLString s)
{
    upper(s);
    return (s == "TRUE");
}

/// Search for exact string in vector of string
bool TryFind(const AQLString& searchedItem, const vector<AQLString>& inVector, size_t& foundIdx)
{
    foundIdx = std::distance(inVector.begin(), std::find(inVector.begin(), inVector.end(), searchedItem));
    return !(foundIdx < 0 || foundIdx >= inVector.size());
}

/// Search for substring within string
bool TryFind(const AQLString& searchedItem, const AQLString& searchedString, size_t& foundIdx)
{
    string refString(searchedString.getCString());
    string srcString(searchedItem.getCString());
    foundIdx = refString.find(srcString);
    return !(foundIdx == string::npos);
}
