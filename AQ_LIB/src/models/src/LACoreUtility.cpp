//  2008, AlgoQuantHub.
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
#include "LACoreUtility.h"
#include "LAMathDateUtilities.h"

using namespace std;

LAStringVector 
LACoreUtility::findVector(LAStringMatrix& mat, const LAString& input,bool ischangeroworcol, bool isemptyerror)
{
	//#define chrcol this source is almost similar to LAMathCurveGenerateFuncUtility	
	LAStringVector ret;
	if(ischangeroworcol)
	{	
		const int tmprow= LACoreUtility::findRowsNumber(mat,input);
		if(tmprow < 0)
		{
			LAStringVector empty(1,"");
			if(isemptyerror)
			{
				LAString msg = input + "is not registerd in XLLPLUSINPUT.h";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else
				return empty;
		}
		ret = mat[tmprow];
	}
	else	
	{
		const int tmpcol= LACoreUtility::findColsNumber(mat,input);
		if(tmpcol <0)
		{
			LAStringVector empty(1,"");
			if(isemptyerror)
			{
				LAString msg = input + "is not registerd in XLLPLUSINPUT.h";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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

LAString 
LACoreUtility::findElement(LAStringMatrix& mat, const LAString& input, const int row, const int col, bool ischangeroworcol, bool isemptyerror)
{
	LAString ret;
	if(ischangeroworcol)
	{	
		const int tmprow= LACoreUtility::findRowsNumber(mat,input);
		if(tmprow < 0)
		{
			LAString empty("");
			if(isemptyerror)
			{
				LAString msg = input + "is not registerd in XLLPLUSINPUT.h";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else
				return empty;
		}
		if(mat[0].size()<=static_cast<unsigned int>(col))
			throw LACoreInvalidData("Column Size is wrong",__FILE__,__LINE__);
		ret = mat[tmprow][col];
	}
	else	
	{
		const int tmpcol= LACoreUtility::findColsNumber(mat,input);
		if(tmpcol <0)
		{
			LAString empty("");
			if(isemptyerror)
			{
				LAString msg = input + "is not registerd in XLLPLUSINPUT.h";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else
				return empty;
		}
		if(mat.size()<= static_cast<unsigned int>(row))
			throw LACoreInvalidData("Row Size is wrong",__FILE__,__LINE__);
		ret = mat[row][tmpcol];
	}
	return ret;
}

const int  
LACoreUtility::findRowsNumber(LAStringMatrix& defmat, const LAString& input)
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
LACoreUtility::findColsNumber(LAStringMatrix& defmat, const LAString& input)
{
	int j=0;
	if(0==defmat.size())
		throw LACoreInvalidData("Matrix is NULL",__FILE__,__LINE__);
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
LACoreUtility::removeQuotation2(LAString &input)
{
	while(input.findString('"')!=-1)
	{
		int i = input.findString('"');
		input.remove(i,1);
	}
	return;
}
		
LAString
LACoreUtility::removeQuotation(const LAString & input)
{
	LAString ret;		
	std::vector<LAString> tmpvec = input.toToken('"');
	for(unsigned int c=0;c < tmpvec.size(); c++)
		ret += tmpvec[c];
	return ret;
}

LAStringVector
LACoreUtility::removeQuotation(const LAStringVector& inputvec)
{
	int N=inputvec.size();
	if(0==N)
		throw LACoreInvalidData("stringvector size =0",__FILE__,__LINE__);
	
	LAStringVector ret(N);
	for(int i=0;i<N;i++)
		ret.at(i) = removeQuotation(inputvec.at(i));
	return ret;
}

void 
LACoreUtility::removeQuotation(LAStringVector& inputvec)
{
	int N=inputvec.size();
	if(0==N)
		throw LACoreInvalidData("stringvector size =0",__FILE__,__LINE__);

	for(int i=0;i<N;i++)
		inputvec.at(i) = removeQuotation(inputvec.at(i));
}

void 
LACoreUtility::xlDumpMemory(LADataInstance* dataInstance, const LAString& file)
{	
		LAObjectPool   & objPool = dataInstance->getObjectPool();
		for ( EntityConstIter it = objPool.begin(); it != objPool.end(); it++ )
		{
			it->second.print( file.getCString() );
		}
}

std::vector<const LAObject*> 
LACoreUtility::findEntity(LADataInstance* dataInstance, const LAString& keyname)
{
	std::vector<const LAObject*> ret;
	const LAObjectPool& objPool = dataInstance->getObjectPool();
	EntityConstIter it;
	for(it = objPool.begin();it!=objPool.end();++it)
	{
		LAString tmp = it->first;
		if(tmp.findString(keyname) != -1)
		{
			ret.push_back(&it->second.get());
		}
	}
	if(0==ret.size())
		throw LACoreInvalidData("Cashlet does not exsist",__FILE__,__LINE__);

	return ret;
}

void 
LACoreUtility::dataLoad(const LAString& fname, LADataInstance &dataInstance, bool isolddataclear, bool iscompletedependency)
{
	MDCSVFileLoader fileLoader;
	fileLoader.setDataInstance(&dataInstance);

	LAObject* e = new LAObject;
	LAStringVector sv(1);
	sv[0] = fname;

	e->add(CALIBRATION_DATA_MD_FILEPATHS,new LADataStrings(sv));
	Records_var ret = fileLoader.get(LAObjectHolder(e,true));

	LAObjectPool&	objPool = dataInstance.getObjectPool();
	// clear old data
	if(isolddataclear)
		objPool.clear();
	
	std::vector<LAObjectHolder>::iterator it;
	for (it = ret->begin(); it != ret->end(); ++it)
	{
		objPool.set(it->getName(), &(it->get()));
	}
	
	if(iscompletedependency)
		dataInstance.getReferencePool().completeDependency();
	
	return;
};

DoubleArray 
LACoreUtility::changeDoubleFromString(const LAStringVector& strvec)
{
	DoubleArray ret(strvec.size());
	for(unsigned int i=0;i<ret.size();i++)
		ret[i] = strvec[i].getDoubleValue();
	return ret;
};

DoubleMatrix 
LACoreUtility::changeDoubleFromString(const LAStringMatrix& strMat)
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
LACoreUtility::changeDateFromString(const LAStringVector& strvec)
{
	DateVector ret(strvec.size());
	for(unsigned int i=0;i<ret.size();i++)
		ret[i] = LAMathDateUtilities::getLADate( (int) strvec[i].getDoubleValue() );
	return ret;
};

