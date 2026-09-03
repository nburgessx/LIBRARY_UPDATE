/*!
	@file LAQuantLibArray.cpp
	@brief array class for math utility using external library

 */

#include "LAQuantLibArray.h"
#include "LAQuantLibProblem.h"


LAQuantLibArray::LAQuantLibArray(size_t size)
{
	mArray = new QuantLib::Array(size);
}

LAQuantLibArray::LAQuantLibArray(size_t size, double value)
{
	mArray = new QuantLib::Array(size, value);
}

LAQuantLibArray::LAQuantLibArray(size_t size, double value, double increment)
{
	mArray = new QuantLib::Array(size, value, increment);
}

LAQuantLibArray::LAQuantLibArray(const LAQuantLibArray& from)
{
	mArray = new QuantLib::Array(*from.mArray);
}

LAQuantLibArray::LAQuantLibArray(const QuantLib::Array& from)
{
	mArray = new QuantLib::Array(from);
}

LAQuantLibArray::LAQuantLibArray(const DoubleVector& from)
{
	//furuya//
	//mArray = new QuantLib::Array(from);
	mArray = new QuantLib::Array(from.size());
}

LAQuantLibArray::~LAQuantLibArray()
{
	delete mArray;
}

double LAQuantLibArray::operator[](size_t i) const
{
	return (*mArray)[i];
}

double& LAQuantLibArray::operator[](size_t i)
{
	return (*mArray)[i];
}
