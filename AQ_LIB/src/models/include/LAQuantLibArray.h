#pragma once      


#include <ql/math/array.hpp>
#include "AQLCoreTemplateType.h"
#include "LAQuantLibDisposable.h"


class LAQuantLibArray
{
public:
	explicit LAQuantLibArray(size_t size = 0);
	LAQuantLibArray(size_t size, double value);
	LAQuantLibArray(size_t size, double value, double increment);
	LAQuantLibArray(const LAQuantLibArray& from);
	LAQuantLibArray(const QuantLib::Array& from);
	LAQuantLibArray(const DoubleVector& from);

	~LAQuantLibArray();

	double operator[](size_t i) const;
	double& operator[](size_t i);

	QuantLib::Array* getArray() const { return mArray; }

	size_t size() { return mArray->size(); }

private:
	QuantLib::Array *mArray;
};
