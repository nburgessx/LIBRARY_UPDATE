#include "HasCurveData.h"


namespace etrading
{
    void HasCurveData::setCurveData( const std::shared_ptr<CurveData>& ptrMktData)
    {
        ptrMktData_ = ptrMktData;
    }
    std::shared_ptr<CurveData> HasCurveData::getCurveData()
    {
        return ptrMktData_;
    }
};