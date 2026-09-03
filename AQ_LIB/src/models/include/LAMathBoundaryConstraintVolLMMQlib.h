#ifndef LAMathBoundaryConstraintVolLMMQlib_h
#define LAMathBoundaryConstraintVolLMMQlib_h

#include "LAQuantLibConstraint.h"

// %Constraint of vol_LMM_model1_V
class LAMathBoundaryConstraintLMMVolVQlib : public QuantLib::Constraint
{
public:
    LAMathBoundaryConstraintLMMVolVQlib(double low, double high)
    :
    QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>( new LAMathBoundaryConstraintLMMVolVQlib::Impl(low, high) ) )
    {
    }

private:
    class Impl : public QuantLib::Constraint::Impl
    {
    public:
        Impl(double low, double high)
        :
        low_(low),
        high_(high)
        {
        }
        
        bool test(const QuantLib::Array& params) const
        {
            if( params.size() != 4) throw AQLCoreInvalidData("size out of range : Object::get_ID",__FILE__,__LINE__);

            if(params[0] + params[3] < low_) return false;
            if(params[3] < low_ ) return false;
            if(params[2] < low_ ) return false;

            if(params[0] > high_) return false;
            if(params[1] > high_) return false;
            if(params[2] > high_) return false;
            if(params[3] > high_) return false;

            return true;
        }

    private:
        double low_, high_;
    };
};

//
//----------
//
class LAMathBoundaryConstraintLMMVolFQlib : public QuantLib::Constraint
{
public:
    LAMathBoundaryConstraintLMMVolFQlib(double low, double high)
    :
    QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>( new LAMathBoundaryConstraintLMMVolFQlib::Impl(low, high) ) )
    {
    }

private:
    class Impl : public QuantLib::Constraint::Impl
    {
    public:
        Impl(double low, double high)
        :
        low_(low),
        high_(high)
        {
        }
        
        bool test(const QuantLib::Array& params) const
        {
            if( params.size() != 8) throw AQLCoreInvalidData("size out of range : Object::get_ID",__FILE__,__LINE__);

            if(params[0] < low_ ) return false;
            if(params[1] < low_ ) return false;
            if(params[2] < 0.0 ) return false;
            if(params[3] < low_ ) return false;
            if(params[4] < 0.0 ) return false;
            if(params[5] < low_ ) return false;
            if(params[6] < 0.0 ) return false;
            if(params[7] < low_ ) return false;

            if(params[0] > high_) return false;
            if(params[1] > high_) return false;
            if(params[2] > 3.141593) return false;
            if(params[3] > high_) return false;
            if(params[4] > 3.141593) return false;
            if(params[5] > high_) return false;
            if(params[6] > 3.141593) return false;
            if(params[7] > high_) return false;

            if(params[0] < 0.0 ) return false;
            if(params[7] < 0.0 ) return false;
            return true;
        }

    private:
        double low_, high_;
    };
};

//
//----------
//
class LAMathBoundaryConstraintLMMVolGQlib : public QuantLib::Constraint
{
public:
    LAMathBoundaryConstraintLMMVolGQlib(double low, double high, size_t G_size_, double smooth_bound_ = numeric_limits<double>::max())
    :
    QuantLib::Constraint(boost::shared_ptr<QuantLib::Constraint::Impl>( new LAMathBoundaryConstraintLMMVolGQlib::Impl(low, high, G_size_, smooth_bound_) ) )
    {
    }

private:
    class Impl : public QuantLib::Constraint::Impl
    {
    public:
        Impl(double low, double high, size_t G_size_, double smooth_bound_)
        :
        low_(low),
        high_(high),
        G_size(G_size_),
        smooth_bound(smooth_bound_)
        {
        }
        
        bool test(const QuantLib::Array& params) const
        {
            if( params.size() != G_size ) throw AQLCoreInvalidData("size is not correct! : Object::get_ID",__FILE__,__LINE__);

           if( params[0] < low_ ) return false;
           if( params[0] > high_ ) return false;
           if( params[G_size - 1] < low_ ) return false;
           if( params[G_size - 1] > high_ ) return false;

            for(size_t i = 1; i < G_size - 2; ++i)
            {
                if( params[i] < low_ ) return false;
                if( params[i] > high_ ) return false;
                
                double jump = fabs( params[i - 1] - 2.0 * params[i] + params[i + 1] );
                if( jump * jump > smooth_bound  )
                {
                    return false;
                }
            }
           
           return true;
        }

    private:
        double low_, high_;
        size_t G_size;
        double smooth_bound;
    };
};
#endif
