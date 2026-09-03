// AQLMonotoneConvexInterpolation.cpp

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMonotoneConvexInterpolation.h"
#include "AQLBasic.h"
#include <algorithm>

using namespace std;

//================ AQLInterpolationBase ===================================
/*!
    @brief constructor
*/
AQLMonotoneConvexInterpolation::AQLMonotoneConvexInterpolation()
: AQLInterpolationBase()
{
	mpDataProvider = NULL;
	lambda = 0.0;//not ameliorate
	isAllowedNegative = true;
	inputMode = INPUTGENERAL;
	outputMode = OUTPUTGENERAL;
}

/*!
    @brief constructor 
	@param[in] dLambda coefficient to ameliorate 
	@param[in] dIsallowedNegative allow negative or not
*/
AQLMonotoneConvexInterpolation::AQLMonotoneConvexInterpolation(const double dLambda, const bool dIsAllowedNegative)
{
	mpDataProvider=NULL;
	lambda = dLambda;
	isAllowedNegative = dIsAllowedNegative;
	inputMode = INPUTGENERAL;
	outputMode = OUTPUTGENERAL;
}

/*!
	@brief constructor
	@param[in] dLambda coefficient to ameliorate 
	@param[in] dIsallowedNegative allow negative or not
	@param[in] dInputMode input mode
	@param[in] dOutputMode output mode
*/
AQLMonotoneConvexInterpolation::AQLMonotoneConvexInterpolation(const double dLambda, const bool dIsAllowedNegative,
															 const int dInputMode, const int dOutputMode)
{
	mpDataProvider=NULL;
	lambda = dLambda;
	isAllowedNegative = dIsAllowedNegative;
	inputMode = dInputMode;
	outputMode = dOutputMode;
}

/*!
    @brief destructor
*/
AQLMonotoneConvexInterpolation::~AQLMonotoneConvexInterpolation()
{
	if (mpDataProvider != NULL)
	{
		delete mpDataProvider;
	}
}

/*!
    @brief copy constructor
*/
AQLMonotoneConvexInterpolation::AQLMonotoneConvexInterpolation(const AQLMonotoneConvexInterpolation& v)
: AQLInterpolationBase(v), mpDataProvider(NULL)
{
    try 
	{	
		mpDataProvider = new AQLMonotoneConvexInterpolationDataProvider(*v.mpDataProvider);
		
		// Base Class Data
		interpolationData_ = std::make_shared<InterpolationData>( mpDataProvider->index, mpDataProvider->value );
	}
    catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
}

/*!
    @brief clone
*/
AQLCoreFunctionBase*	
AQLMonotoneConvexInterpolation::clone() const
{
    try 
	{
		AQLMonotoneConvexInterpolation* pTmp;
		pTmp =new AQLMonotoneConvexInterpolation(lambda, isAllowedNegative, inputMode, outputMode);

		if (mpDataProvider != NULL)
		{
			pTmp->mpDataProvider = new AQLMonotoneConvexInterpolationDataProvider(this->mpDataProvider->index, 
				this->mpDataProvider->value, this->mpDataProvider->fdiscrete, this->mpDataProvider->f, this->mpDataProvider->size);

			// Base Class Data
			pTmp->interpolationData_ = std::make_shared<InterpolationData>( pTmp->mpDataProvider->index, pTmp->mpDataProvider->value );
		}

		return pTmp;
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief  check whether this class derives from base class with type id

	@param[in] id ID to check
	@return True or false
*/
bool
AQLMonotoneConvexInterpolation::isTypeOf(function_t id) const
{
	return (id == FN_MONOTONECONVEXINTERPOLATION ? true : AQLInterpolationBase::isTypeOf(id));
}

/*!
    @brief class type
	@return class type
*/
function_t
AQLMonotoneConvexInterpolation::getType() const
{
	return FN_MONOTONECONVEXINTERPOLATION;
}

int
AQLMonotoneConvexInterpolation::getInputMode(void) const
{
	return inputMode;
}

int
AQLMonotoneConvexInterpolation::getOutputMode(void) const
{
	return outputMode;
}

void
AQLMonotoneConvexInterpolation::setInputMode(const int dInputMode)
{
	inputMode = dInputMode;	return;
}

void
AQLMonotoneConvexInterpolation::setOutputMode(const int dOutputMode)
{
	outputMode = dOutputMode;
	return;
}

/*!
	@brief set the information of one-dimensional curve
	@param[in] index X-axis value of the data to be interpolated
	@param[in] value Y-axis value of the data to be interpolated
*/
void
AQLMonotoneConvexInterpolation::set(const DoubleArray& index, const DoubleArray& value)
{
	if (index.size() != value.size())
	{
		throw AQLCoreInvalidData("index size and value size are not same", __FILE__, __LINE__);
	}
	if (index.size() == 0)
	{
		throw AQLCoreInvalidData("data size is zero", __FILE__, __LINE__);
	}
	if (index.size() == 1) return;

	// Check for Duplicates
	if ( index.size() > 1 )
	{
		double previousIndex = index[0];
		double thisIndex = index[1];
		for ( size_t i = 1; i < index.size(); ++i )
		{
			thisIndex		= index[i];
			previousIndex	= index[i-1];
			AQ_THROW_IF(thisIndex == previousIndex, "Invalid Interpolation Data: Duplicate data found with time value: " + AQ_TO_STRING_FROM_DOUBLE(thisIndex) + " years" )
		}
	}

	const unsigned int n = index.size();

	//sort input data		
	std::map<double, double> mp;
	for (unsigned int i = 0; i < n; i++)
	{
		mp.insert(make_pair(index[i], value[i]));
	}
	DoubleArray tau(n);
	DoubleArray input(n);
	std::map<double, double>::iterator it = mp.begin();
	int pos = 0;
	while (it != mp.end())
	{
		//const int pos = distance(mp.begin(), it);
		tau[pos] = it->first;
		input[pos] = it->second;
		it++;
		pos++;
	}
	mp.clear();

	//extend the curve to time 0, for the purpose of calculating forward at time 1
	tau.insert(tau.begin(), 0.0);
	const double tmp = input.front();
	input.insert(input.begin(), tmp);
	
	//step 1
	DoubleArray fdiscrete(n + 1, 0.0);
	switch (inputMode)
	{
	case INPUTFORWARDRATE://forward rate
		copy(++input.begin(), input.end(), ++fdiscrete.begin());
		break;
	case INPUTZERORATE://zero rate
		for (unsigned int j = 1; j <= n; j++)
		{
			fdiscrete[j] = (tau[j] - tau[j - 1] == 0.0) ? input[j]
							: (tau[j] * input[j] - tau[j - 1] * input[j - 1]) / (tau[j] - tau[j - 1]);		}
		break;
	case INPUTGENERAL://general
		for (unsigned int j = 1; j <= n; j++)
		{
			fdiscrete[j] = (tau[j] - tau[j - 1] == 0.0) ? 0.0
							: (input[j] - input[j - 1]) / (tau[j] - tau[j - 1]);		}
		break;
	default:
		throw AQLCoreInvalidData("the input mode is not supported", __FILE__, __LINE__);
	}

	//step 2
	DoubleArray f(n + 1, 0.0);
	if (lambda == 0)
	{
		//set under the unameliorated method
		//numbering refers to Wilmott paper
		//(22)
		for (unsigned int j = 1; j <= n - 1; j++)
		{
			f[j] = (tau[j] - tau[j - 1]) / (tau[j + 1] - tau[j - 1]) * fdiscrete[j + 1]
				+ (tau[j + 1] - tau[j]) / (tau[j + 1] - tau[j - 1]) * fdiscrete[j];
		}
		//(23)
		f[0] = fdiscrete[1] - 0.5 * (f[1] - fdiscrete[1]);
		//(24)
		f[n] = fdiscrete[n] - 0.5 * (f[n - 1] - fdiscrete[n]);
	}
	else
	{
		//set under the ameliorated method
		//numbering refers to AMF paper
		DoubleArray theta_m(n, 0.0);//theta_minus
		DoubleArray theta_p(n, 0.0);//theta_plus
		DoubleArray fmin1(n, 0.0);
		DoubleArray fmax1(n, 0.0);
		DoubleArray fmin2(n, 0.0);
		DoubleArray fmax2(n, 0.0);
		//(72) and (73)
		DoubleArray falseTau(tau);//for the false interval
		falseTau.insert(falseTau.begin(), - tau[1]);//add beginning interval(-1)
		fdiscrete[0] = fdiscrete[1] - (tau[1] - tau[0]) / (tau[2] - tau[0]) * (fdiscrete[2] - fdiscrete[1]);
		falseTau.push_back(2 * tau[n] - tau[n - 1]);//add end interval(n+1)
		fdiscrete.push_back(fdiscrete[n] + (tau[n] - tau[n - 1]) / (tau[n] - tau[n - 2]) * (fdiscrete[n] - fdiscrete[n - 1]));
		//(74) fi
		for (unsigned int j = 0; j <= n; j++)
		{
			f[j] = (falseTau[j + 1] - falseTau[j]) / (falseTau[j + 2] - falseTau[j]) * fdiscrete[j + 1] 
								+ (falseTau[j + 2] - falseTau[j + 1]) / (falseTau[j + 2] - falseTau[j]) * fdiscrete[j];
		}
		//(68) theta_minus
		for (unsigned int j = 1; j <= n - 1; j++)
		{
			theta_m[j] = (falseTau[j + 1] - falseTau[j]) / (falseTau[j + 1] - falseTau[j - 1]) * (fdiscrete[j] - fdiscrete[j - 1]);
		}
		//(71) theta_plus
		for (unsigned int j = 1; j <= n - 1; j++)
		{
			theta_p[j] = (falseTau[j + 2] - falseTau[j + 1]) / (falseTau[j + 3] - falseTau[j + 1]) * (fdiscrete[j + 2] - fdiscrete[j + 1]);
		}
		//(67) fmin1, fmax1
		for (unsigned int j = 1; j <= n - 1; j++)
		{
			if (fdiscrete[j - 1] < fdiscrete[j] && fdiscrete[j] <= fdiscrete[j + 1])
			{
				fmin1[j] = min(fdiscrete[j] + 0.5 * theta_m[j], fdiscrete[j + 1]);
				fmax1[j] = min(fdiscrete[j] + 2.0 * theta_m[j], fdiscrete[j + 1]);
			}
			else if (fdiscrete[j - 1] < fdiscrete[j] && fdiscrete[j] > fdiscrete[j + 1])
			{
				fmin1[j] = max(fdiscrete[j] - 0.5 * lambda * theta_m[j], fdiscrete[j + 1]);
				fmax1[j] = fdiscrete[j];
			}
			else if (fdiscrete[j - 1] >= fdiscrete[j] && fdiscrete[j] <= fdiscrete[j + 1])
			{
				fmin1[j] = fdiscrete[j];
				fmax1[j] = min(fdiscrete[j] - 0.5 * lambda * theta_m[j], fdiscrete[j + 1]);
			}
			else if (fdiscrete[j - 1] >= fdiscrete[j] && fdiscrete[j] > fdiscrete[j + 1])
			{
				fmin1[j] = max(fdiscrete[j] + 2.0 * theta_m[j], fdiscrete[j + 1]);
				fmax1[j] = max(fdiscrete[j] + 0.5 * theta_m[j], fdiscrete[j + 1]);
			}
		}
		//(70) fmin2, fmax2
		for (unsigned int j = 1; j <= n - 1; j++)
		{
			if (fdiscrete[j] < fdiscrete[j + 1] && fdiscrete[j + 1] <= fdiscrete[j + 2])
			{
				fmin2[j] = max(fdiscrete[j + 1] - 2.0 * theta_p[j], fdiscrete[j]);
				fmax2[j] = max(fdiscrete[j + 1] - 0.5 * theta_p[j], fdiscrete[j]);
			}
			else if (fdiscrete[j] < fdiscrete[j + 1] && fdiscrete[j + 1] > fdiscrete[j + 2])
			{
				fmin2[j] = max(fdiscrete[j + 1] + 0.5 * lambda * theta_p[j], fdiscrete[j]);
				fmax2[j] = fdiscrete[j + 1];
			}
			else if (fdiscrete[j] >= fdiscrete[j + 1] && fdiscrete[j + 1] < fdiscrete[j + 2])
			{
				fmin2[j] = fdiscrete[j + 1];
				fmax2[j] = min(fdiscrete[j + 1] + 0.5 * lambda * theta_p[j], fdiscrete[j]);
			}
			else if (fdiscrete[j] >= fdiscrete[j + 1] && fdiscrete[j + 1] >= fdiscrete[j + 2])
			{
				fmin2[j] = min(fdiscrete[j + 1] - 0.5 * theta_p[j], fdiscrete[j]);
				fmax2[j] = min(fdiscrete[j + 1] - 2.0 * theta_p[j], fdiscrete[j]);
			}
		}
		for (unsigned int j = 1; j <= n - 1; j++) //ameliorate fi
		{
			if (max(fmin1[j], fmin2[j]) <= min(fmax1[j], fmax2[j])) //overlap case
			{
				//(75, 76)
				f[j] = max(max(fmin1[j], fmin2[j]), min(f[j], min(fmax1[j], fmax2[j])));//adjust in the common range
			}
			else //not overlap case
			{
				//(78)
				f[j] = max(min(fmax1[j], fmax2[j]), min(f[j], max(fmin1[j], fmin2[j])));//adjust in the gap
			}
		}
		//(79) ameliorate f0
		if (AQLMath::abs(f[0] - fdiscrete[0]) > 0.5 * AQLMath::abs(f[1] - fdiscrete[0])) 
			f[0] = fdiscrete[1] - 0.5 * (f[1] - fdiscrete[0]);
		//(80) ameliorate fn
		if (AQLMath::abs(f[n] - fdiscrete[n]) > 0.5 * AQLMath::abs(f[n - 1] - fdiscrete[n])) 
			f[n] = fdiscrete[n] - 0.5 * (f[n - 1] - fdiscrete[n]);
	}

	//step 3
	if (!isAllowedNegative)
	{
		if(fdiscrete[1] >= 0)
			f[0] = max(0.0, min(f[0], 2 * fdiscrete[1]));
		for (unsigned int j = 1; j <= n - 1; j++)
		{
			if(min(fdiscrete[j], fdiscrete[j + 1]) >= 0)
				f[j] =max(0.0, min(f[j], 2 * min(fdiscrete[j], fdiscrete[j + 1])));
		}
		if(fdiscrete[n] >= 0)
			f[n] = max(0.0, min(f[n], 2 * fdiscrete[n]));
	}

	if (mpDataProvider != NULL)
	{
		//delete mpDataProvider if it has values
		delete mpDataProvider;
	}
	
	mpDataProvider = new AQLMonotoneConvexInterpolationDataProvider(tau, input, fdiscrete, f, n + 1);
	interpolationData_ = std::make_shared<InterpolationData>( index, value );
	return;
}

/*!
    @brief a value of one-dimensional complement of the curve
	@param[in] x1 a point for interpolation
	@return a value of one-dimensional complement of the curve
*/
double
AQLMonotoneConvexInterpolation::value(const double x1) const
{
	if (mpDataProvider == NULL)
	{
		throw AQLCoreInvalidData("#Error: Monotone Convex interpolation data has not been set", __FILE__, __LINE__);
	}
	if ( mpDataProvider->size == 1) return mpDataProvider->value[0];

	switch (outputMode)
	{
	case OUTPUTFORWARDRATE://forward rate
		return forward(x1);
	case OUTPUTZERORATE://zero rate
		return interpolant(x1);
	case OUTPUTGENERAL://general
		return interpolantGeneral(x1);
	case OUTPUTANALYZE://analyze interpolant case
		return analyzeInterpolant(x1);
	default:
		throw AQLCoreInvalidData("the output mode is not supported", __FILE__, __LINE__);
	}
} 

/*!
    @brief a value of one-dimensional complement of the forward rate (private function)
	@param[in] x1 a point for interpolation
	@return a value of one-dimensional complement of the forward rate
*/
double
AQLMonotoneConvexInterpolation::forward(const double x1) const
{
	//numbering refers to Wilmott paper
	int size = mpDataProvider->size;
	double ret = 0.0;
	if (x1 <= 0) //horizontal extrapolate
	{
	    ret = mpDataProvider->f[0];
	}
	else if (x1 > mpDataProvider->index[size - 1]) //horizontal extrapolate
	{
	    ret = forward(mpDataProvider->index[size - 1]);
	}
	else
	{
		DoubleArray::const_iterator it = upper_bound(mpDataProvider->index.begin(), mpDataProvider->index.end(), x1);
		int i = distance(mpDataProvider->index.begin(), it) - (it == mpDataProvider->index.end() ? 2 : 1);
		double x, g0, g1, G, eta, A;
		//the x in (25)
		x = (x1 - mpDataProvider->index[i]) / (mpDataProvider->index[i + 1] - mpDataProvider->index[i]);
		g0 = mpDataProvider->f[i] - mpDataProvider->fdiscrete[i + 1];
		g1 = mpDataProvider->f[i + 1] - mpDataProvider->fdiscrete[i + 1];
		if (x == 0)
		{
			G = g0;
		}
		else if (x == 1)
		{
			G = g1;
		}
		else if ((g0 < 0 && -0.5 * g0 <= g1 && g1 <= -2 * g0) || (g0 > 0 && -0.5 * g0 >= g1 && g1 >= -2 * g0))
		{
			//zone (i)
			G = g0 * (1.0 - 4.0 * x + 3.0 * pow(x, 2)) + g1 * (- 2.0 * x + 3.0 * pow(x, 2));
		}
		else if ((g0 < 0 && g1 > -2 * g0) || (g0 > 0 && g1 < -2 * g0))
		{
			//zone (ii)
			//(29)
			eta = (g1 + 2.0 * g0) / (g1 - g0);
			//(28)
			if (x <= eta)
			{
				G = g0;
			}
			else
			{
				G = g0 + (g1 - g0) * pow((x - eta) / (1.0 - eta), 2);
			}
		}
		else if ((g0 > 0 && 0 > g1 && g1 > -0.5 * g0) || (g0 < 0 && 0 < g1 && g1 < -0.5 * g0))
		{
			//zone (iii)
			//(31)
			eta = 3.0 * g1 / (g1 - g0);
			//(30)
			if (x <= eta)
			{
				G = g1 + (g0 - g1) * pow((eta - x) / eta, 2);
			}
			else
			{
				G = g1;
			}
		}
		else if (g0 == 0 || g1 == 0)
		{
			G = 0.0;
		}
		else
		{
			//zone (iv)
			//(33)
			eta = g1 / (g1 + g0);
			//(34)
			A = - g0 * g1 / (g0 + g1);
			//(32)
			if (x <= eta)
			{
				G = A + (g0 - A) * pow((eta - x) / eta, 2);
			}
			else
			{
				G = A + (g1 - A) * pow((x - eta) / (1.0 - eta), 2);
			}
		}
		ret = G + mpDataProvider->fdiscrete[i + 1];
	}
	return ret;
}

/*!
    @brief a value of one-dimensional complement of the zero rate (private function)
	@param[in] x1 a point for interpolation
	@return a value of one-dimensional complement of the zero rate
*/
double
AQLMonotoneConvexInterpolation::interpolant(const double x1) const
{
	//numbering refers to Wilmott paper
	int size = mpDataProvider->size;
	double ret = 0.0;
	if (x1 <= 0) //horizontal extrapolate of forward
	{
		ret = mpDataProvider->f[0];
	}
	else if (x1 > mpDataProvider->index[size - 1]) //horizontal extrapolate of forward
	{
	    ret = 1.0 / x1 * (interpolant(mpDataProvider->index[size - 1]) * mpDataProvider->index[size - 1] 
			+ forward(mpDataProvider->index[size - 1]) * (x1 - mpDataProvider->index[size - 1]));
	}
	else
	{
		DoubleArray::const_iterator it = upper_bound(mpDataProvider->index.begin(), mpDataProvider->index.end(), x1);
		int i = distance(mpDataProvider->index.begin(), it) - (it == mpDataProvider->index.end() ? 2 : 1);
		double x, g0, g1, G, eta, A;
		//the x in (25)
		x = (x1 - mpDataProvider->index[i]) / (mpDataProvider->index[i + 1] - mpDataProvider->index[i]);
		g0 = mpDataProvider->f[i] - mpDataProvider->fdiscrete[i + 1];
		g1 = mpDataProvider->f[i + 1] - mpDataProvider->fdiscrete[i + 1];
		if (x == 0 || x == 1)
		{
			G = 0.0;
		}
		else if ((g0 < 0 && -0.5 * g0 <= g1 && g1 <= -2 * g0) || (g0 > 0 && -0.5 * g0 >= g1 && g1 >= -2 * g0))
		{
			//zone (i)
			G = g0 * (x - 2.0 * pow(x, 2) + pow(x, 3)) + g1 * (- pow(x, 2) + pow(x, 3));
		}
		else if ((g0 < 0 && g1 > -2 * g0) || (g0 > 0 && g1 < -2 * g0))
		{
			//zone (ii)
			//(29)
			eta = (g1 + 2.0 * g0) / (g1 - g0);
			//(28)
			if (x <= eta)
			{
				G = g0 * x;
			}
			else
			{
				G = g0 * x + 1.0 / 3.0 * (g1 - g0) * pow(x - eta, 3) / pow(1.0 - eta, 2);
			}
		}
		else if ((g0 > 0 && 0 > g1 && g1 > -0.5 * g0) || (g0 < 0 && 0 < g1 && g1 < -0.5 * g0))
		{
			//zone (iii)
			//(31)
			eta = 3.0 * g1 / (g1 - g0);
			//(30)
			if (x <= eta)
			{
				G = g1 * x - 1.0 / 3.0 * (g0 - g1) * (pow(eta - x, 3) / pow(eta, 2) - eta);
			}
			else
			{
				G = (2.0 / 3.0 * g1 + 1.0 / 3.0 * g0) * eta + g1 * (x - eta);
			}
		}
		else if (g0 == 0 || g1 == 0)
		{
			G = 0.0;
		}
		else
		{
			//zone (iv)
			//(33)
			eta = g1 / (g1 + g0);
			//(34)
			A = - g0 * g1 / (g0 + g1);
			//(32)
			if (x <= eta)
			{
				G = A * x - 1.0 / 3.0 * (g0 - A) * (pow(eta - x, 3) / pow(eta, 2) - eta);
			}
			else
			{
				G = (2.0 / 3.0 * A + 1.0 / 3.0 * g0) * eta + A * (x - eta) + 1.0 / 3.0 * (g1 - A) * pow(x - eta, 3) / pow(1.0 - eta, 2);
			}
		}
		ret = 1.0 / x1 * (mpDataProvider->index[i] * mpDataProvider->value[i] + (x1 - mpDataProvider->index[i]) * mpDataProvider->fdiscrete[i + 1] 
			+ (mpDataProvider->index[i + 1] - mpDataProvider->index[i]) * G);
	}
	return ret;
}

/*!
    @brief a value of one-dimensional complement
	@param[in] x1 a point for interpolation
	@return a value of one-dimensional complement
*/
double
AQLMonotoneConvexInterpolation::interpolantGeneral(const double x1) const
{	
	//numbering refers to Wilmott paper	
	int size = mpDataProvider->size;
	double ret = 0.0;
	if (x1 <= 0) //horizontal extrapolate of differential
	{
		//ret = mpDataProvider->f[0];
		ret = mpDataProvider->value[0] + mpDataProvider->f[0] * (x1 - mpDataProvider->index[0]);
	}
	else if (x1 > mpDataProvider->index[size - 1]) //horizontal extrapolate of differential
	{
		//ret = interpolantGeneral(mpDataProvider->index[size - 1]) + forward(mpDataProvider->index[size - 1]) * (x1 - mpDataProvider->index[size - 1]);
		ret = mpDataProvider->value[size - 1] + mpDataProvider->f[size - 1] * (x1 - mpDataProvider->index[size - 1]);
	}
	else
	{
		DoubleArray::const_iterator it = upper_bound(mpDataProvider->index.begin(), mpDataProvider->index.end(), x1);	
		int i = distance(mpDataProvider->index.begin(), it) - (it == mpDataProvider->index.end() ? 2 : 1);
		double x, g0, g1, G, eta, A;
		//the x in (25)	
		x = (x1 - mpDataProvider->index[i]) / (mpDataProvider->index[i + 1] - mpDataProvider->index[i]);
		g0 = mpDataProvider->f[i] - mpDataProvider->fdiscrete[i + 1];
		g1 = mpDataProvider->f[i + 1] - mpDataProvider->fdiscrete[i + 1];
		if (x == 0 || x == 1)
		{
			G = 0.0;
		}
		else if ((g0 < 0 && -0.5 * g0 <= g1 && g1 <= -2 * g0) || (g0 > 0 && -0.5 * g0 >= g1 && g1 >= -2 * g0))
		{
			//zone (i)	
			G = g0 * (x - 2.0 * pow(x, 2) + pow(x, 3)) + g1 * (- pow(x, 2) + pow(x, 3));
		}
		else if ((g0 < 0 && g1 > -2 * g0) || (g0 > 0 && g1 < -2 * g0))
		{
			//zone (ii)	
			//(29)			
			eta = (g1 + 2.0 * g0) / (g1 - g0);
			//(28)			
			if (x <= eta)			
			{				
				G = g0 * x;	
			}			
			else
			{	
				G = g0 * x + 1.0 / 3.0 * (g1 - g0) * pow(x - eta, 3) / pow(1.0 - eta, 2);
			}
		}
		else if ((g0 > 0 && 0 > g1 && g1 > -0.5 * g0) || (g0 < 0 && 0 < g1 && g1 < -0.5 * g0))
		{			
			//zone (iii)
			//(31)			
			eta = 3.0 * g1 / (g1 - g0);
			//(30)
			if (x <= eta)
			{
				G = g1 * x - 1.0 / 3.0 * (g0 - g1) * (pow(eta - x, 3) / pow(eta, 2) - eta);
			}
			else
			{
				G = (2.0 / 3.0 * g1 + 1.0 / 3.0 * g0) * eta + g1 * (x - eta);
			}
		}
		else if (g0 == 0 || g1 == 0)
		{
			G = 0.0;
		}
		else
		{
			//zone (iv)
			//(33)			
			eta = g1 / (g1 + g0);
			//(34)			
			A = - g0 * g1 / (g0 + g1);
			//(32)			
			if (x <= eta)
			{
				G = A * x - 1.0 / 3.0 * (g0 - A) * (pow(eta - x, 3) / pow(eta, 2) - eta);
			}
			else
			{
				G = (2.0 / 3.0 * A + 1.0 / 3.0 * g0) * eta + A * (x - eta) + 1.0 / 3.0 * (g1 - A) * pow(x - eta, 3) / pow(1.0 - eta, 2);
			}
		}
		ret = mpDataProvider->value[i] + (x1 - mpDataProvider->index[i]) * mpDataProvider->fdiscrete[i + 1] + (mpDataProvider->index[i + 1] - mpDataProvider->index[i]) * G;
	}
	return ret;
}

/*!
    @brief analyze which interpolant case select
	@param[in] x1 a point for interpolation
	@return interpolant case
*/
double
AQLMonotoneConvexInterpolation::analyzeInterpolant(const double x1) const
{
	int size = mpDataProvider->size;
	double ret = 0.0;
	if (x1 <= 0) 
	{
		//zone min
		ret = -0.1;
	}
	else if (x1 > mpDataProvider->index[size - 1])
	{
		// zone max
	    ret = -0.2;
	}
	else
	{
		DoubleArray::const_iterator it = upper_bound(mpDataProvider->index.begin(), mpDataProvider->index.end(), x1);	
		int i = distance(mpDataProvider->index.begin(), it) - (it == mpDataProvider->index.end() ? 2 : 1);
		double x, g0, g1, eta;
		x = (x1 - mpDataProvider->index[i]) / (mpDataProvider->index[i + 1] - mpDataProvider->index[i]);
		g0 = mpDataProvider->f[i] - mpDataProvider->fdiscrete[i + 1];
		g1 = mpDataProvider->f[i + 1] - mpDataProvider->fdiscrete[i + 1];
		if (x == 0)
		{
			//zone start point
			ret = -1.1;
		}
		else if (x == 1)
		{
			//zone end point
			ret = -1.2;
		}
		else if ((g0 < 0 && -0.5 * g0 <= g1 && g1 <= -2 * g0) || (g0 > 0 && -0.5 * g0 >= g1 && g1 >= -2 * g0))
		{
			//zone (i)
			ret = 1.0;
		}
		else if ((g0 < 0 && g1 > -2 * g0) || (g0 > 0 && g1 < -2 * g0))
		{
			//zone (ii)
			eta = (g1 + 2.0 * g0) / (g1 - g0);
			if (x <= eta)
			{
				ret = 2.1;
			}
			else
			{
				ret = 2.2;
			}
		}
		else if ((g0 > 0 && 0 > g1 && g1 > -0.5 * g0) || (g0 < 0 && 0 < g1 && g1 < -0.5 * g0))
		{
			//zone (iii)
			eta = 3.0 * g1 / (g1 - g0);
			if (x <= eta)
			{
				ret = 3.1;
			}
			else
			{
				ret = 3.2;
			}
		}
		else if (g0 == 0 || g1 == 0)
		{
			//zone zero
			ret = 0.0;
		}
		else
		{
			//zone (iv)
			eta = g1 / (g1 + g0);
			if (x <= eta)
			{
				ret = 4.1;
			}
			else
			{
				ret = 4.2;
			}
		}
	}
	return ret;
};


std::tuple<std::vector<double>,std::vector<double>> AQLMonotoneConvexInterpolation::getXY() const
{
	return std::make_tuple(mpDataProvider->index, mpDataProvider->value);
};
