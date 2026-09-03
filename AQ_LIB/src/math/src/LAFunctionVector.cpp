#include "LABasic.h"
#include "LAFunctionVector.h"

LAFunctionVector::~LAFunctionVector() 
{
}

bool LAFunctionVector::stopNow() {
   return false;
}

double  LAFunctionVector::finiteDifferencingWidth(){
   static const double  d = LAMath::pow(DBL_EPSILON, 1.0/3.0);
   return d;
}

double  LAFunctionVector::argumentTolerance(){
    static const double  d = LAMath::sqrt(DBL_EPSILON);
   return d;
}

double  LAFunctionVector::functionTolerance(){
   static const double  d = LAMath::sqrt(DBL_EPSILON);
   return d;
}

unsigned long LAFunctionVector::maximumNumberOfIterations() {
	return DBL_DIG*10;
}

void LAFunctionVector::setJacobianByCentreDifferencing(std::vector<std::vector<double > >&jacobian, const std::vector <double > &x) {

   ASSERT(jacobian.size() >= lengthOfFunctionVector() );
   ASSERT(jacobian[0].size() >= lengthOfArgumentVector() );
   ASSERT(x.size() >= lengthOfArgumentVector() );

   const unsigned long m = lengthOfArgumentVector();
   const unsigned long n = lengthOfFunctionVector();
   const double  eps = finiteDifferencingWidth();
   unsigned long i,j;
   double  d;

   m_workSpaceForArgumentVector = x;
   if ( m_workSpaceForFunctionVector.size() < n )
      m_workSpaceForFunctionVector.resize(n);
   
   for (j=0; j<m; ++j){
      d = x[j] * eps;
      if (d<eps)
         d = eps;
      m_workSpaceForArgumentVector[j] += d;
      (*this)(m_workSpaceForFunctionVector, m_workSpaceForArgumentVector);
//      (*this)(m_workSpaceForArgumentVector,m_workSpaceForFunctionVector2);
      if (stopNow()) return;
      for (i=0; i<n; ++i)
         jacobian[i][j] = m_workSpaceForFunctionVector[i];
      m_workSpaceForArgumentVector[j] = x[j]-d;
//      (*this)(m_workSpaceForArgumentVector,m_workSpaceForFunctionVector2);
      (*this)(m_workSpaceForFunctionVector,m_workSpaceForArgumentVector);
      if (stopNow())
         return;
      for (i=0;i<n;i++)
         jacobian[i][j] = (jacobian[i][j] - m_workSpaceForFunctionVector[i]) / (2*d);
      m_workSpaceForArgumentVector[j] = x[j];
   }
}

void LAFunctionVector::operator()(std::vector<std::vector<double > > &jacobian, const std::vector <double > &x) {
   setJacobianByCentreDifferencing(jacobian, x);
}

void LAFunctionVector::operator()(std::vector<std::vector<double > > &jacobian, std::vector<double > &f, const std::vector <double > &x) {
   (*this)(f,x);
   if (stopNow()) return;
   (*this)(jacobian,f,x);
};


bool LAFunctionVector::anyNegative(const std::vector <double > &x){
   ASSERT( x.size() >= lengthOfArgumentVector() );
   const unsigned long m = lengthOfArgumentVector();
   for (unsigned long i=0; i<m; ++i) if (x[i]<0.) return true;
   return false;
}
   
bool LAFunctionVector::anyNonPositive(const std::vector <double > &x){
   ASSERT( x.size() >= lengthOfArgumentVector() );
   const unsigned long m = lengthOfArgumentVector();
   for (unsigned long i=0; i<m; ++i) if (x[i]<=0.) return true;
   return false;
}

bool LAFunctionVector::anyNotANumber(const std::vector <double > &x){
   ASSERT( x.size() >= lengthOfArgumentVector() );
   const unsigned long m = lengthOfArgumentVector();
   for (unsigned long i=0; i<m; ++i) if ( ! ieee::isNumber(x[i]) ) return true;
   return false;
}

bool LAFunctionVector::constraintsAreViolated(const std::vector <double > &x) {
   return anyNotANumber(x);
}

