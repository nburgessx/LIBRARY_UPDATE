#ifndef _LAMATHSWAPTIONFORMULA_H_
#define _LAMATHSWAPTIONFORMULA_H_

#include "AQLFunction.h"

#include <math.h>
#include <algorithm>
#include <AQLMathAnalyticalFormula.h>
#include "AQLDist.h"

#include <vector>
#include "AQLMathIntegralMulti.h"
#include <AQLMathYieldCurve.h>

using namespace std;

//
class DDSZ_util : public AQLFunction
{
public:

	DDSZ_util() {}
	DDSZ_util(AQLMathPathYieldCurve& curve_d_,
			  AQLMathPathYieldCurve& curve_f_,
              vector<double> T_param_,
              vector<double> beta_,
              double rho_,             
              vector<double> theta_,
              vector<double> kappa_,
              vector<double> epsilon_,
              double S0_,
              double V0_
             );

	virtual double operator()( double x );
	virtual double operator()( double x ) const = 0;

	size_t get_num_param() { return T_param.size(); }

	void Get_param(vector<double>& T_param_,
				   vector<double>& beta_,
				   vector<double>& theta_,
				   vector<double>& kappa_,
				   vector<double>& epsilon_
				  ) const;

	void Set_param(const vector<double>& T_param_,
		           const vector<double>& beta_,
				   const vector<double>& theta_,
				   const vector<double>& kappa_,
				   const vector<double>& epsilon_
				  );

	void Set_param(const vector<double>& T_param_,
		           const vector<double>& beta_,
				   const vector<double>& theta_,
				   const vector<double>& kappa_,
				   const vector<double>& epsilon_,
				   const map<double, double>& cathe_Int_E_theta_ 
				  );

	void Set_T_param(const vector<double>& T_param_) { T_param = T_param_; }
	void Set_beta(const vector<double>& beta_) { beta = beta_; }
	void Set_theta(const vector<double>& theta_) { theta = theta_; }
	void Set_kappa(const vector<double>& kappa_) { kappa = kappa_; }
	void Set_epsilon(const vector<double>& epsilon_) { epsilon = epsilon_; }

	void Set_theta_over_kappa(const vector<double>& theta_, const vector<double>& kappa_)
	{
		unsigned int size = theta_.size();
		theta_over_kappa.resize(size);
		for (unsigned int i = 0; i < size; ++i)
		{
			theta_over_kappa[i] = theta_[i] / kappa_[i];
		}
	}

	void Set_exp_kappa_t(const vector<double>& kappa_, const vector<double>& T_param_)
	{
		unsigned int size = kappa_.size();
		exp_kappa_t.resize(size);
		for (unsigned int i = 0; i < size; ++i)
		{
			exp_kappa_t[i].resize(size);
			for (unsigned int j = 0; j < size; ++j)
			{
				exp_kappa_t[i][j] = AQLMath::exp(kappa_[i] * T_param_[j]);
			}
		}
	}

	void Get_cathe_Int_E_theta(map<double, double>& cathe_Int_E_theta_) { cathe_Int_E_theta_ = cathe_Int_E_theta; }

protected:
	DDSZ_util(const DDSZ_util& rhs) {}
	
	////double fwd1( double t );
	inline double fwd1( double t ) const;
	////double fwd2( double t );
	inline double fwd2( double t )const;
	inline double E_fwd2(double t)const;

    ////double E(double t);
	inline double E(double t) const;
	////double vol1(double t);
	inline double vol1(double t)const;
	////double D1_vol1(double t);
	inline double D1_vol1(double t) const;
	////double D2_vol1(double t);
	inline double D2_vol1(double t) const;
	////double D11_vol1(double t);
	inline double D11_vol1(double t) const;
	////double D12_vol1(double t);
	inline double D12_vol1(double t) const;
	////double D22_vol1(double t);
	inline double D22_vol1(double t)const;

    ////double vol2(double t);
	inline double vol2(double t) const;
	////double D_vol2(double t);
	inline double D_vol2(double t) const;

	double S0;
	double V0;

    AQLMathPathYieldCurve* curve_d;
	AQLMathPathYieldCurve* curve_f;

    vector<double> T_param;
    vector<double> beta;

    double rho;
	double sigma;
    vector<double> theta;
    vector<double> kappa;
    vector<double> epsilon;
	vector<double> theta_over_kappa;
	DoubleMatrix exp_kappa_t;

    AQLMathFunction<DDSZ_util> Int_kappa;
    ////double kappa_(double t);
	inline double kappa_(double t) const;

    //Func<DDSZ_util> Int_E_theta;
    ////double E_theta_(double t);
	inline double E_theta_(double t) const;

	////double Int_E_theta(double ts, double te);
	inline double Int_E_theta(double ts, double te) const;

    map<double, double> cathe_Int_kappa;
	////map<double, double> cathe_Int_E_theta;
	mutable map<double, double> cathe_Int_E_theta;

	size_t pos_s;

    //
    ////size_t DDSZ_util::idx(double t);
	    size_t DDSZ_util::idx(double t) const;

	inline void calc_cathe_E_theta();
};

//
class Vol1_SQ_DDSV : public DDSZ_util
{
public:
	Vol1_SQ_DDSV(AQLMathPathYieldCurve& curve_d_,
     			 AQLMathPathYieldCurve& curve_f_,
                 vector<double> T_param_,      
                 vector<double> beta_,      
                 double rho_,                   
                 vector<double> theta_,
                 vector<double> kappa_,
				 vector<double> epsilon_,
                 double S0_,
                 double V0_
		        ) : DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_) {}

	////double operator() (double t) { double tmp = vol1(t); return tmp * tmp; }
	double operator() (double t) const { double tmp = vol1(t); return tmp * tmp; }

private:
	Vol1_SQ_DDSV(const Vol1_SQ_DDSV& rhs) {}
};

//
class Vol2_E_SQ_DDSV : public DDSZ_util
{
public:
	Vol2_E_SQ_DDSV(AQLMathPathYieldCurve& curve_d_,
     			   AQLMathPathYieldCurve& curve_f_,
                   vector<double> T_param_,      
                   vector<double> beta_,        
                   double rho_,       
                   vector<double> theta_,
                   vector<double> kappa_,
                   vector<double> epsilon_,
                   double S0_,
                   double V0_
		          ) : DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_) {}

	////double operator() (double t) { double tmp = E(t) * vol2(t); return tmp * tmp; }
	double operator() (double t) const { double tmp = E(t) * vol2(t); return tmp * tmp; }

private:
	Vol2_E_SQ_DDSV(const Vol2_E_SQ_DDSV& rhs) {}
};

//
class Rho_vol1_vol2_E_DDSV : public DDSZ_util
{
public:
	Rho_vol1_vol2_E_DDSV(AQLMathPathYieldCurve& curve_d_,
     			         AQLMathPathYieldCurve& curve_f_,
                         vector<double> T_param_,
                         vector<double> beta_,      
                         double rho_,                           
                         vector<double> theta_,
                         vector<double> kappa_,
                         vector<double> epsilon_,
                         double S0_,
                         double V0_
		                ) : DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_) {}

	////double operator() (double t) { return rho * vol1(t) * vol2(t) * E(t); }
	double operator() (double t) const 
	{
		// vol1 = fwd2, vol2 = epsilon
		//return rho * vol1(t) * vol2(t) * E(t);
		return rho * E_fwd2(t) * epsilon[idx(t)];
	}

private:
    Rho_vol1_vol2_E_DDSV(const Rho_vol1_vol2_E_DDSV& rhs) {}
};

//
class P1_SQ_DDSV : public DDSZ_util
{
public:
	P1_SQ_DDSV(AQLMathPathYieldCurve& curve_d_,
     		   AQLMathPathYieldCurve& curve_f_,
               vector<double> T_param_,
               vector<double> beta_,
               double rho_,            
               vector<double> theta_,
               vector<double> kappa_,
               vector<double> epsilon_,
               double S0_,
               double V0_
		      )
           :
		   DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_),
           vol1_SQ(new Vol1_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
           vol2_E_SQ(new Vol2_E_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)),
           rho_vol1_vol2_E(new Rho_vol1_vol2_E_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)) {}

	~P1_SQ_DDSV() { delete vol1_SQ; vol1_SQ = 0; delete vol2_E_SQ; vol2_E_SQ = 0; delete rho_vol1_vol2_E; rho_vol1_vol2_E = 0; }

	////double operator() (double t);
	double operator() (double t) const ;

private:
	P1_SQ_DDSV(const P1_SQ_DDSV& rhs) {}
	Vol1_SQ_DDSV* vol1_SQ;
    Vol2_E_SQ_DDSV* vol2_E_SQ;
    Rho_vol1_vol2_E_DDSV* rho_vol1_vol2_E;
};

//
class P1_Vol1_DDSV : public DDSZ_util
{
public:
	P1_Vol1_DDSV(AQLMathPathYieldCurve& curve_d_,
     			 AQLMathPathYieldCurve& curve_f_,
                 vector<double> T_param_,
                 vector<double> beta_,
                 double rho_,                   
                 vector<double> theta_,
                 vector<double> kappa_,
                 vector<double> epsilon_,
                 double S0_,
                 double V0_
		        )
				:
				DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_),
				vol1_SQ(new Vol1_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,   theta_, kappa_, epsilon_, S0_, V0_)),
				vol2_E_SQ(new Vol2_E_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,   theta_, kappa_, epsilon_, S0_, V0_)),
				rho_vol1_vol2_E(new Rho_vol1_vol2_E_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,   theta_, kappa_, epsilon_, S0_, V0_)) {}

	~P1_Vol1_DDSV() { delete vol1_SQ; vol1_SQ = 0; delete vol2_E_SQ; vol2_E_SQ = 0; delete rho_vol1_vol2_E; rho_vol1_vol2_E = 0; }

	////double operator() (double t);
	double operator() (double t) const;

private:
	P1_Vol1_DDSV(const P1_Vol1_DDSV& rhs) {}
	Vol1_SQ_DDSV* vol1_SQ;
    Vol2_E_SQ_DDSV* vol2_E_SQ;
    Rho_vol1_vol2_E_DDSV* rho_vol1_vol2_E;
};

//
class P1_P2_DDSV : public DDSZ_util
{
public:
	P1_P2_DDSV(AQLMathPathYieldCurve& curve_d_,
     		   AQLMathPathYieldCurve& curve_f_,
               vector<double> T_param_,
               vector<double> beta_,      
               double rho_,                 
               vector<double> theta_,
               vector<double> kappa_,
               vector<double> epsilon_,
               double S0_,
               double V0_
		      ) :
			  DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_),
			  vol1_SQ(new Vol1_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_)),
			  vol2_E_SQ(new Vol2_E_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)),
			  rho_vol1_vol2_E(new Rho_vol1_vol2_E_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)) {}

	~P1_P2_DDSV() { delete vol1_SQ; vol1_SQ = 0; delete vol2_E_SQ; vol2_E_SQ = 0; delete rho_vol1_vol2_E; rho_vol1_vol2_E = 0; }

	////double operator() (double t);
	double operator() (double t) const;

private:
	P1_P2_DDSV(const P1_P2_DDSV& rhs) {}
	Vol1_SQ_DDSV* vol1_SQ;
    Vol2_E_SQ_DDSV* vol2_E_SQ;
    Rho_vol1_vol2_E_DDSV* rho_vol1_vol2_E;
};

//
class P1_P3_DDSV : public DDSZ_util
{
public:
	P1_P3_DDSV(AQLMathPathYieldCurve& curve_d_,
     		   AQLMathPathYieldCurve& curve_f_,
               vector<double> T_param_,
               vector<double> beta_,      
               double rho_,               
               vector<double> theta_,
               vector<double> kappa_,
               vector<double> epsilon_,
               double S0_,
               double V0_
		      ) : 
			  DDSZ_util(curve_d_, curve_f_, T_param_, beta_,rho_, theta_, kappa_, epsilon_, S0_, V0_),
              vol1_SQ(new Vol1_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,   theta_, kappa_, epsilon_, S0_, V0_)),
              vol2_E_SQ(new Vol2_E_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,   theta_, kappa_, epsilon_, S0_, V0_)),
              rho_vol1_vol2_E(new Rho_vol1_vol2_E_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,   theta_, kappa_, epsilon_, S0_, V0_)) {}

	~P1_P3_DDSV() { delete vol1_SQ; vol1_SQ = 0; delete vol2_E_SQ; vol2_E_SQ = 0; delete rho_vol1_vol2_E; rho_vol1_vol2_E = 0; }

	////double operator() (double t);
	double operator() (double t) const;

private:
	P1_P3_DDSV(const P1_P3_DDSV& rhs) {}
	Vol1_SQ_DDSV* vol1_SQ;
    Vol2_E_SQ_DDSV* vol2_E_SQ;
    Rho_vol1_vol2_E_DDSV* rho_vol1_vol2_E;
};

//
class Rho_P1_P4_DDSV : public DDSZ_util
{
public:
	Rho_P1_P4_DDSV(AQLMathPathYieldCurve& curve_d_,
     			   AQLMathPathYieldCurve& curve_f_,
                   vector<double> T_param_,
                   vector<double> beta_,      
                   double rho_,                     
                   vector<double> theta_,
                   vector<double> kappa_,
                   vector<double> epsilon_,
                   double S0_,
                   double V0_
		          )
				  :
				  DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_),
				  vol1_SQ(new Vol1_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)),
				  vol2_E_SQ(new Vol2_E_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)),
				  rho_vol1_vol2_E(new Rho_vol1_vol2_E_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)) {}

	~Rho_P1_P4_DDSV() { delete vol1_SQ; vol1_SQ = 0; delete vol2_E_SQ; vol2_E_SQ = 0; delete rho_vol1_vol2_E; rho_vol1_vol2_E = 0; }

	////double operator() (double t);
	double operator() (double t) const;

private:
	Rho_P1_P4_DDSV(const Rho_P1_P4_DDSV& rhs) {}
	Vol1_SQ_DDSV* vol1_SQ;
    Vol2_E_SQ_DDSV* vol2_E_SQ;
    Rho_vol1_vol2_E_DDSV* rho_vol1_vol2_E;
};

//
class P1_P5_DDSV : public DDSZ_util
{
public:
	P1_P5_DDSV(AQLMathPathYieldCurve& curve_d_,
     		   AQLMathPathYieldCurve& curve_f_,
			   vector<double> T_param_,
			   vector<double> beta_,      
			   double rho_,                
			   vector<double> theta_,
			   vector<double> kappa_,
			   vector<double> epsilon_,
			   double S0_,
			   double V0_
		      )
			  :
			  DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_),
              vol1_SQ(new Vol1_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,   theta_, kappa_, epsilon_, S0_, V0_)),
              vol2_E_SQ(new Vol2_E_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)),
              rho_vol1_vol2_E(new Rho_vol1_vol2_E_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,   theta_, kappa_, epsilon_, S0_, V0_)) {}

	~P1_P5_DDSV() { delete vol1_SQ; vol1_SQ = 0; delete vol2_E_SQ; vol2_E_SQ = 0; delete rho_vol1_vol2_E; rho_vol1_vol2_E = 0; }

	////double operator() (double t);
	double operator() (double t) const;

private:
	P1_P5_DDSV(const P1_P5_DDSV& rhs) {}
	Vol1_SQ_DDSV* vol1_SQ;
    Vol2_E_SQ_DDSV* vol2_E_SQ;
    Rho_vol1_vol2_E_DDSV* rho_vol1_vol2_E;
};

//
class P1_P6_DDSV : public DDSZ_util
{
public:
	P1_P6_DDSV(AQLMathPathYieldCurve& curve_d_,
     		   AQLMathPathYieldCurve& curve_f_,
               vector<double> T_param_,
               vector<double> beta_,
               double rho_,                 
               vector<double> theta_,
               vector<double> kappa_,
               vector<double> epsilon_,
               double S0_,
               double V0_
		      )
			  :
			  DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_),
              vol1_SQ(new Vol1_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,   theta_, kappa_, epsilon_, S0_, V0_)),
              vol2_E_SQ(new Vol2_E_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)),
              rho_vol1_vol2_E(new Rho_vol1_vol2_E_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)) {}

	~P1_P6_DDSV() { delete vol1_SQ; vol1_SQ = 0; delete vol2_E_SQ; vol2_E_SQ = 0; delete rho_vol1_vol2_E; rho_vol1_vol2_E = 0; }

	////double operator() (double t);
	double operator() (double t) const;

private:
	P1_P6_DDSV(const P1_P6_DDSV& rhs) {}
	Vol1_SQ_DDSV* vol1_SQ;
    Vol2_E_SQ_DDSV* vol2_E_SQ;
    Rho_vol1_vol2_E_DDSV* rho_vol1_vol2_E;
};

//
class Rho_P1_P7_DDSV : public DDSZ_util
{
public:
	Rho_P1_P7_DDSV(AQLMathPathYieldCurve& curve_d_,
     	 		   AQLMathPathYieldCurve& curve_f_,
                   vector<double> T_param_,
                   vector<double> beta_,
                   double rho_,                    
                   vector<double> theta_,
                   vector<double> kappa_,
                   vector<double> epsilon_,
                   double S0_,
                   double V0_
		          )
				  :
				  DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_),
                  vol1_SQ(new Vol1_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)),
                  vol2_E_SQ(new Vol2_E_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)),
                  rho_vol1_vol2_E(new Rho_vol1_vol2_E_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)) {}

	~Rho_P1_P7_DDSV() { delete vol1_SQ; vol1_SQ = 0; delete vol2_E_SQ; vol2_E_SQ = 0; delete rho_vol1_vol2_E; rho_vol1_vol2_E = 0; }

	////double operator() (double t);
	double operator() (double t) const;

private:
	Rho_P1_P7_DDSV(const Rho_P1_P7_DDSV& rhs) {}
	Vol1_SQ_DDSV* vol1_SQ;
    Vol2_E_SQ_DDSV* vol2_E_SQ;
    Rho_vol1_vol2_E_DDSV* rho_vol1_vol2_E;
};

//
class P1_P8_DDSV : public DDSZ_util
{
public:
	P1_P8_DDSV(AQLMathPathYieldCurve& curve_d_,
     		   AQLMathPathYieldCurve& curve_f_,
			   vector<double> T_param_,
			   vector<double> beta_,
			   double rho_,                
			   vector<double> theta_,
			   vector<double> kappa_,
			   vector<double> epsilon_,
			   double S0_,
			   double V0_
		      )
			  :
			  DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_),
              vol1_SQ(new Vol1_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_,   theta_, kappa_, epsilon_, S0_, V0_)),
              vol2_E_SQ(new Vol2_E_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)),
              rho_vol1_vol2_E(new Rho_vol1_vol2_E_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)) {}

	~P1_P8_DDSV() { delete vol1_SQ; vol1_SQ = 0; delete vol2_E_SQ; vol2_E_SQ = 0; delete rho_vol1_vol2_E; rho_vol1_vol2_E = 0; }

	////double operator() (double t);
	double operator() (double t) const;

private:
	P1_P8_DDSV(const P1_P8_DDSV& rhs) {}
	Vol1_SQ_DDSV* vol1_SQ;
    Vol2_E_SQ_DDSV* vol2_E_SQ;
    Rho_vol1_vol2_E_DDSV* rho_vol1_vol2_E;
};

//
class P1_P9_DDSV : public DDSZ_util
{
public:
	P1_P9_DDSV(AQLMathPathYieldCurve& curve_d_,
     		   AQLMathPathYieldCurve& curve_f_,
               vector<double> T_param_,
               vector<double> beta_, 
               double rho_,                 
               vector<double> theta_,
               vector<double> kappa_,
               vector<double> epsilon_,
               double S0_,
               double V0_
		      )
			  : DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_),
                vol1_SQ(new Vol1_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)),
                vol2_E_SQ(new Vol2_E_SQ_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)),
                rho_vol1_vol2_E(new Rho_vol1_vol2_E_DDSV(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_)) {}

	~P1_P9_DDSV() { delete vol1_SQ; vol1_SQ = 0; delete vol2_E_SQ; vol2_E_SQ = 0; delete rho_vol1_vol2_E; rho_vol1_vol2_E = 0; }

	////double operator() (double t);
	double operator() (double t) const;

private:
	P1_P9_DDSV(const P1_P9_DDSV& rhs) {}
	Vol1_SQ_DDSV* vol1_SQ;
    Vol2_E_SQ_DDSV* vol2_E_SQ;
    Rho_vol1_vol2_E_DDSV* rho_vol1_vol2_E;
};

//
class P2_SQ_DDSV : public DDSZ_util
{
public:
	P2_SQ_DDSV(AQLMathPathYieldCurve& curve_d_,
     		   AQLMathPathYieldCurve& curve_f_,
               vector<double> T_param_,
               vector<double> beta_,      
               double rho_,                 
               vector<double> theta_,
               vector<double> kappa_,
               vector<double> epsilon_,
               double S0_,
               double V0_
		      ) : DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_) {}

	////double operator() (double t);
	double operator() (double t) const;

private:
	P2_SQ_DDSV(const P2_SQ_DDSV& rhs) {}
};

//
class P4_SQ_DDSV : public DDSZ_util
{
public:
	P4_SQ_DDSV(AQLMathPathYieldCurve& curve_d_,
     		   AQLMathPathYieldCurve& curve_f_,
               vector<double> T_param_,
               vector<double> beta_,      
               double rho_,
               vector<double> theta_,
               vector<double> kappa_,
               vector<double> epsilon_,
               double S0_,
               double V0_
		      ) : DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_) {}

	////double operator() (double t);
	double operator() (double t) const;

private:
	P4_SQ_DDSV(const P4_SQ_DDSV& rhs) {}
};

//
class P6_SQ_DDSV : public DDSZ_util
{
public:
	P6_SQ_DDSV(AQLMathPathYieldCurve& curve_d_,
     		   AQLMathPathYieldCurve& curve_f_,
               vector<double> T_param_,
               vector<double> beta_,      
               double rho_,              
               vector<double> theta_,
               vector<double> kappa_,
               vector<double> epsilon_,
               double S0_,
               double V0_
		      ) : DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_) {}

	////double operator() (double t);
	double operator() (double t) const;

private:
	P6_SQ_DDSV(const P6_SQ_DDSV& rhs) {}
};

//
class Rho_P4_P6_DDSV : public DDSZ_util
{
public:
	Rho_P4_P6_DDSV(AQLMathPathYieldCurve& curve_d_,
     	  		   AQLMathPathYieldCurve& curve_f_,
                   vector<double> T_param_,
                   vector<double> beta_,      
                   double rho_,                    
                   vector<double> theta_,
                   vector<double> kappa_,
                   vector<double> epsilon_,
                   double S0_,
                   double V0_
		          ) : DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_) {}

	////double operator() (double t);
	double operator() (double t) const;

private:
	Rho_P4_P6_DDSV(const Rho_P4_P6_DDSV& rhs) {}
};

//
class P2_Vol1_DDSV : public DDSZ_util
{
public:
	P2_Vol1_DDSV(AQLMathPathYieldCurve& curve_d_,
     			 AQLMathPathYieldCurve& curve_f_,
                 vector<double> T_param_,
                 vector<double> beta_,      
                 double rho_,                   
                 vector<double> theta_,
                 vector<double> kappa_,
                 vector<double> epsilon_,
                 double S0_,
                 double V0_
		        ) : DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_) {}

	~P2_Vol1_DDSV() {}

	////double operator() (double t);
	double operator() (double t) const;

private:
	P2_Vol1_DDSV(const P1_P2_DDSV& rhs) {}
};

//
//
class Rho_Vol1_P4_DDSV : public DDSZ_util
{
public:
	Rho_Vol1_P4_DDSV(AQLMathPathYieldCurve& curve_d_,
     			     AQLMathPathYieldCurve& curve_f_,
                     vector<double> T_param_,
                     vector<double> beta_,      
                     double rho_,                     
                     vector<double> theta_,
                     vector<double> kappa_,
                     vector<double> epsilon_,
                     double S0_,
                     double V0_
                    ) : DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_,  theta_, kappa_, epsilon_, S0_, V0_) {}

	~Rho_Vol1_P4_DDSV() {}

	////double operator() (double t);
	double operator() (double t) const;

private:
	Rho_Vol1_P4_DDSV(const Rho_Vol1_P4_DDSV& rhs) {}
};

//
class P6_Vol1_DDSV : public DDSZ_util
{
public:
	P6_Vol1_DDSV(AQLMathPathYieldCurve& curve_d_,
     			 AQLMathPathYieldCurve& curve_f_,
                 vector<double> T_param_,
                 vector<double> beta_,
                 double rho_,                 
                 vector<double> theta_,
                 vector<double> kappa_,
                 vector<double> epsilon_,
                 double S0_,
                 double V0_
		        ) : DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_) {}

	~P6_Vol1_DDSV() {}

	////double operator() (double t);
	double operator() (double t) const;

private:
	P6_Vol1_DDSV(const P6_Vol1_DDSV& rhs) {}
};

//
class Rho_P2_P4_DDSV : public DDSZ_util
{
public:
	Rho_P2_P4_DDSV(AQLMathPathYieldCurve& curve_d_,
     			   AQLMathPathYieldCurve& curve_f_,
                   vector<double> T_param_,
                   vector<double> beta_,      
                   double rho_,                 
                   vector<double> theta_,
                   vector<double> kappa_,
                   vector<double> epsilon_,
                   double S0_,
                   double V0_
		          ) : DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_) {}

	~Rho_P2_P4_DDSV() {}

	////double operator() (double t);
	double operator() (double t) const;

private:
	Rho_P2_P4_DDSV(const Rho_P2_P4_DDSV& rhs) {}
};

//
class P2_P6_DDSV : public DDSZ_util
{
public:
	P2_P6_DDSV(AQLMathPathYieldCurve& curve_d_,
     		   AQLMathPathYieldCurve& curve_f_,
               vector<double> T_param_,
               vector<double> beta_,
               double rho_,                 
               vector<double> theta_,
               vector<double> kappa_,
               vector<double> epsilon_,
               double S0_,
               double V0_
		      ) : DDSZ_util(curve_d_, curve_f_, T_param_, beta_, rho_, theta_, kappa_, epsilon_, S0_, V0_) {}

	~P2_P6_DDSV() {}

	////double operator() (double t);
	double operator() (double t) const;

private:
	P2_P6_DDSV(const P2_P6_DDSV& rhs) {}
};

//
class AQLMathSwaptionFormulaSZ
{
public:
	AQLMathSwaptionFormulaSZ(AQLMathPathYieldCurve& curve_d_,
                 		AQLMathPathYieldCurve& curve_f_,
                        vector<double> T_param_,
                        vector<double> beta_,      
                        double rho_,                          
                        vector<double> theta_,
                        vector<double> kappa_,
                        vector<double> epsilon_,
                        double S0_,
                        double V0_
		               );
	
	~AQLMathSwaptionFormulaSZ()
	{
        delete vol1_SQ; vol1_SQ = 0;
        delete vol2_E_SQ; vol2_E_SQ = 0;
        delete rho_vol1_vol2_E; rho_vol1_vol2_E = 0;
        delete p1_SQ; p1_SQ = 0;
        delete p1_vol1; p1_vol1 = 0;
        delete p1_p2; p1_p2 = 0;
        delete p1_p3; p1_p3 = 0;
        delete rho_p1_p4; rho_p1_p4 = 0;
        delete p1_p5; p1_p5 = 0;
        delete p1_p6; p1_p6 = 0;
        delete rho_p1_p7; rho_p1_p7 = 0;
        delete p1_p8; p1_p8 = 0;
        delete p1_p9; p1_p9 = 0;
        delete p2_SQ; p2_SQ = 0;
        delete p4_SQ; p4_SQ = 0;
        delete p6_SQ; p6_SQ = 0;
        delete rho_p4_p6; rho_p4_p6 = 0;
        delete p2_vol1; p2_vol1 = 0;
		delete rho_vol1_p4; rho_vol1_p4 = 0;

        //
        delete p6_vol1; p6_vol1 = 0;
        delete rho_p2_p4; rho_p2_p4 = 0;
        delete p2_p6; p2_p6 = 0;

        delete int_multi; int_multi = 0;
	}

	void Get_param(vector<double>& T_param,
				   vector<double>& beta,
	 			   vector<double>& theta,
			       vector<double>& kappa,
			       vector<double>& epsilon
			      )
	{
		vol1_SQ->Get_param(T_param, beta, theta, kappa, epsilon);
	}

	size_t get_num_param() { return vol1_SQ->get_num_param(); }

	void Set_param(const vector<double>& T_param,
		           const vector<double>& beta,
		 		   const vector<double>& theta,
				   const vector<double>& kappa,
				   const vector<double>& epsilon
				  )
	{
		clear_cache();

		vol1_SQ->Set_param(T_param, beta, theta, kappa, epsilon);

		map<double, double> cathe_Int_E_theta;
		vol1_SQ->Get_cathe_Int_E_theta(cathe_Int_E_theta);

        vol2_E_SQ->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        rho_vol1_vol2_E->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        p1_SQ->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        p1_vol1->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        p1_p2->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        p1_p3->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        rho_p1_p4->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        p1_p5->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        p1_p6->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);

        //rho_p1_p7->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        p1_p8->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        //p1_p9->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        p2_SQ->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        p4_SQ->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        p6_SQ->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        rho_p4_p6->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        p2_vol1->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);

        //
        rho_p4_p6->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        p6_vol1->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        rho_p2_p4->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
        p2_p6->Set_param(T_param, beta, theta, kappa, epsilon, cathe_Int_E_theta);
	}

	virtual double Get_PDF(double t, double y)
	{ 
		//
		//int_multi->Set_func_integral_1D_0(p1_SQ, 8);
		//double Sigma = int_multi->integral_1D_0(t);

		//double P_d = curve_d->GetP(t);
		//double P_f = curve_f->GetP(t);

		//double fwd = S0 * P_f / P_d;

		//double x = y / fwd - 1.;
		//double Sigma_SQ = Sigma * Sigma;
		//double x_SQ = x * x;

		////
		//double PI = 3.14159265358979323846;
		//double n = exp( - x_SQ / 2. / Sigma ) / sqrt(2. * PI * Sigma);

		////
		//double tmp1 =  2. * pow( 6.);

		//double tmp2 = q3(t) * (pow(x,6.) - 15. * pow(x,4.) * Sigma + 45. * x_SQ * Sigma_SQ - 15. * Sigma_SQ * Sigma);
		//tmp2 += Sigma_SQ * (2. * q2(t) + q4(t)) * (pow(x, 4.) - 6. * x_SQ * Sigma + 3. * Sigma_SQ);
		//tmp2 += Sigma * Sigma_SQ * (2. * q1(t) * x_SQ * x - 6. * q1(t) * x * Sigma + q5(t) * x_SQ * Sigma - q5(t) * Sigma_SQ + 2. * Sigma_SQ * Sigma);
		//
		//return n * tmp2 / tmp1 / fwd;


		return 1.;
	}

	virtual double Get_Call_Opt(double t, double K, bool cache = false)
	{ 
		//
		if(!cache) clear_cache();
		if(cathe_q1[t] == 0 )
		{
			map<double, double>::iterator ITR = max_element(cathe_q2.begin(), cathe_q2.end());
			double t_prev = ITR->first;

			cathe_q1[t] = cathe_q1[t_prev] + q1(t_prev,t);
			cathe_q2[t] = cathe_q2[t_prev] + q2(t_prev,t);
			cathe_q3[t] = cathe_q3[t_prev] + q3(t_prev,t);
			//cathe_q4[t] = q4(t);
			cathe_q4[t] = cathe_q1[t] * cathe_q1[t];
			cathe_q5[t] = cathe_q5[t_prev] + q5(t_prev,t);
			cathe_q6[t] = cathe_q6[t_prev] + q6(t_prev,t);
			//cathe_q7[t] = q7(t);
			cathe_q7[t] = cathe_q2[t] * cathe_q2[t];
			cathe_q8[t] = cathe_q8[t_prev] + q8(t_prev,t);
			cathe_q9[t] = cathe_q9[t_prev] + q9(t_prev,t);
			//cathe_q10[t] = q10(t);
			cathe_q10[t] = cathe_q1[t] * cathe_q2[t];
			cathe_q11[t] = cathe_q11[t_prev] + q11(t_prev,t);
			cathe_q12[t] = cathe_q12[t_prev] + q12(t_prev,t);

			int_multi->Set_func_integral_1D_0(p1_SQ, 10);
			cathe_Sigma[t] = cathe_Sigma[t_prev] + int_multi->integral_1D_0(t_prev, t);
		}

		double P_d = curve_d->getP(t);
		double P_f = curve_f->getP(t);

		double fwd = S0 * P_f / P_d;
		double K_ = 1. - K / fwd;
		double K_SQ = K_ * K_;

		double Sigma = cathe_Sigma[t];
		double Sigma_SQ = Sigma * Sigma;

		//
		double PI = 3.14159265358979323846;
		double n = exp( - K_SQ / 2. / Sigma ) / sqrt(2. * PI * Sigma);
		//double n_ = exp( K_SQ / 2. / Sigma ) > 10000000000000000. ? 10000000000000000. : exp( K_SQ / 2. / Sigma );

		//
		double tmp1 =  2. * sqrt(2.) * pow(Sigma, 4.);

	    //furuya
		//ProbDist_normal stdNorm;

		double tmp2 = sqrt(2.) * ( cathe_q4[t] + cathe_q7[t] + 2. * cathe_q10[t] ) * ( pow(K_,4.) - 6. * pow(K_,2.) * Sigma + 3. * Sigma_SQ );

        //double tmp2 = sqrt(2.) * ( cathe_q4(t) + cathe_q7(t) ) * ( pow(K_,4.) - 6. * pow(K_,2.) * Sigma + 3. * Sigma_SQ );
		tmp2 += Sigma_SQ * sqrt(2.) * (cathe_q8[t] + cathe_q5[t] + 2. * cathe_q11[t] + 2. * cathe_q3[t]) * (K_SQ - Sigma);
		//tmp2 += Sigma_SQ * sqrt(2.) * (cathe_q8(t) + cathe_q5(t) + 2. * cathe_q3(t)) * (K_SQ - Sigma);
        tmp2 += Sigma * Sigma_SQ * (-2. * sqrt(2.) * (cathe_q1[t] + cathe_q2[t]) * K_ + sqrt(2.) * (cathe_q6[t] + cathe_q9[t] + 2. * cathe_q12[t]) * Sigma + 2. * sqrt(2.) * Sigma_SQ);
        //tmp2 += Sigma * Sigma_SQ * (-2. * sqrt(2.) * (cathe_q1(t) + cathe_q2(t)) * K_ + sqrt(2.) * (cathe_q6(t) + cathe_q9(t)) * Sigma + 2. * sqrt(2.) * Sigma_SQ);
		//furuya
		//return S0 * P_f * (  n * tmp2 / tmp1 + K_ * ( 1. - stdNorm.cdf(-K_ / sqrt(Sigma)) ));
		return S0 * P_f * (  n * tmp2 / tmp1 + K_ * ( 1. - AQLDist::normsdist(-K_ / sqrt(Sigma)) ));
	}

	double Get_imp_vol(double t, double K, bool cache, int sgn = 1)
	{
		double P_d = curve_d->getP(t);
		double P_f = curve_f->getP(t);
	    
		double prem = Get_Call_Opt(t, K, cache);
		prem = prem > 0 ? prem : 0.000000000000001; 
		imp_vol_ddsz imp_vol_ddsz_(prem / P_d, S0 * P_f / P_d, K, sgn);

		return imp_vol_ddsz_.SolveBR( -100.0, 100.0, 100000, 0.000000000000001 ) / sqrt(t);
	}

private:
	AQLMathSwaptionFormulaSZ(const AQLMathSwaptionFormulaSZ& rhs) {}

	double S0;
	AQLMathPathYieldCurve* curve_d;
	AQLMathPathYieldCurve* curve_f;

	double q1(double t);
	double q2(double t);
	double q3(double t);
	double q4(double t);
	double q5(double t);
	double q6(double t);
	double q7(double t);
	double q8(double t);
	double q9(double t);
	double q10(double t);
	double q11(double t);
	double q12(double t);

	double q1(double ts, double te);
	double q2(double ts, double te);
	double q3(double ts, double te);
	double q4(double ts, double te);
	double q5(double ts, double te);
	double q6(double ts, double te);
	double q7(double ts, double te);
	double q8(double ts, double te);
	double q9(double ts, double te);
	double q10(double ts, double te);
	double q11(double ts, double te);
	double q12(double ts, double te);

	map<double, double> cathe_Sigma;
	map<double, double> cathe_q1;
	map<double, double> cathe_q2;
	map<double, double> cathe_q3;
	map<double, double> cathe_q4;
	map<double, double> cathe_q5;
	map<double, double> cathe_q6;
	map<double, double> cathe_q7;
	map<double, double> cathe_q8;
	map<double, double> cathe_q9;
	map<double, double> cathe_q10;
	map<double, double> cathe_q11;
	map<double, double> cathe_q12;

	void clear_cache();


	Vol1_SQ_DDSV*    vol1_SQ;
    Vol2_E_SQ_DDSV*  vol2_E_SQ;
    Rho_vol1_vol2_E_DDSV* rho_vol1_vol2_E;
    P1_SQ_DDSV* p1_SQ;
    P1_Vol1_DDSV* p1_vol1;
    P1_P2_DDSV* p1_p2;
    P1_P3_DDSV* p1_p3;
    Rho_P1_P4_DDSV* rho_p1_p4;
    P1_P5_DDSV* p1_p5;
    P1_P6_DDSV* p1_p6;
    Rho_P1_P7_DDSV* rho_p1_p7;
    P1_P8_DDSV* p1_p8;
    P1_P9_DDSV* p1_p9;
    P2_SQ_DDSV* p2_SQ;
    P4_SQ_DDSV* p4_SQ;
    P6_SQ_DDSV* p6_SQ;
    Rho_P4_P6_DDSV* rho_p4_p6;
    P2_Vol1_DDSV* p2_vol1;	

    //
    Rho_Vol1_P4_DDSV* rho_vol1_p4;
    P6_Vol1_DDSV* p6_vol1;
    Rho_P2_P4_DDSV* rho_p2_p4;
    P2_P6_DDSV* p2_p6;

    AQLMathIntegralMulti* int_multi;

	class imp_vol_ddsz : public AQLFunction
	{
	public:
		imp_vol_ddsz( double prem_, double mean_LN_, double margin_, int sgn_ )
			: prem(prem_), mean_LN(mean_LN_), margin(margin_), sgn(sgn_) {}
		////double operator()(double x) { return AQLMathAnalyticalFormula::BlackFormula( mean_LN, x, margin, sgn) / prem - 1.; }
		double operator()(double x) const { return AQLMathAnalyticalFormula::BlackFormula( mean_LN, x, margin, sgn) / prem - 1.; }
	private:
		double prem;
		double mean_LN;
		double margin;
		int sgn;
	};
};

#endif