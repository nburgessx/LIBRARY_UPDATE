/*
*/

/*! \MMGaussLobattointegral.h
\brief
*/

#ifndef MMGAUSSLOBATTOINTEGRAL_H
#define MMGAUSSLOBATTOINTEGRAL_H


#include <limits>
#include <cmath>
#include <stdexcept>
#include "AQLCoreAppError.h" 

//using namespace std;

class Integrator
{
public:
    Integrator(double absoluteAccuracy, size_t maxEvaluations);

	virtual ~Integrator() {}

    void setAbsoluteAccuracy(double);

	void setMaxEvaluations(size_t);

	double absoluteAccuracy() const;

	size_t maxEvaluations() const;

    double absoluteError() const ;

    size_t numberOfEvaluations() const;

    virtual bool integrationSuccess() const;

protected:
    
	void setAbsoluteError(double error) const;
    
	void setNumberOfEvaluations(size_t evaluations) const;
    
	void increaseNumberOfEvaluations(size_t increase) const;

	mutable size_t evaluations_;

private:
    double absoluteAccuracy_;
    
	mutable double absoluteError_;
    
	size_t maxEvaluations_;
};


class GaussLobattoIntegral : public Integrator
{
public:
    GaussLobattoIntegral(size_t maxIterations,
                         double absAccuracy,
                         double relAccuracy = std::numeric_limits<double>::max(),
                         bool useConvergenceEstimate = true
						);

	template <class C>
    double operator()(const C& f,
		              double a,
					  double b) const
	{
		evaluations_ = 0;
		if (a == b)
			return 0.0;
		if (b > a)
			return integrate(f, a, b);
		else
			return -integrate(f, b, a);
	}


protected:

	template <class C>
	double integrate (const C& f,
                      double a,
					  double b
					 ) const
	{
	    setNumberOfEvaluations(0);
		const double calcAbsTolerance = calculateAbsTolerance(f, a, b);

	    increaseNumberOfEvaluations(2);
		return adaptivGaussLobattoStep(f, a, b, f(a), f(b), calcAbsTolerance);
	}

	template <class C>
    double adaptivGaussLobattoStep(const C& f,
                                   double a, double b, double fa, double fb,
                                   double acc
								  ) const
   {
	   	if( maxEvaluations() < numberOfEvaluations())
		{
			throw AQLCoreInvalidData("max number of iterations reached @ GaussLobattoIntegral::adaptivGaussLobattoStep", __FILE__, __LINE__);
		}
 
		const double h=(b-a)/2; 
		const double m=(a+b)/2;
    
		const double mll=m-alpha_*h; 
		const double ml =m-beta_*h; 
		const double mr =m+beta_*h; 
		const double mrr=m+alpha_*h;
    
		const double fmll= f(mll);
		const double fml = f(ml);
		const double fm  = f(m);
		const double fmr = f(mr);
		const double fmrr= f(mrr);
		increaseNumberOfEvaluations(5);
    
		const double integral2=(h/6)*(fa+fb+5*(fml+fmr));
		const double integral1=(h/1470)*(77*(fa+fb)
                                   +432*(fmll+fmrr)+625*(fml+fmr)+672*fm);
    
	    // avoid 80 bit logic on x86 cpu
		volatile double dist = acc + (integral1-integral2);
		if(dist==acc || mll<=a || b<=mrr)
		{
		   	if( !(m>a && b>m) )
			{
				throw AQLCoreInvalidData("Interval contains no more machine number @ GaussLobattoIntegral::adaptivGaussLobattoStep ", __FILE__, __LINE__);
			}
			return integral1;
	    }
		else
		{
			return  adaptivGaussLobattoStep(f,a,mll,fa,fmll,acc)  
				  + adaptivGaussLobattoStep(f,mll,ml,fmll,fml,acc)
				  + adaptivGaussLobattoStep(f,ml,m,fml,fm,acc)
				  + adaptivGaussLobattoStep(f,m,mr,fm,fmr,acc)
				  + adaptivGaussLobattoStep(f,mr,mrr,fmr,fmrr,acc)
                  +	adaptivGaussLobattoStep(f,mrr,b,fmrr,fb,acc);
		}
	}

	template <class C>	
    double calculateAbsTolerance(const C& f,
	                             double a, double b
								) const
	{
		double eps = std::numeric_limits<double>::epsilon();
		double relTol = std::max(relAccuracy_, eps);
    
		const double m = (a+b)/2; 
		const double h = (b-a)/2;
		const double y1 = f(a);
		const double y3 = f(m-alpha_*h);
		const double y5 = f(m-beta_*h);
		const double y7 = f(m);
		const double y9 = f(m+beta_*h);
		const double y11= f(m+alpha_*h);
		const double y13= f(b);

		double acc = h * ( 0.0158271919734801831*(y1+y13)
				         + 0.0942738402188500455*(f(m-x1_*h)+f(m+x1_*h))
				         + 0.1550719873365853963*(y3+y11)
				         + 0.1888215739601824544*(f(m-x2_*h)+ f(m+x2_*h))
				         + 0.1997734052268585268*(y5+y9) 
				         + 0.2249264653333395270*(f(m-x3_*h)+f(m+x3_*h))
				         + 0.2426110719014077338*y7
					     );  
	    
		increaseNumberOfEvaluations(13);
		if(acc == 0.)
		{
			throw AQLCoreInvalidData("can not calculate absolute accuracy from relative accuracy @ GaussLobattoIntegral::adaptivGaussLobattoStep", __FILE__, __LINE__);
		}
		
		double r = 1.;
		if (useConvergenceEstimate_)
		{
			const double integral2 = (h/6)*(y1+y13+5*(y5+y9));
			const double integral1 = (h/1470)*(77*(y1+y13)+432*(y3+y11)+
											 625*(y5+y9)+672*y7);
	    
			if (std::fabs(integral2-acc) != 0.0) 
				r = std::fabs(integral1-acc)/std::fabs(integral2-acc);
			if (r == 0.0 || r > 1.0)
				r = 1.0;
		}

		if (relAccuracy_ != std::numeric_limits<double>::max())
		{
			return std::min(absoluteAccuracy(), acc * relTol) / (r * eps);
		}
		else
		{
			return absoluteAccuracy() /(r * eps);
		}

	}

    double relAccuracy_;
    const bool useConvergenceEstimate_;
    const static double alpha_, beta_, x1_, x2_, x3_;
};
#endif
