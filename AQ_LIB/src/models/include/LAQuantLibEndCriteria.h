#pragma once

#include <ql/math/optimization/endcriteria.hpp>


class LAQuantLibEndCriteria
{
public:
	typedef QuantLib::EndCriteria::Type Type;

	LAQuantLibEndCriteria(size_t maxIterations,
						  size_t maxStationaryStateIterations,
						  double rootEpsilon,
						  double functionEpsilon,
						  double gradientNormEpsilon);

	~LAQuantLibEndCriteria();

	QuantLib::EndCriteria* getEndCriteria() const { return mEndCriteria; }

private:
	QuantLib::EndCriteria *mEndCriteria;
};
