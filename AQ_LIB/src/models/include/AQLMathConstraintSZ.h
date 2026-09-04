#ifndef __AQLMATHCOSTRAINTSZ_H__
#define __AQLMATHCOSTRAINTSZ_H__

#include <ql/math/optimization/constraint.hpp>
#include <vector>

using namespace std;
using namespace QuantLib;

class AQLMathYieldCurve;

//  Constraint for optimizing SZ++ models
class AQLMathConstraintSZ : public Constraint
{
public:
    AQLMathConstraintSZ(double max_beta,
				  double min_beta,
		   		  double max_theta,
				  double min_theta,
					  double max_kappa,
					  double min_kappa,
		   		  double max_epsilon,
					  double min_epsilon,
					  const vector<bool>& calibFlag
				  );

private:
    class Impl : public Constraint::Impl
    {
    public:
        Impl(double max_beta,
			 double min_beta,
		   	 double max_theta,
			 double min_theta,
			  double max_kappa,
			  double min_kappa,
		   	 double max_epsilon,
			  double min_epsilon,
			  const vector<bool>& calibFlag
			);

		Impl(const Impl& rhs);

		virtual ~Impl();
        
        bool test(const Array& p) const;

    private:

		double max_beta;
		double min_beta;
		
		double max_theta;
		double min_theta;
		
		double max_kappa;
		double min_kappa;
		
		double max_epsilon;
		double min_epsilon;

		vector<bool> calibFlag;
		
		bool cloned;
    };
};

#endif
