#include <AQLCoreAppError.h>
#include <LAMathConstraintSZ.h>


//  Constraint for optimizing SZ++ models
LAMathConstraintSZ::LAMathConstraintSZ(double max_beta, double min_beta,
					   		 double max_theta, double min_theta,
									double max_kappa, double min_kappa,
		   							double max_epsilon, double min_epsilon,
									const vector<bool>& calibFlag
							)
:
Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>( new LAMathConstraintSZ::Impl(max_beta, min_beta,
										 							    max_theta, min_theta,
																					  max_kappa, min_kappa,
		   																			  max_epsilon, min_epsilon,
																					  calibFlag
																	   )
											  ))
{
}

//
LAMathConstraintSZ::Impl::Impl(double max_beta_, double min_beta_,
					   	  double max_theta_, double min_theta_,
							  double max_kappa_, double min_kappa_,
		   					  double max_epsilon_, double min_epsilon_,
							  const vector<bool>& calibFlag_
						 )
:
max_beta(max_beta_),
min_beta(min_beta_),
max_theta(max_theta_),
min_theta(min_theta_),
max_kappa(max_kappa_),
min_kappa(min_kappa_),
max_epsilon(max_epsilon_),
min_epsilon(min_epsilon_),
calibFlag(calibFlag_)
{	
}

//
LAMathConstraintSZ::Impl::Impl( const Impl& rhs )
:
max_beta(rhs.max_beta),
min_beta(rhs.min_beta),
max_theta(rhs.max_theta),
min_theta(rhs.min_theta),
max_kappa(rhs.max_kappa),
min_kappa(rhs.min_kappa),
max_epsilon(rhs.max_epsilon),
min_epsilon(rhs.min_epsilon),
calibFlag(rhs.calibFlag)
{
}

//
LAMathConstraintSZ::Impl::~Impl()
{
}


bool LAMathConstraintSZ::Impl::test(const Array& p) const
{
 	//double m = p.size();

	//if(m != 4) throw invalid_argument("p.size() != 4 : Constraint_SZ::Impl::test");
	//if(m != 3) throw AQLCoreInvalidData("p.size() != 4 : Constraint_SZ::Impl::test", __FILE__, __LINE__);

	size_t counter = 0;
	for (size_t i = 0; i < 4; ++i)
	{
		if(calibFlag[i])
		{
			double value = p[counter++];
			if(i == 0)
			{
				if(max_beta < value)	return false;
				if(value < min_beta)	return false;
			}
			else if(i == 1)
			{
				if(max_theta < value)	return false;
				if(value < min_theta)	return false;
			}
			else if(i == 2)
			{
				if(max_kappa < value)	return false;
				if(value < min_kappa)	return false;
			}
			else if(i == 3)
			{
				if(max_epsilon < value)	return false;
				if(value < min_epsilon)	return false;
			}
		}
	}

	return true;
}