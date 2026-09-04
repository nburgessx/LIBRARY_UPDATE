#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <cmath>
#include "AQLCoreUtility.h"
#include "AQLMathDateUtilities.h"

using namespace std;

AQLStringVector 
AQLCoreUtility::findVector(AQLStringMatrix& mat, const AQLString& input,bool ischangeroworcol, bool isemptyerror)
{
	//#define chrcol this source is almost similar to AQLMathCurveGenerateFuncUtility	
	AQLStringVector ret;
	if(ischangeroworcol)
	{	
		const int tmprow= AQLCoreUtility::findRowsNumber(mat,input);
		if(tmprow < 0)
		{
			AQLStringVector empty(1,"");
			if(isemptyerror)
			{
				AQLString msg = input + "is not registerd in XLLPLUSINPUT.h";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else
				return empty;
		}
		ret = mat[tmprow];
	}
	else	
	{
		const int tmpcol= AQLCoreUtility::findColsNumber(mat,input);
		if(tmpcol <0)
		{
			AQLStringVector empty(1,"");
			if(isemptyerror)
			{
				AQLString msg = input + "is not registerd in XLLPLUSINPUT.h";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else
				return empty;
		}
		ret.resize(mat.size());
		for(unsigned int i=0;i<mat.size();i++)
			ret[i] = mat[i][tmpcol];
		
	}
	ret.erase(ret.begin());
	return ret;
}

AQLString 
AQLCoreUtility::findElement(AQLStringMatrix& mat, const AQLString& input, const int row, const int col, bool ischangeroworcol, bool isemptyerror)
{
	AQLString ret;
	if(ischangeroworcol)
	{	
		const int tmprow= AQLCoreUtility::findRowsNumber(mat,input);
		if(tmprow < 0)
		{
			AQLString empty("");
			if(isemptyerror)
			{
				AQLString msg = input + "is not registerd in XLLPLUSINPUT.h";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else
				return empty;
		}
		if(mat[0].size()<=static_cast<unsigned int>(col))
			throw AQLCoreInvalidData("Column Size is wrong",__FILE__,__LINE__);
		ret = mat[tmprow][col];
	}
	else	
	{
		const int tmpcol= AQLCoreUtility::findColsNumber(mat,input);
		if(tmpcol <0)
		{
			AQLString empty("");
			if(isemptyerror)
			{
				AQLString msg = input + "is not registerd in XLLPLUSINPUT.h";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else
				return empty;
		}
		if(mat.size()<= static_cast<unsigned int>(row))
			throw AQLCoreInvalidData("Row Size is wrong",__FILE__,__LINE__);
		ret = mat[row][tmpcol];
	}
	return ret;
}

const int  
AQLCoreUtility::findRowsNumber(AQLStringMatrix& defmat, const AQLString& input)
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
	return i;
}

const int  
AQLCoreUtility::findColsNumber(AQLStringMatrix& defmat, const AQLString& input)
{
	int j=0;
	if(0==defmat.size())
		throw AQLCoreInvalidData("Matrix is NULL",__FILE__,__LINE__);
	//change
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
	return j;
}

void 
AQLCoreUtility::removeQuotation2(AQLString &input)
{
	while(input.findString('"')!=-1)
	{
		int i = input.findString('"');
		input.remove(i,1);
	}
	return;
}
		
AQLString
AQLCoreUtility::removeQuotation(const AQLString & input)
{
	AQLString ret;		
	std::vector<AQLString> tmpvec = input.toToken('"');
	for(unsigned int c=0;c < tmpvec.size(); c++)
		ret += tmpvec[c];
	return ret;
}

AQLStringVector
AQLCoreUtility::removeQuotation(const AQLStringVector& inputvec)
{
	int N=inputvec.size();
	if(0==N)
		throw AQLCoreInvalidData("stringvector size =0",__FILE__,__LINE__);
	
	AQLStringVector ret(N);
	for(int i=0;i<N;i++)
		ret.at(i) = removeQuotation(inputvec.at(i));
	return ret;
}

void 
AQLCoreUtility::removeQuotation(AQLStringVector& inputvec)
{
	int N=inputvec.size();
	if(0==N)
		throw AQLCoreInvalidData("stringvector size =0",__FILE__,__LINE__);

	for(int i=0;i<N;i++)
		inputvec.at(i) = removeQuotation(inputvec.at(i));
}

void 
AQLCoreUtility::xlDumpMemory(AQLDataInstance* dataInstance, const AQLString& file)
{	
		AQLObjectPool   & objPool = dataInstance->getObjectPool();
		for ( EntityConstIter it = objPool.begin(); it != objPool.end(); it++ )
		{
			it->second.print( file.getCString() );
		}
}

std::vector<const AQLObject*> 
AQLCoreUtility::findEntity(AQLDataInstance* dataInstance, const AQLString& keyname)
{
	std::vector<const AQLObject*> ret;
	const AQLObjectPool& objPool = dataInstance->getObjectPool();
	EntityConstIter it;
	for(it = objPool.begin();it!=objPool.end();++it)
	{
		AQLString tmp = it->first;
		if(tmp.findString(keyname) != -1)
		{
			ret.push_back(&it->second.get());
		}
	}
	if(0==ret.size())
		throw AQLCoreInvalidData("Cashlet does not exsist",__FILE__,__LINE__);

	return ret;
}

void 
AQLCoreUtility::dataLoad(const AQLString& fname, AQLDataInstance &dataInstance, bool isolddataclear, bool iscompletedependency)
{
	AQLCSVFileLoader fileLoader;
	fileLoader.setDataInstance(&dataInstance);

	AQLObject* e = new AQLObject;
	AQLStringVector sv(1);
	sv[0] = fname;

	e->add(CALIBRATION_DATA_MD_FILEPATHS,new AQLDataStrings(sv));
	Records_var ret = fileLoader.get(AQLObjectHolder(e,true));

	AQLObjectPool&	objPool = dataInstance.getObjectPool();
	// clear old data
	if(isolddataclear)
		objPool.clear();
	
	std::vector<AQLObjectHolder>::iterator it;
	for (it = ret->begin(); it != ret->end(); ++it)
	{
		objPool.set(it->getName(), &(it->get()));
	}
	
	if(iscompletedependency)
		dataInstance.getReferencePool().completeDependency();
	
	return;
};

DoubleArray 
AQLCoreUtility::changeDoubleFromString(const AQLStringVector& strvec)
{
	DoubleArray ret(strvec.size());
	for(unsigned int i=0;i<ret.size();i++)
		ret[i] = strvec[i].getDoubleValue();
	return ret;
};

DoubleMatrix 
AQLCoreUtility::changeDoubleFromString(const AQLStringMatrix& strMat)
{
    DoubleMatrix ret(strMat.size(), DoubleVector(strMat[0].size()));
    for(size_t i=0; i<strMat.size(); i++)
    {
        for(size_t j=0; j<strMat[0].size(); j++)
        {
            ret[i][j] = strMat[i][j].getDoubleValue();
        }
    }

    return ret;
};

DateVector 
AQLCoreUtility::changeDateFromString(const AQLStringVector& strvec)
{
	DateVector ret(strvec.size());
	for(unsigned int i=0;i<ret.size();i++)
		ret[i] = AQLMathDateUtilities::getAQLDate( (int) strvec[i].getDoubleValue() );
	return ret;
};

