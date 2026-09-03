#include <LAMathIntegralMulti.h>
#include <algorithm>

//
LAMathIntegralMulti::LAMathIntegralMulti()
:
func_integral_1D_0(0),
step_integral_1D_0(0),
func_integral_1D_0_(0),
step_integral_1D_0_(0),
func_integral_2D_0(0),
steps_integral_2D_0(0),
func_integral_2D_1(0),
steps_integral_2D_1(0),
func_integral_2D_2(0),
steps_integral_2D_2(0),
func_integral_3D_0(0),
steps_integral_3D_0(0)
{
	integral_2D_0_helper_.SetFunc(*this, &LAMathIntegralMulti::integral_2D_0_helper);
	integral_2D_1_helper_.SetFunc(*this, &LAMathIntegralMulti::integral_2D_1_helper);
	integral_2D_2_helper_.SetFunc(*this, &LAMathIntegralMulti::integral_2D_2_helper);
    integral_3D_0_helper_.SetFunc(*this, &LAMathIntegralMulti::integral_3D_0_helper);
}

// int_1D_0
void LAMathIntegralMulti::Set_func_integral_1D_0(LAFunction* method, int step) const
{
	func_integral_1D_0 = method;
	step_integral_1D_0 = step;
}

void LAMathIntegralMulti::Set_func_integral_1D_0_(LAFunction* method, int step) const
{
	func_integral_1D_0_ = method;
	step_integral_1D_0_ = step;
}

void LAMathIntegralMulti::Set_func_integral_1D_0__(LAFunction* method, int step) const
{
	func_integral_1D_0__ = method;
	step_integral_1D_0__ = step;
}

//
double LAMathIntegralMulti::integral_1D_0(double t) const
{
	//return func_integral_1D_0->Integrate_GLeg(0, t, step_integral_1D_0);
	return func_integral_1D_0->IntegrateGLegendre(0, t, step_integral_1D_0);
}

//
double LAMathIntegralMulti::integral_1D_0(double ts, double te) const
{
	//return func_integral_1D_0->Integrate_GLeg(ts, te, step_integral_1D_0);
	return func_integral_1D_0->IntegrateGLegendre(ts, te, step_integral_1D_0);
}

double LAMathIntegralMulti::integral_1D_0_(double t) const
{
	//return func_integral_1D_0_->Integrate_GLeg(0, t, step_integral_1D_0_);
	return func_integral_1D_0_->IntegrateGLegendre(0, t, step_integral_1D_0_);
}

double LAMathIntegralMulti::integral_1D_0_(double ts, double te) const
{
	//return func_integral_1D_0_->Integrate_GLeg(ts, te, step_integral_1D_0_);
	return func_integral_1D_0_->IntegrateGLegendre(ts, te, step_integral_1D_0_);
}

double LAMathIntegralMulti::integral_1D_0__(double t) const
{
	//return func_integral_1D_0__->Integrate_GLeg(0, t, step_integral_1D_0__);
	return func_integral_1D_0__->IntegrateGLegendre(0, t, step_integral_1D_0__);
}

double LAMathIntegralMulti::integral_1D_0__(double ts, double te) const
{
	//return func_integral_1D_0__->Integrate_GLeg(ts, te, step_integral_1D_0__);
	return func_integral_1D_0__->IntegrateGLegendre(ts, te, step_integral_1D_0__);
}

// int_2D_0
void LAMathIntegralMulti::Set_func_integral_2D_0(const vector<LAFunction*>& method, const vector<int>& steps) const
{
	func_integral_2D_0 = method;
	steps_integral_2D_0 = steps;
}

//
double LAMathIntegralMulti::integral_2D_0(double t) const
{
	Set_func_integral_1D_0(func_integral_2D_0[1], steps_integral_2D_0[1]);
	//return integral_2D_0_helper_.Integrate_GLeg(0., t, steps_integral_2D_0[0]);
	return integral_2D_0_helper_.IntegrateGLegendre(0., t, steps_integral_2D_0[0]);
}

//
double LAMathIntegralMulti::integral_2D_0(double ts, double te) const
{
	Set_func_integral_1D_0(func_integral_2D_0[1], steps_integral_2D_0[1]);
	//return integral_2D_0_helper_.Integrate_GLeg(ts, te, steps_integral_2D_0[0]);
	return integral_2D_0_helper_.IntegrateGLegendre(ts, te, steps_integral_2D_0[0]);
}

//
double LAMathIntegralMulti::integral_2D_0_helper (double t) const
{
	return func_integral_2D_0[0]->operator() (t) * integral_1D_0(t);
}

// int_2D_1
void LAMathIntegralMulti::Set_func_integral_2D_1(const vector<LAFunction*>& method, const vector<int>& steps) const
{
	func_integral_2D_1 = method;
	steps_integral_2D_1 = steps;
}

//
double LAMathIntegralMulti::integral_2D_1(double t) const
{
	Set_func_integral_1D_0(func_integral_2D_1[1], steps_integral_2D_1[1]);
	//return integral_2D_1_helper_.Integrate_GLeg(0., t, steps_integral_2D_1[0]);
	return integral_2D_1_helper_.IntegrateGLegendre(0., t, steps_integral_2D_1[0]);

}

//
double LAMathIntegralMulti::integral_2D_1(double ts, double te) const
{
	Set_func_integral_1D_0(func_integral_2D_1[1], steps_integral_2D_1[1]);
	//return integral_2D_1_helper_.Integrate_GLeg(ts, te, steps_integral_2D_1[0]);
	return integral_2D_1_helper_.IntegrateGLegendre(ts, te, steps_integral_2D_1[0]);
}

//
double LAMathIntegralMulti::integral_2D_1_helper(double t) const
{
    double tmp = integral_1D_0(t);
	return func_integral_2D_1[0]->operator() (t) * tmp * tmp;
}

// int_2D_2
void LAMathIntegralMulti::Set_func_integral_2D_2(const vector<LAFunction*>& method, const vector<int>& steps) const
{
	func_integral_2D_2 = method;
	steps_integral_2D_2 = steps;
}

//
double LAMathIntegralMulti::integral_2D_2(double t) const
{
	Set_func_integral_1D_0_(func_integral_2D_2[1], steps_integral_2D_2[1]);
    Set_func_integral_1D_0__(func_integral_2D_2[2], steps_integral_2D_2[2]);
	//return integral_2D_2_helper_.Integrate_GLeg(0., t, steps_integral_2D_2[0]);
	return integral_2D_2_helper_.IntegrateGLegendre(0., t, steps_integral_2D_2[0]);
}

//
double LAMathIntegralMulti::integral_2D_2(double ts, double te) const
{
	Set_func_integral_1D_0_(func_integral_2D_2[1], steps_integral_2D_2[1]);
    Set_func_integral_1D_0__(func_integral_2D_2[2], steps_integral_2D_2[2]);
	//return integral_2D_2_helper_.Integrate_GLeg(ts, te, steps_integral_2D_2[0]);
	return integral_2D_2_helper_.IntegrateGLegendre(ts, te, steps_integral_2D_2[0]);
}

//
double LAMathIntegralMulti::integral_2D_2_helper(double t) const
{
	return func_integral_2D_2[0]->operator() (t) * integral_1D_0_(t) * integral_1D_0__(t);
}

// int_3D_0
void LAMathIntegralMulti::Set_func_integral_3D_0(const vector<LAFunction*>& method, const vector<int>& steps) const
{
	func_integral_3D_0 = method;
	steps_integral_3D_0 = steps;
}

//
double LAMathIntegralMulti::integral_3D_0 (double t) const
{
	vector<LAFunction*> tmp1(func_integral_3D_0.begin()+1, func_integral_3D_0.end());
	vector<int> tmp2(steps_integral_3D_0.begin()+1, steps_integral_3D_0.end());
	Set_func_integral_2D_0(tmp1, tmp2);

	//return integral_3D_0_helper_.Integrate_GLeg(0., t, steps_integral_3D_0[0]);
	return integral_3D_0_helper_.IntegrateGLegendre(0., t, steps_integral_3D_0[0]);
}

//
double LAMathIntegralMulti::integral_3D_0 (double ts, double te) const
{
	vector<LAFunction*> tmp1(func_integral_3D_0.begin()+1, func_integral_3D_0.end());
	vector<int> tmp2(steps_integral_3D_0.begin()+1, steps_integral_3D_0.end());
	Set_func_integral_2D_0(tmp1, tmp2);

	//return integral_3D_0_helper_.Integrate_GLeg(ts, te, steps_integral_3D_0[0]);
	return integral_3D_0_helper_.IntegrateGLegendre(ts, te, steps_integral_3D_0[0]);
}

//
double LAMathIntegralMulti::integral_3D_0_helper(double t) const
{
	return func_integral_3D_0[0]->operator () (t) * integral_2D_0(t);
}