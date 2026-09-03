#pragma once

#include <ql/math/optimization/endcriteria.hpp>


class AQLQuantLibEndCriteria
{
public:
	typedef QuantLib::EndCriteria::Type Type;

	AQLQuantLibEndCriteria(size_t maxIterations,
						  size_t maxStationaryStateIterations,
						  double rootEpsilon,
						  double functionEpsilon,
						  double gradientNormEpsilon);

	~AQLQuantLibEndCriteria();

	QuantLib::EndCriteria* getEndCriteria() const { return mEndCriteria; }

private:
	QuantLib::EndCriteria *mEndCriteria;
};
