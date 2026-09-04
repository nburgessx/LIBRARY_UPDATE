/*!
	@file AQLQuantLibArray.cpp
	@brief array class for math utility using external library
 */

#include "AQLQuantLibArray.h"
#include "AQLQuantLibProblem.h"


AQLQuantLibArray::AQLQuantLibArray(size_t size)
{
	mArray = new QuantLib::Array(size);
}

AQLQuantLibArray::AQLQuantLibArray(size_t size, double value)
{
	mArray = new QuantLib::Array(size, value);
}

AQLQuantLibArray::AQLQuantLibArray(size_t size, double value, double increment)
{
	mArray = new QuantLib::Array(size, value, increment);
}

AQLQuantLibArray::AQLQuantLibArray(const AQLQuantLibArray& from)
{
	mArray = new QuantLib::Array(*from.mArray);
}

AQLQuantLibArray::AQLQuantLibArray(const QuantLib::Array& from)
{
	mArray = new QuantLib::Array(from);
}

AQLQuantLibArray::AQLQuantLibArray(const DoubleVector& from)
{
	//furuya//
	//mArray = new QuantLib::Array(from);
	mArray = new QuantLib::Array(from.size());
}

AQLQuantLibArray::~AQLQuantLibArray()
{
	delete mArray;
}

double AQLQuantLibArray::operator[](size_t i) const
{
	return (*mArray)[i];
}

double& AQLQuantLibArray::operator[](size_t i)
{
	return (*mArray)[i];
}
