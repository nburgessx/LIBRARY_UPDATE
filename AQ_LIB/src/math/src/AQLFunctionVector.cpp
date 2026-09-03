#include "AQLBasic.h"
#include "AQLFunctionVector.h"

AQLFunctionVector::~AQLFunctionVector() 
{
}

bool AQLFunctionVector::stopNow() {
   return false;
}

double  AQLFunctionVector::finiteDifferencingWidth(){
   static const double  d = AQLMath::pow(DBL_EPSILON, 1.0/3.0);
   return d;
}

double  AQLFunctionVector::argumentTolerance(){
    static const double  d = AQLMath::sqrt(DBL_EPSILON);
   return d;
}

double  AQLFunctionVector::functionTolerance(){
   static const double  d = AQLMath::sqrt(DBL_EPSILON);
   return d;
}

unsigned long AQLFunctionVector::maximumNumberOfIterations() {
	return DBL_DIG*10;
}

void AQLFunctionVector::setJacobianByCentreDifferencing(std::vector<std::vector<double > >&jacobian, const std::vector <double > &x) {

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

void AQLFunctionVector::operator()(std::vector<std::vector<double > > &jacobian, const std::vector <double > &x) {
   setJacobianByCentreDifferencing(jacobian, x);
}

void AQLFunctionVector::operator()(std::vector<std::vector<double > > &jacobian, std::vector<double > &f, const std::vector <double > &x) {
   (*this)(f,x);
   if (stopNow()) return;
   (*this)(jacobian,f,x);
};


bool AQLFunctionVector::anyNegative(const std::vector <double > &x){
   ASSERT( x.size() >= lengthOfArgumentVector() );
   const unsigned long m = lengthOfArgumentVector();
   for (unsigned long i=0; i<m; ++i) if (x[i]<0.) return true;
   return false;
}
   
bool AQLFunctionVector::anyNonPositive(const std::vector <double > &x){
   ASSERT( x.size() >= lengthOfArgumentVector() );
   const unsigned long m = lengthOfArgumentVector();
   for (unsigned long i=0; i<m; ++i) if (x[i]<=0.) return true;
   return false;
}

bool AQLFunctionVector::anyNotANumber(const std::vector <double > &x){
   ASSERT( x.size() >= lengthOfArgumentVector() );
   const unsigned long m = lengthOfArgumentVector();
   for (unsigned long i=0; i<m; ++i) if ( ! ieee::isNumber(x[i]) ) return true;
   return false;
}

bool AQLFunctionVector::constraintsAreViolated(const std::vector <double > &x) {
   return anyNotANumber(x);
}

