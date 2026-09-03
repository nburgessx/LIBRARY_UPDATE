#ifndef __LAMATHINTEGRALMULTI_H__
#define __LAMATHINTEGRALMULTI_H__

#include "LAFunction.h"
#include <vector>

using namespace std;

class LAMathIntegralMulti
{
public:
	//
	LAMathIntegralMulti();

	// integral_1D_0::int_{0}^{t} f(s) ds
	void Set_func_integral_1D_0(LAFunction* method, int step) const;
	double integral_1D_0(double t) const;
	double integral_1D_0(double ts, double te) const;
	
	void Set_func_integral_1D_0_(LAFunction* method, int step) const;
	double integral_1D_0_(double t) const;
	double integral_1D_0_(double ts, double te) const;

	void Set_func_integral_1D_0__(LAFunction* method, int step) const;
	double integral_1D_0__(double t) const;
	double integral_1D_0__(double ts, double te) const;

    // integral_2D_0::int_{0}^{t} f(s) int_{0}^{s} f(u) du ds
	void Set_func_integral_2D_0(const vector<LAFunction*>& method, const vector<int>& steps) const;
	double integral_2D_0(double t) const;
	double integral_2D_0(double ts, double te) const;
	
	// integral_2D_1::int_{0}^{t} f(s) ( int_{0}^{s} f(u) du)^{2} ds
	void Set_func_integral_2D_1(const vector<LAFunction*>& method, const vector<int>& steps) const;
	double integral_2D_1(double t) const;
	double integral_2D_1(double ts, double te) const;
	
    // integral_2D_2::int_{0}^{t} f(s) ( int_{0}^{s} f(u) du ) (int_{0}^{u} f(u)  du) ds
	void Set_func_integral_2D_2(const vector<LAFunction*>& method, const vector<int>& steps) const;
	double integral_2D_2(double t) const;
	double integral_2D_2(double ts, double te) const;

    // integral_3D_0::int_{0}^{t} f(s) int_{0}^{s} f(u) int_{0}^{u} f(r) dr du ds
	void Set_func_integral_3D_0(const vector<LAFunction*>& method, const vector<int>& steps) const;
	double integral_3D_0(double t) const;
	double integral_3D_0(double ts, double te) const;

private:

	//
	LAMathIntegralMulti(const LAMathIntegralMulti& rhs) {rhs;}

	// integral_1D_0
	mutable LAFunction* func_integral_1D_0;
	mutable int step_integral_1D_0;

	mutable LAFunction* func_integral_1D_0_;
	mutable int step_integral_1D_0_;

	mutable LAFunction* func_integral_1D_0__;
	mutable int step_integral_1D_0__;

	// integral_2D_0
	mutable vector<LAFunction*> func_integral_2D_0;
	mutable vector<int> steps_integral_2D_0;
	
	// integral_2D_1
	mutable vector<LAFunction*> func_integral_2D_1;
	mutable vector<int> steps_integral_2D_1;

    // integral_2D_2
	mutable vector<LAFunction*> func_integral_2D_2;
	mutable vector<int> steps_integral_2D_2;

    // integral_3D_0
	mutable vector<LAFunction*> func_integral_3D_0;
	mutable vector<int> steps_integral_3D_0;

	//
	mutable LAMathFunction<LAMathIntegralMulti> integral_2D_0_helper_;
	mutable LAMathFunction<LAMathIntegralMulti> integral_2D_1_helper_;
	mutable LAMathFunction<LAMathIntegralMulti> integral_2D_2_helper_;
    mutable LAMathFunction<LAMathIntegralMulti> integral_3D_0_helper_;

	//
	double integral_2D_0_helper(double t) const;
	double integral_2D_1_helper(double t) const;
	double integral_2D_2_helper(double t) const;
    double integral_3D_0_helper(double t) const;

};
#endif