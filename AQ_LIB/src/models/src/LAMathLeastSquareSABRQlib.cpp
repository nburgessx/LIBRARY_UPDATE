#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <LACoreTemplateType.h>
#include <LAString.h>
#include "LABasic.h"
#include "LADist.h"

#include "LAMathBaseFuncUtility.h"
#include "LAMathInterpolationUtilities.h"
#include "LACoreComponentManager.h"
#include "LAAnalyticFormula.h"
#include "LAMathIRVanillaFuncUtility.h"
#include "LAMathLeastSquareSABRQlib.h"
#include "LAMathSwaptionVolUtility.h"
#include "LAMathSwaptionSABR.h"
#include "LAMathDateCalculations.h"
#include <math.h>

#include <boost/math/special_functions/asinh.hpp>
#include <boost/math/special_functions/atanh.hpp>

//
//----------------------------------------------------------------
//

//  Cost Func for optimizing SABR Parameters
LAMathLeastSquareSABRCostFuncQlib::LAMathLeastSquareSABRCostFuncQlib( LAMathSABR& sabr_,
														  double expiry_,
														  double forward_,
														  double numeraire_,
														  double atmVol_,
														  const vector<double>& strike_,
														  const vector<double>& weight_,
														  const vector<int>& sgn_,
														  const vector<double>& param0_,
														  const vector<bool>& flg_,
														  const vector<double>& target2fit_,
														  const LAString& target_
														)
:
CostFunction(),
n(strike_.size()),
expiry(expiry_),
forward(forward_),
numeraire(numeraire_),
atmVol(atmVol_),
strike(strike_),
weight(weight_),	
sgn(sgn_),//Call-Put Sign (-1: put, 1: call)
param0(param0_),//Initial alpha, beta, nu, rho
flg(flg_),
target2fit(target2fit_),
sabr(&sabr_),
target(target_),
cloned(false)
{
	if(weight.size() != n)
	{
		throw LACoreInvalidData(" strike.size() != weight.size() : LAMathLeastSquareSABRCostFuncQlib::LAMathLeastSquareSABRCostFuncQlib", __FILE__, __LINE__);
	}
	
	if(param0.size() != 4)
	{
		throw LACoreInvalidData("param0.size() != 4 : LAMathLeastSquareSABRCostFuncQlib::LAMathLeastSquareSABRCostFuncQlib", __FILE__, __LINE__);
	}

	if(flg.size() != 4)
	{
		throw LACoreInvalidData("flg.size() != 4 : LAMathLeastSquareSABRCostFuncQlib::LAMathLeastSquareSABRCostFuncQlib", __FILE__, __LINE__);
	}

	for(size_t i = 0; i < n; ++i)
	{
		if(sgn[i] != 1 && sgn[i] != -1)
		{
			throw LACoreInvalidData("sgn must -1 or 1 : LAMathLeastSquareSABRCostFuncQlib::LAMathLeastSquareSABRCostFuncQlib", __FILE__, __LINE__);
		}
	}

	alpha = param0[0];
	beta = param0[1];
	nu = param0[2];
	rho = param0[3];
}


//
LAMathLeastSquareSABRCostFuncQlib::LAMathLeastSquareSABRCostFuncQlib(const LAMathLeastSquareSABRCostFuncQlib& rhs)
:
CostFunction(rhs),
n(rhs.n),
expiry(rhs.expiry),
forward(rhs.forward),
numeraire(rhs.numeraire),
atmVol(rhs.atmVol),
strike(rhs.strike),
weight(rhs.weight),
sgn(rhs.sgn),
param0(rhs.param0),
flg(rhs.flg),
target2fit(rhs.target2fit),
sabr(rhs.sabr != 0 ? rhs.sabr->clone() : 0),
alpha(rhs.alpha),
beta(rhs.beta),
nu(rhs.nu),
rho(rhs.rho),
cloned(true)
{}


LAMathLeastSquareSABRCostFuncQlib::~LAMathLeastSquareSABRCostFuncQlib()
{
	if(cloned)
	{
		delete sabr;
		sabr = 0;
	}
}

QuantLib::Real LAMathLeastSquareSABRCostFuncQlib::value(const QuantLib::Array& x) const
{   
	size_t m = x.size();
	
	if(m == 0 || m > 4)
		throw LACoreInvalidData("m == 0 || m > 4 : LAMathLeastSquareSABRCostFuncQlib::value", __FILE__, __LINE__);

	set_params(x);
	sabr->setSABRParam(alpha,beta,nu,rho); 

	//If atmVol is set, derived Alpha from atm vol
	if (!boost::math::isnan(atmVol))
	{
		sabr->setAlphaForATMVol(atmVol, expiry, forward);
		alpha = sabr->getAlpha();
	}

	QuantLib::Real tmp = 0.;
	double tmp2 = 0.;
	for(size_t i = 0; i < n; ++i)
	{
		if( weight[i] > 0.000000001 )
		{
			if( target == CALIB_TARGET_PREMIUM )
			{
				int sgn_ = sgn[i];
				tmp2 = sqrt( weight[i] ) * ( target2fit[i] - sabr->getSABRPrem(expiry,forward,strike[i],numeraire,sgn_) );
			}
			else if( target == CALIB_TARGET_VOLATILITY )
			{
				tmp2 = sqrt( weight[i] ) * ( target2fit[i] - sabr->getSABRVol(expiry,forward,strike[i]) );
			}
		
			tmp += tmp2 * tmp2;
		}
	}

	return tmp;
}

QuantLib::Array LAMathLeastSquareSABRCostFuncQlib::values(const QuantLib::Array& x) const
{
	size_t m = x.size();
	
	if(m == 0 || m > 4)
		throw LACoreInvalidData("m == 0 || m > 4 : LAMathLeastSquareSABRCostFuncQlib::value", __FILE__, __LINE__);

	set_params(x);    
	sabr->setSABRParam(alpha,beta,nu,rho); 

	//If atmVol is set, derived Alpha from atm vol
	if (!boost::math::isnan(atmVol))
	{
		sabr->setAlphaForATMVol(atmVol, expiry, forward);
		alpha = sabr->getAlpha();
	}

    size_t k = 0;
	QuantLib::Array diff_SQs( n );
	
	for(size_t i = 0; i < n; ++i)
	{
		if( weight[i] > 0.000000001 )
		{
			if( target == CALIB_TARGET_PREMIUM )
			{
				int sgn_ = sgn[i];
				diff_SQs[i] = sqrt( weight[i] ) * (  sabr->getSABRPrem(expiry,forward,strike[i],numeraire,sgn_)- target2fit[i] ); 
			}
			else if( target == CALIB_TARGET_VOLATILITY )
			{
				diff_SQs[i] = sqrt( weight[i] ) * ( sabr->getSABRVol(expiry,forward,strike[i]) - target2fit[i] ); 
			}
		}
	}

	return diff_SQs;
}

void LAMathLeastSquareSABRCostFuncQlib::set_params(const QuantLib::Array& x) const
{
	// 1st
	if(!flg[0])
	{
		if(!flg[1])
		{
			if(!flg[2])
			{
				if(!flg[3])
				{
					throw LACoreInvalidData("All element of flg is false : LAMathLeastSquareSABRCostFuncQlib::set_params", __FILE__, __LINE__);
				}
				else rho = x[0];
			}
			else
			{
				nu = x[0];
				if(flg[3]) rho = x[1];
			}
		}
		else
		{
			beta = x[0];
			if(flg[2])
			{
				nu = x[1];
				if(flg[3]) rho = x[2];
			}
			else
			{
				if(flg[3]) rho = x[1];
			}
		}
	}
	else
	{
		alpha = x[0];
		if(!flg[1])
		{
			if(flg[2])
			{
				nu = x[1];
				if(flg[3]) rho = x[2];
			}
			else
			{
				if(flg[3]) rho = x[1];
			}
		}
		else
		{
			beta = x[1];
			if(flg[2])
			{
				nu = x[2];
				if(flg[3]) rho = x[3];
			}
			else
			{
				if(flg[3]) rho = x[2];
			}
		}
	}
}

//
//----------------------------------------------------------------
//

//LAMathLeastSquareSABRConstraintQlib::LAMathLeastSquareSABRConstraintQlib(vector<double>& param0,
//															 double max_alpha,
//								                             double min_alpha,
//								                             double max_beta,
//								                             double min_beta,
//								                             double max_nu,
//								                             double min_nu,
//								                             double max_rho,
//								                             double min_rho
//								                            )
//: QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>( new LAMathLeastSquareSABRConstraintQlib::Impl(param0,
//																											  max_alpha,
//																											  min_alpha,
//																											  max_beta,
//																											  min_beta,
//																											  max_nu,
//																											  min_nu,
//																											  max_rho,
//																											  min_rho
//																											 )) )
//{
//}

//
LAMathLeastSquareSABRConstraintQlib::Impl::Impl(vector<double>& param0_,
										  const vector<bool>& flg_,
										  double max_alpha_,
							              double min_alpha_,
							              double max_beta_,
							              double min_beta_,
							              double max_nu_,
							              double min_nu_,
							              double max_rho_,
							              double min_rho_
							             )
:
param0(param0_),
flg(flg_),
max_alpha(max_alpha_),
min_alpha(min_alpha_),
max_beta(max_beta_),
min_beta(min_beta_),
max_nu(max_nu_),
min_nu(min_nu_),
max_rho(max_rho_),
min_rho(min_rho_),
alpha(param0_[0]),
beta(param0_[1]),
nu(param0_[2]),
rho(param0_[3])
{	
}
//
//
bool LAMathLeastSquareSABRConstraintQlib::Impl::test(const QuantLib::Array& p) const
{
 	size_t m = p.size();
	
	set_params(p);

	if(alpha < min_alpha || max_alpha < alpha ) return false;
	if(beta < min_beta || max_beta < beta ) return false;
	if(nu < min_nu || max_nu < nu ) return false;
	if(rho < min_rho || max_rho < rho ) return false;

	return true;
}

void LAMathLeastSquareSABRConstraintQlib::Impl::set_params(const QuantLib::Array& x) const
{
	// 1st
	if(!flg[0])
	{
		if(!flg[1])
		{
			if(!flg[2])
			{
				if(!flg[3])
				{
					throw LACoreInvalidData("All element of flg is false : LAMathLeastSquareSABRCostFuncQlib::set_params", __FILE__, __LINE__);
				}
				else rho = x[0];
			}
			else
			{
				nu = x[0];
				if(flg[3]) rho = x[1];
			}
		}
		else
		{
			beta = x[0];
			if(flg[2])
			{
				nu = x[1];
				if(flg[3]) rho = x[2];
			}
			else
			{
				if(flg[3]) rho = x[1];
			}
		}
	}
	else
	{
		alpha = x[0];
		if(!flg[1])
		{
			if(flg[2])
			{
				nu = x[1];
				if(flg[3]) rho = x[2];
			}
			else
			{
				if(flg[3]) rho = x[1];
			}
		}
		else
		{
			beta = x[1];
			if(flg[2])
			{
				nu = x[2];
				if(flg[3]) rho = x[3];
			}
			else
			{
				if(flg[3]) rho = x[2];
			}
		}
	}
}

//
//----------------------------------------------------------------
//

/*LAMathLeastSquareSABRConstraint_AntonovQlib::LAMathLeastSquareSABRConstraint_AntonovQlib( double fwd,
																			  vector<double>& param0,
																			  double max_alpha,
																			  double min_alpha,
																			  double max_beta,
																			  double min_beta,
																			  double max_nu,
																			  double min_nu,
																			  double max_rho,
																			  double min_rho
																			)
:
Constraint(boost::shared_ptr<Constraint::Impl>( new LAMathLeastSquareSABRConstraint_AntonovQlib::Impl(fwd,
																								param0,
																								max_alpha,
																								min_alpha,
																								max_beta,
																								min_beta,
																								max_nu,
																								min_nu,
																								max_rho,
																								min_rho
																								)) )
{
}*/

//
LAMathLeastSquareSABRConstraint_AntonovQlib::Impl::Impl( double fwd_,
												   vector<double>& param0_,
												   const vector<bool>& flg_,
												   double max_alpha_,
												   double min_alpha_,
												   double max_beta_,
												   double min_beta_,
												   double max_nu_,
												   double min_nu_,
												   double max_rho_,
												   double min_rho_
												  )
:
fwd(fwd_),
param0(param0_),
flg(flg_),
max_alpha(max_alpha_),
min_alpha(min_alpha_),
max_beta(max_beta_),
min_beta(min_beta_),
max_nu(max_nu_),
min_nu(min_nu_),
max_rho(max_rho_),
min_rho(min_rho_),
alpha(param0_[0]),
beta(param0_[1]),
nu(param0_[2]),
rho(param0_[3])
{	
}

//
bool LAMathLeastSquareSABRConstraint_AntonovQlib::Impl::test(const QuantLib::Array& p) const
{
 	size_t m = p.size();
	
	set_params(p);

	if(alpha < min_alpha || max_alpha < alpha ) return false;
	if(beta < min_beta || max_beta < beta ) return false;
	if(nu < min_nu || max_nu < nu ) return false;
	if(rho < min_rho || max_rho < rho ) return false;

	if(nu * nu - 1.5 * (nu * nu * rho * rho + alpha * nu * rho * (1. - beta) * pow(fwd, beta - 1.) ) < 0. )  return false;
	
	return true;
}

void LAMathLeastSquareSABRConstraint_AntonovQlib::Impl::set_params(const QuantLib::Array& x) const
{
	if(!flg[0])
	{
		if(!flg[1])
		{
			if(!flg[2])
			{
				if(!flg[3])
				{
					throw LACoreInvalidData("All element of flg is false : LAMathLeastSquareSABRCostFuncQlib::set_params", __FILE__, __LINE__);
				}
				else rho = x[0];
			}
			else
			{
				nu = x[0];
				if(flg[3]) rho = x[1];
			}
		}
		else
		{
			beta = x[0];
			if(flg[2])
			{
				nu = x[1];
				if(flg[3]) rho = x[2];
			}
			else
			{
				if(flg[3]) rho = x[1];
			}
		}
	}
	else
	{
		alpha = x[0];
		if(!flg[1])
		{
			if(flg[2])
			{
				nu = x[1];
				if(flg[3]) rho = x[2];
			}
			else
			{
				if(flg[3]) rho = x[1];
			}
		}
		else
		{
			beta = x[1];
			if(flg[2])
			{
				nu = x[2];
				if(flg[3]) rho = x[3];
			}
			else
			{
				if(flg[3]) rho = x[2];
			}
		}
	}

}

//  Cost Func for optimizing SABR Parameters with CapFloor
LAMathLeastSquareSABRCapFloorCostFuncQlib::LAMathLeastSquareSABRCapFloorCostFuncQlib(
	DoubleMatrix alphamat_,
	DoubleMatrix betamat_,
	DoubleMatrix numat_,
	DoubleMatrix rhomat_,
	const std::vector<int>& param_pos_,
	const LAString& approxmethod_,
	const LAString& tenor_point_str_,
	const vector<double>& expiry_capfloor_,
	const vector<double>& expiry_capfloorlet_,
	const vector<double>& tenor_,
	const vector<double>& forward_,
	const vector<double>& numeraire_,
	const vector<double>& strike_,
	const vector<double>& weight_,
	const vector<int>& sgn_,
	const vector<double>& param0_,
	const vector<bool>& flg_,
	const vector<double>& target2fit_,
	const LAString& target_,
	const size_t& num_capfloorlet_,
	const size_t& num_diffterm_
)
	:
	CostFunction(),
	n(strike_.size()),
	alphamat(alphamat_),
	betamat(betamat_),
	numat(numat_),
	rhomat(rhomat_),
	param_pos(param_pos_),
	approxmethod(approxmethod_),
	tenor_point(LAMathSwaptionVolUtility::getTenorPoint(tenor_point_str_)),
	expiry_capfloor(expiry_capfloor_),
	expiry_capfloorlet(expiry_capfloorlet_),
	tenor(tenor_),
	forward(forward_),
	numeraire(numeraire_),
	strike(strike_),
	weight(weight_),
	sgn(sgn_),//Call-Put Sign (-1: put, 1: call)
	param0(param0_),//Initial alpha, beta, nu, rho
	flg(flg_),
	target2fit(target2fit_),
	target(target_),
	num_capfloorlet(num_capfloorlet_),
	num_diffterm(num_diffterm_),
	cloned(false)
{
	if (weight.size() != n)
	{
		throw LACoreInvalidData(" strike.size() != weight.size() : LAMathLeastSquareSABRCostFuncQlib::LAMathLeastSquareSABRCostFuncQlib", __FILE__, __LINE__);
	}

	if (param0.size() != 4)
	{
		throw LACoreInvalidData("param0.size() != 4 : LAMathLeastSquareSABRCostFuncQlib::LAMathLeastSquareSABRCostFuncQlib", __FILE__, __LINE__);
	}

	if (flg.size() != 4)
	{
		throw LACoreInvalidData("flg.size() != 4 : LAMathLeastSquareSABRCostFuncQlib::LAMathLeastSquareSABRCostFuncQlib", __FILE__, __LINE__);
	}

	for (size_t i = 0; i < n; ++i)
	{
		if (sgn[i] != 1 && sgn[i] != -1)
		{
			throw LACoreInvalidData("sgn must -1 or 1 : LAMathLeastSquareSABRCostFuncQlib::LAMathLeastSquareSABRCostFuncQlib", __FILE__, __LINE__);
		}
	}

	alpha = param0[0];
	beta = param0[1];
	nu = param0[2];
	rho = param0[3];

	mpSabr.reset(LAMathSwaptionVolUtility::createSABR(approxmethod, alpha, beta, nu, rho));
}


//
LAMathLeastSquareSABRCapFloorCostFuncQlib::LAMathLeastSquareSABRCapFloorCostFuncQlib(const LAMathLeastSquareSABRCapFloorCostFuncQlib& rhs)
	:
	CostFunction(rhs),
	n(rhs.n),
	alphamat(rhs.alphamat),
	betamat(rhs.betamat),
	numat(rhs.numat),
	rhomat(rhs.rhomat),
	param_pos(rhs.param_pos),
	approxmethod(rhs.approxmethod),
	tenor_point(rhs.tenor_point),
	expiry_capfloor(rhs.expiry_capfloor),
	expiry_capfloorlet(rhs.expiry_capfloorlet),
	forward(rhs.forward),
	numeraire(rhs.numeraire),
	strike(rhs.strike),
	weight(rhs.weight),
	sgn(rhs.sgn),
	param0(rhs.param0),
	flg(rhs.flg),
	target2fit(rhs.target2fit),
	num_capfloorlet(rhs.num_capfloorlet),
	num_diffterm(rhs.num_diffterm),
	alpha(rhs.alpha),
	beta(rhs.beta),
	nu(rhs.nu),
	rho(rhs.rho),
	cloned(true)
{
	mpSabr.reset(rhs.mpSabr->clone());
}


LAMathLeastSquareSABRCapFloorCostFuncQlib::~LAMathLeastSquareSABRCapFloorCostFuncQlib()
{
}

QuantLib::Real LAMathLeastSquareSABRCapFloorCostFuncQlib::value(const QuantLib::Array& x) const
{
	size_t m = x.size();

	if (m == 0 || m > 4)
		throw LACoreInvalidData("m == 0 || m > 4 : LAMathLeastSquareSABRCapFloorCostFuncQlib::value", __FILE__, __LINE__);

	set_params(x);
	
	//set SABR params matrix
	alphamat_temp = alphamat;
	betamat_temp = betamat;
	numat_temp = numat;
	rhomat_temp = rhomat;
	alphamat_temp[param_pos[0]][param_pos[1]] = alpha;
	betamat_temp[param_pos[0]][param_pos[1]] = beta;
	numat_temp[param_pos[0]][param_pos[1]] = nu;
	rhomat_temp[param_pos[0]][param_pos[1]] = rho;
	
	// add parameters of non-calibration-target terms
	// non-target-term parameter is calculated using the ratio of initial target-grid parameter. ex)  non-target-term 3M alpha = (init 3M alpha) * (1Y alpha(dynamic))/(init 1Y alpha) 
	if (num_diffterm != 0 && param_pos[0] == num_diffterm)
	{
		for (size_t i = 0; i < num_diffterm; i++)
		{
			int num_diff = num_diffterm - i;
			alphamat_temp[param_pos[0] - num_diff][param_pos[1]] = alphamat[param_pos[0] - num_diff][param_pos[1]] *alpha / alphamat[param_pos[0]][param_pos[1]];
			betamat_temp[param_pos[0] - num_diff][param_pos[1]] = betamat[param_pos[0] - num_diff][param_pos[1]] * beta / betamat[param_pos[0]][param_pos[1]];
			numat_temp[param_pos[0] - num_diff][param_pos[1]] = numat[param_pos[0] - num_diff][param_pos[1]]* nu / numat[param_pos[0]][param_pos[1]];
			// if rho has diffrent sign from initial value, extapolation should be done in opopsite direction
			double rhosign = (std::signbit(rho * rhomat[param_pos[0]][param_pos[1]]) ? -1. : 1.);
			rhomat_temp[param_pos[0] - num_diff][param_pos[1]] = cos(acos(rho) + rhosign * (acos(rhomat[param_pos[0] - num_diff][param_pos[1]]) - acos(rhomat[param_pos[0]][param_pos[1]])));
		}
	}
	LAMathSwaptionMatrix alphaMat_(alphamat_temp, expiry_capfloor, tenor);
	LAMathSwaptionMatrix betaMat_(betamat_temp, expiry_capfloor, tenor);
	LAMathSwaptionMatrix nuMat_(numat_temp, expiry_capfloor, tenor);
	LAMathSwaptionMatrix rhoMat_(rhomat_temp, expiry_capfloor, tenor);

	QuantLib::Real tmp = 0.;
	double tmp2 = 0.;
	for (size_t i = 0; i < n; ++i)
	{
		if (weight[i] > 0.000000001)
		{
			if (target == CALIB_TARGET_PREMIUM)
			{
				int sgn_ = sgn[i];
				double SABRCapFloorPrem = 0.;
				if (num_capfloorlet != 0)
				{
					// get cap/floor premium by summing up each caplet/floorlet premium
					for (size_t j = 0; j < num_capfloorlet; j++)
					{
						// get SABR params for each caplet/floorlet expiry in an interpolation method
						double alpha_temp = alphaMat_.lookUpMatrix(expiry_capfloorlet[j], tenor_point);
						double beta_temp = betaMat_.lookUpMatrix(expiry_capfloorlet[j], tenor_point);
						double nu_temp = nuMat_.lookUpMatrix(expiry_capfloorlet[j], tenor_point);
						double rho_temp = rhoMat_.lookUpMatrix(expiry_capfloorlet[j], tenor_point);
						mpSabr->setSABRParam(alpha_temp, beta_temp, nu_temp, rho_temp);
						
						SABRCapFloorPrem += mpSabr->getSABRPrem(expiry_capfloorlet[j], forward[j], strike[i], numeraire[j], sgn_);
					}
				}
				else
				{
					LAString msg = "Do not input target premium.";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				tmp2 = sqrt(weight[i]) * (target2fit[i] - SABRCapFloorPrem);
			}
			else if (target == CALIB_TARGET_VOLATILITY)
			{
				LAString msg = "Only premium is allowed for the calibration target.";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			tmp += tmp2 * tmp2;
		}
	}

	return tmp;
}

QuantLib::Array LAMathLeastSquareSABRCapFloorCostFuncQlib::values(const QuantLib::Array& x) const
{
	size_t m = x.size();

	if (m == 0 || m > 4)
		throw LACoreInvalidData("m == 0 || m > 4 : LAMathLeastSquareSABRCapFloorCostFuncQlib::values", __FILE__, __LINE__);

	set_params(x);
	
	//set SABR params matrix
	alphamat_temp = alphamat;
	betamat_temp = betamat;
	numat_temp = numat;
	rhomat_temp = rhomat;
	alphamat_temp[param_pos[0]][param_pos[1]] = alpha;
	betamat_temp[param_pos[0]][param_pos[1]] = beta;
	numat_temp[param_pos[0]][param_pos[1]] = nu;
	rhomat_temp[param_pos[0]][param_pos[1]] = rho;

	// add parameters of non-calibration-target terms
	// non-target-term parameter is calculated using the ratio of initial target-grid parameter. ex)  non-target-term 3M alpha = (init 3M alpha) * (1Y alpha(dynamic))/(init 1Y alpha) 
	if (num_diffterm != 0 && param_pos[0] == num_diffterm)
	{
		for (size_t i = 0; i < num_diffterm; i++)
		{
			int num_diff = num_diffterm - i;
			alphamat_temp[param_pos[0] - num_diff][param_pos[1]] = alphamat[param_pos[0] - num_diff][param_pos[1]] * alpha / alphamat[param_pos[0]][param_pos[1]];
			betamat_temp[param_pos[0] - num_diff][param_pos[1]] = betamat[param_pos[0] - num_diff][param_pos[1]] * beta / betamat[param_pos[0]][param_pos[1]];
			numat_temp[param_pos[0] - num_diff][param_pos[1]] = numat[param_pos[0] - num_diff][param_pos[1]] * nu / numat[param_pos[0]][param_pos[1]];
			// if rho has diffrent sign from initial value, extapolation should be done in opopsite direction
			double rhosign = (std::signbit(rho * rhomat[param_pos[0]][param_pos[1]]) ? -1. : 1.);
			rhomat_temp[param_pos[0] - num_diff][param_pos[1]] = cos(acos(rho) + rhosign * (acos(rhomat[param_pos[0] - num_diff][param_pos[1]]) - acos(rhomat[param_pos[0]][param_pos[1]])));
		}
	}
	LAMathSwaptionMatrix alphaMat_(alphamat_temp, expiry_capfloor, tenor);
	LAMathSwaptionMatrix betaMat_(betamat_temp, expiry_capfloor, tenor);
	LAMathSwaptionMatrix nuMat_(numat_temp, expiry_capfloor, tenor);
	LAMathSwaptionMatrix rhoMat_(rhomat_temp, expiry_capfloor, tenor);

	QuantLib::Array diff_SQs(n);

	for (size_t i = 0; i < n; ++i)
	{
		if (weight[i] > 0.000000001)
		{
			if (target == CALIB_TARGET_PREMIUM)
			{
				int sgn_ = sgn[i];
				double SABRCapFloorPrem = 0;
				if (num_capfloorlet != 0)
				{
					// get cap/floor premium by summing up each caplet/floorlet premium
					for (size_t j = 0; j < num_capfloorlet; j++)
					{
						// get SABR params for each caplet/floorlet expiry in an interpolation method
						double alpha_temp = alphaMat_.lookUpMatrix(expiry_capfloorlet[j], tenor_point);
						double beta_temp = betaMat_.lookUpMatrix(expiry_capfloorlet[j], tenor_point);
						double nu_temp = nuMat_.lookUpMatrix(expiry_capfloorlet[j], tenor_point);
						double rho_temp = rhoMat_.lookUpMatrix(expiry_capfloorlet[j], tenor_point);
						mpSabr->setSABRParam(alpha_temp, beta_temp, nu_temp, rho_temp);

						SABRCapFloorPrem += mpSabr->getSABRPrem(expiry_capfloorlet[j], forward[j], strike[i], numeraire[j], sgn_);
					}
				}
				else
				{
					LAString msg = "Do not input target premium.";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				diff_SQs[i] = sqrt(weight[i]) * (SABRCapFloorPrem - target2fit[i]);
			}
			else if (target == CALIB_TARGET_VOLATILITY)
			{
				LAString msg = "Only premium is allowed for the calibration target.";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}
	}

	return diff_SQs;
}

void LAMathLeastSquareSABRCapFloorCostFuncQlib::set_params(const QuantLib::Array& x) const
{
	// 1st
	if (!flg[0])
	{
		if (!flg[1])
		{
			if (!flg[2])
			{
				if (!flg[3])
				{
					throw LACoreInvalidData("All element of flg is false : LAMathLeastSquareSABRCapFloorCostFuncQlib::set_params", __FILE__, __LINE__);
				}
				else rho = x[0];
			}
			else
			{
				nu = x[0];
				if (flg[3]) rho = x[1];
			}
		}
		else
		{
			beta = x[0];
			if (flg[2])
			{
				nu = x[1];
				if (flg[3]) rho = x[2];
			}
			else
			{
				if (flg[3]) rho = x[1];
			}
		}
	}
	else
	{
		alpha = x[0];
		if (!flg[1])
		{
			if (flg[2])
			{
				nu = x[1];
				if (flg[3]) rho = x[2];
			}
			else
			{
				if (flg[3]) rho = x[1];
			}
		}
		else
		{
			beta = x[1];
			if (flg[2])
			{
				nu = x[2];
				if (flg[3]) rho = x[3];
			}
			else
			{
				if (flg[3]) rho = x[2];
			}
		}
	}
}