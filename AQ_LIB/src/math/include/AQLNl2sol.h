#ifndef AQLNl2sol_h
#define AQLNl2sol_h

#include <vector>
#include "AQLFunctionVector.h"

class NL2SOL {
public:
   NL2SOL(AQLFunctionVector & function);
   
   /// Note : The function below uses half the sum of the squares of the function vector as its error measure.
   /// It also calculates Jacobians always by finite differencing, thus only using operator()(std::vector<double> & f, const std::vector<double> & x) of the functor.
   /// The return value is the number of iterations it took. A negative value indicates failure to converge.
   long tryToSolve(std::vector<double> &startingAndEndPointForTheParameters, double *achievedErrorMeasure = 0 );
   long tryToSolve2(std::vector<double> &x, double *errormeasure = 0);

private:

   int nl2sno(int *n, int *p, double *x, int *iv, double *v);
   int nl2sol(int *n, int *p, double *x, int *iv, double *v);
   
   //int nl2sol(int *n, int *p, double *x,
   //           void (*calcJ)(int *n, int *p, double *x, int *withinbounds, double *residualvector, int *userIntegerParameter, double *userdoubleParameter, int (*userFunctionParameter)(int*)),
   //           int *iv, double *v,
   //           int *userIntegerParameter, double *userdoubleParameter, int (*userFunctionParameter)(int*));

   int nl2itr(double *, int *, double *, int *, int *, int *, double *, double *, double *);
   void calcR(int *n, int *p, double *, int *, double *);
   void calcJ(int *n, int *p, double *x, int *withinbounds, double *j);
   int UFP(int *);


   AQLFunctionVector& m_function;
   std::vector<double> m_f, m_x;
   std::vector<std::vector<double> > m_j;

   unsigned long m_lv, m_liv;
   std::vector<double> m_workspace;
   std::vector<int> m_iv;

   static int assess(double *, int *, int *, double *, double *, double *, double *, double *);
   static double dotprd(int *, double *, double *);
   static int gqtstp(double *, double *, double *, int *, double *, int *, double *, double *, double *);
   static int linvrt(int *, double *, double *);
   static int litvmu(int *, double *, double *, double *);
   static int livmul(int *, double *, double *, double *);
   static int lmstep(double *, double *, int *, int *, int *, int *, double *, double *, double *, double *, double *);
   static int lsqrt(int , int *, double *, double *, int *);
   static double lsvmin(int *, double *, double *, double *);
   static int ltsqar(int *, double *, double *);
   static int nl2sol_defaults(int *iv, double *v);
   static int parchk(int *, int *, int *, int *, double *);
   static int covclc(int *, double *, int *, double *, int *, int *, int *, double *, double *, double *);
   static int qrfact(int *, int *, int *, double *, double *, int *, int *, int , double *);
   static int dupdat(double *, int *, double *, int *, int *, int *, double *);
   static int qapply(int *, int *n, int *p, double *, double *, int *);
   static double reldst(int *, double *, double *, double *);
   static double rmdcon(int k);
   static int rptmul(int , int *, double *, int *, int *, double *, double *, double *, double *);
   static int slupdt(double *, double *, int *, double *, double *, double *, double *, double *, double *, double *);
   static int slvmul(int *, double *, double *, double *);
   static bool stopx(bool);
   static double v2norm(int *, double *);
   static int vaxpy(int *, double *, double, double *, double *);
   static int vcopy(int, double *, double *);
   static int vscopy(int *, double *, double);
}; 

#endif
