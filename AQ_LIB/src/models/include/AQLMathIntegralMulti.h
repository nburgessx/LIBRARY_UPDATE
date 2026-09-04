#ifndef __AQLMATHINTEGRALMULTI_H__
#define __AQLMATHINTEGRALMULTI_H__

#include "AQLFunction.h"
#include <vector>

using namespace std;

class AQLMathIntegralMulti
{
public:
	//
	AQLMathIntegralMulti();

	// integral_1D_0::int_{0}^{t} f(s) ds
	void Set_func_integral_1D_0(AQLFunction* method, int step) const;
	double integral_1D_0(double t) const;
	double integral_1D_0(double ts, double te) const;
	
	void Set_func_integral_1D_0_(AQLFunction* method, int step) const;
	double integral_1D_0_(double t) const;
	double integral_1D_0_(double ts, double te) const;

	void Set_func_integral_1D_0__(AQLFunction* method, int step) const;
	double integral_1D_0__(double t) const;
	double integral_1D_0__(double ts, double te) const;

    // integral_2D_0::int_{0}^{t} f(s) int_{0}^{s} f(u) du ds
	void Set_func_integral_2D_0(const vector<AQLFunction*>& method, const vector<int>& steps) const;
	double integral_2D_0(double t) const;
	double integral_2D_0(double ts, double te) const;
	
	// integral_2D_1::int_{0}^{t} f(s) ( int_{0}^{s} f(u) du)^{2} ds
	void Set_func_integral_2D_1(const vector<AQLFunction*>& method, const vector<int>& steps) const;
	double integral_2D_1(double t) const;
	double integral_2D_1(double ts, double te) const;
	
    // integral_2D_2::int_{0}^{t} f(s) ( int_{0}^{s} f(u) du ) (int_{0}^{u} f(u)  du) ds
	void Set_func_integral_2D_2(const vector<AQLFunction*>& method, const vector<int>& steps) const;
	double integral_2D_2(double t) const;
	double integral_2D_2(double ts, double te) const;

    // integral_3D_0::int_{0}^{t} f(s) int_{0}^{s} f(u) int_{0}^{u} f(r) dr du ds
	void Set_func_integral_3D_0(const vector<AQLFunction*>& method, const vector<int>& steps) const;
	double integral_3D_0(double t) const;
	double integral_3D_0(double ts, double te) const;

private:

	//
	AQLMathIntegralMulti(const AQLMathIntegralMulti& rhs) {rhs;}

	// integral_1D_0
	mutable AQLFunction* func_integral_1D_0;
	mutable int step_integral_1D_0;

	mutable AQLFunction* func_integral_1D_0_;
	mutable int step_integral_1D_0_;

	mutable AQLFunction* func_integral_1D_0__;
	mutable int step_integral_1D_0__;

	// integral_2D_0
	mutable vector<AQLFunction*> func_integral_2D_0;
	mutable vector<int> steps_integral_2D_0;
	
	// integral_2D_1
	mutable vector<AQLFunction*> func_integral_2D_1;
	mutable vector<int> steps_integral_2D_1;

    // integral_2D_2
	mutable vector<AQLFunction*> func_integral_2D_2;
	mutable vector<int> steps_integral_2D_2;

    // integral_3D_0
	mutable vector<AQLFunction*> func_integral_3D_0;
	mutable vector<int> steps_integral_3D_0;

	//
	mutable AQLMathFunction<AQLMathIntegralMulti> integral_2D_0_helper_;
	mutable AQLMathFunction<AQLMathIntegralMulti> integral_2D_1_helper_;
	mutable AQLMathFunction<AQLMathIntegralMulti> integral_2D_2_helper_;
    mutable AQLMathFunction<AQLMathIntegralMulti> integral_3D_0_helper_;

	//
	double integral_2D_0_helper(double t) const;
	double integral_2D_1_helper(double t) const;
	double integral_2D_2_helper(double t) const;
    double integral_3D_0_helper(double t) const;

};
#endif