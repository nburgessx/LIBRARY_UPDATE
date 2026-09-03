#pragma once      


#include <ql/math/array.hpp>
#include "AQLCoreTemplateType.h"
#include "AQLQuantLibDisposable.h"


class AQLQuantLibArray
{
public:
	explicit AQLQuantLibArray(size_t size = 0);
	AQLQuantLibArray(size_t size, double value);
	AQLQuantLibArray(size_t size, double value, double increment);
	AQLQuantLibArray(const AQLQuantLibArray& from);
	AQLQuantLibArray(const QuantLib::Array& from);
	AQLQuantLibArray(const DoubleVector& from);

	~AQLQuantLibArray();

	double operator[](size_t i) const;
	double& operator[](size_t i);

	QuantLib::Array* getArray() const { return mArray; }

	size_t size() { return mArray->size(); }

private:
	QuantLib::Array *mArray;
};
