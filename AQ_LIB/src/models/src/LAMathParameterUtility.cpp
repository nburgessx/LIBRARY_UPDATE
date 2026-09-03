#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <LAMathParameterUtility.h>
#include <LAMathInterpolationUtilities.h>
#include <LADataMatrix.h>
#include <LAMathDefine.h>
#include <LAMathDateUtilities.h>
#include <LAPriceCashFlowGenerator.h>
#include <LAMathBaseFuncUtility.h>
#include <LAFunctionUtilities.h>
#include <LACoreUtil.h>

void LAMathParameterObject::SetParameterMatrix(LADataInstance* dataInstance, const LAString& matID, const LAString& convID, LAStringMatrix& mat)
{
    const LAObject& conventions = dataInstance->getObjectPool().getObject(convID, ENCHKTYPE_ISDEFINED ).get();
    const LAPriceDataCalendar& cal = dynamic_cast<const LAPriceDataCalendar& >(conventions.getData(PRICING_DATA_FIXINGCALENDAR, ISDEFINED).get());
    const LAPriceDataSlidingRule& sr = dynamic_cast<const LAPriceDataSlidingRule& >(conventions.getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED).get());
    const LADate asOfDate = dynamic_cast<const LADataDate& >(conventions.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get()).get();

    size_t nRows = mat.size();
    size_t nCols = mat[0].size();

    if(nRows <= 1 || nCols <= 1)
        throw LACoreInvalidData("The size of matrix is small!", __FILE__, __LINE__);

    LAStringVector expiryTerms;
    DoubleVector expiryVec;
    vector<LADate> expiryDates;
    for(size_t i = 1; i < nRows; i++)
    {
        expiryTerms.push_back(mat[i][0]);
        LADate expiryDate = CalendarAdvance(asOfDate, mat[i][0], sr, cal);
        expiryDates.push_back(expiryDate);
        expiryVec.push_back(ModelTime(asOfDate, expiryDate));
    }

    LAStringVector indexVec;
    for(size_t i = 1; i < nCols; i++)
        indexVec.push_back(mat[0][i]);

    DoubleMatrix mat_per(nCols - 1, DoubleVector(nRows - 1));
    for (size_t i = 0; i < nCols - 1; i++)
        for (size_t j = 0; j < nRows - 1; j++)
            mat_per[i][j] = mat[j + 1][i + 1].getDoubleValue();

    LAObjectPool &objPool = dataInstance->getObjectPool();
    LAObjectHolder objHolder = objPool.getObject(matID, ENCHKTYPE_NOCHECK);
    if(!objHolder.isDefined())
    {
        LAObject* e = new LAObject();
        e->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(matID);
        e->add(CALIBRATION_DATA_ASOFDATE, new LADataDate(asOfDate));
        e->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar(cal));
        e->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule(sr));
        e->add(AQ_DATAMATRIX, new LADataDoubleMatrix(mat_per));
        e->add(AQ_EXPIRYTERMS, new LADataStrings(expiryTerms));
        e->add(AQ_EXPIRYDATES, new LADataDates(expiryDates));
        e->add(AQ_EXPIRYVECTOR, new LADataDoubles(expiryVec));
        e->add(AQ_INDEXVECTOR, new LADataStrings(indexVec));
        objPool.set(matID , e);
    }
    else if(objHolder.isDefined())
    {
        LADataHolder* dh;

        dh = &objHolder.getData(CALIBRATION_DATA_ASOFDATE, ISDEFINED);
        LADataDate& asOfDate_att = dynamic_cast<LADataDate &>(dh->get());
        asOfDate_att.set(asOfDate);

        dh = &objHolder.getData(CALIBRATION_DATA_CALENDAR, ISDEFINED);
        LAPriceDataCalendar& cal_att = dynamic_cast<LAPriceDataCalendar &>(dh->get());
        cal_att = cal;

        dh = &objHolder.getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED);
        LAPriceDataSlidingRule& sr_att = dynamic_cast<LAPriceDataSlidingRule &>(dh->get());
        sr_att = sr;

        dh = &objHolder.getData(AQ_DATAMATRIX, ISDEFINED);
        LADataDoubleMatrix& mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
        mat_att.set(mat_per);

        dh = &objHolder.getData(AQ_EXPIRYTERMS, ISDEFINED);
        LADataStrings& expiryTerms_att = dynamic_cast<LADataStrings &>(dh->get());
        expiryTerms_att.set(expiryTerms);

        dh = &objHolder.getData(AQ_EXPIRYDATES, ISDEFINED);
        LADataDates& expiryDates_att = dynamic_cast<LADataDates &>(dh->get());
        expiryDates_att.set(expiryDates);

        dh = &objHolder.getData(AQ_EXPIRYVECTOR, ISDEFINED);
        LADataDoubles& expiryVec_att = dynamic_cast<LADataDoubles &>(dh->get());
        expiryVec_att.set(expiryVec);

        dh = &objHolder.getData(AQ_INDEXVECTOR, ISDEFINED);
        LADataStrings& tenorVec_att = dynamic_cast<LADataStrings &>(dh->get());
        tenorVec_att.set(indexVec);
    }
}

LAStringMatrix LAMathParameterObject::ParameterMatrix(LADataInstance* dataInstance, const LAString& matID)
{
    LAObjectPool& objPool = dataInstance->getObjectPool();
    const DoubleMatrix& dataMatrix = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_DATAMATRIX, ISDEFINED).get()).get();
    const LAStringVector& expiryVec = dynamic_cast<const LADataStrings& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_EXPIRYTERMS, ISDEFINED).get()).get();
    const LAStringVector& indexVec = dynamic_cast<const LADataStrings& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_INDEXVECTOR, ISDEFINED).get()).get();

    // Reconstruct matrix
    size_t nExpiries = expiryVec.size();
    size_t nIndexes = indexVec.size();
    if (dataMatrix.size() != nIndexes || dataMatrix[0].size() != nExpiries)
        throw LACoreInvalidData("Invalid parameter matrix sizes", __FILE__, __LINE__);

    LAStringMatrix matrix(nExpiries + 1, LAStringVector(nIndexes + 1));
    for (size_t i = 0; i < nExpiries + 1; i++)
    {
        if (i == 0)
        {
            matrix[0][0] = matID;
            for (size_t j = 0; j < nIndexes; j++)
                matrix[0][1 + j] = indexVec[j];
        }
        else
        {
            matrix[i][0] = expiryVec[i - 1];
            for (size_t j = 0; j < nIndexes; j++)
                matrix[i][1 + j] = LAString(n2s(dataMatrix[j][i - 1]).c_str());
        }
    }

    return matrix;
}

double LAMathParameterObject::LookUpParameterMatrix(LADataInstance* dataInstance, const LAString& matID, const LAString& expPoint,
                                                 const LAString& idxPoint, LAString interpolationType)
{
    LAObjectPool& objPool = dataInstance->getObjectPool();
    const DoubleMatrix& dataMatrix = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_DATAMATRIX, ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_EXPIRYVECTOR, ISDEFINED).get()).get();
    const LAStringVector& indexVec = dynamic_cast<const LADataStrings& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_INDEXVECTOR, ISDEFINED).get()).get();
    const LAPriceDataSlidingRule& paySlr = dynamic_cast<const LAPriceDataSlidingRule& >(objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).
        get().getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED).get());
    const LAPriceDataCalendar& fixCal = dynamic_cast<const LAPriceDataCalendar& >(objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().
        getData(CALIBRATION_DATA_CALENDAR, ISDEFINED).get());
    const LADate& asOfDate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(matID, ENCHKTYPE_ISDEFINED).
        get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get()).get();

    LADate expDate =  GetDate(asOfDate, expPoint, paySlr, fixCal);
    return Lookup(asOfDate, expDate, idxPoint, expiryVec, indexVec, dataMatrix, interpolationType);
}

double LAMathParameterObject::LookUpParameterMatrix(LADataInstance* dataInstance, const LAString& matID, LADate expDate,
                                                 const LAString& idxPoint, LAString interpolationType)
{
    LAObjectPool& objPool = dataInstance->getObjectPool();
    const DoubleMatrix& dataMatrix = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_DATAMATRIX, ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_EXPIRYVECTOR, ISDEFINED).get()).get();
    const LAStringVector& indexVec = dynamic_cast<const LADataStrings& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_INDEXVECTOR, ISDEFINED).get()).get();
    const LADate& asOfDate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(matID, ENCHKTYPE_ISDEFINED).
        get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get()).get();

    return Lookup(asOfDate, expDate, idxPoint, expiryVec, indexVec, dataMatrix, interpolationType);
}

LAStringVector LAMathParameterObject::ParameterMatrixTerms(LADataInstance* dataInstance, const LAString& matID)
{
    LAObjectPool& objPool = dataInstance->getObjectPool();
    const LAStringVector& terms = dynamic_cast<const LADataStrings& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_EXPIRYTERMS, ISDEFINED).get()).get();
    return terms;
}

LAStringVector LAMathParameterObject::ParameterMatrixIndexes(LADataInstance* dataInstance, const LAString& matID)
{
    LAObjectPool& objPool = dataInstance->getObjectPool();
    const LAStringVector& indexes = dynamic_cast<const LADataStrings& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_INDEXVECTOR, ISDEFINED).get()).get();
    return indexes;
}

LADate LAMathParameterObject::GetDate(const LADate& asOfDate, LAString term, const LAPriceDataSlidingRule& slr, const LAPriceDataCalendar& cal)
{
    return CalendarAdvance(asOfDate, term, slr, cal);
}

double LAMathParameterObject::Lookup(LADate asOfDate, LADate expDate, const LAString& idxPoint,
                                  const DoubleVector& expiryVec, const LAStringVector& indexVec,
                                  const DoubleMatrix& dataMatrix, LAString interpolationType)
{
    double expPoint_d = ModelTime(asOfDate, expDate);
    MFParameterMatrix parameterMatrix(dataMatrix, expiryVec, indexVec);
    return parameterMatrix.LookUp(expPoint_d, idxPoint, interpolationType);
}

void LAMathParameterObject::SetMatrixAxis(const LAString& title, const LAStringVector& terms, const LAStringVector& indexes,
                                       LAStringMatrix& matrix)
{
    size_t nTerms = terms.size();
    size_t nIndexes = indexes.size();
    matrix = LAStringMatrix(nTerms + 1, LAStringVector(nIndexes + 1));
    matrix[0][0] = title;
    for (size_t i = 0; i < nTerms; i++)
        matrix[1 + i][0] = terms[i];
    for (size_t j = 0; j < nIndexes; j++)
        matrix[0][1 + j] = indexes[j];
}

void LAMathParameterObject::SetMatrixData(const DoubleMatrix& data, LAStringMatrix& matrix)
{
    size_t nRows = matrix.size();
    size_t nCols = matrix[0].size();
    if (data.size() != nRows - 1 || data[0].size() != nCols - 1)
        throw LACoreInvalidData("Invalid sizes in data matrix", __FILE__, __LINE__);

    for (size_t i = 0; i < data.size(); i++)
        for (size_t j = 0; j < data[0].size(); j++)
            matrix[1 + i][1 + j] = LAString(n2s(data[i][j]).c_str());
}

MFParameterMatrix::MFParameterMatrix(const DoubleMatrix& mat_, const DoubleVector& mExpiries_, const LAStringVector& mIndexes_)
: mMatrix(mat_), mExpiries(mExpiries_), mIndexes(mIndexes_)
{
}

double MFParameterMatrix::LookUp(double expiry, const LAString& indexID, LAString interpolationType)
{
    if(expiry < 0.0)
        throw LACoreInvalidData("Expiry is negative", __FILE__, __LINE__);

    size_t idx;
    if (!(TryFind(indexID, mIndexes, idx)))
        throw LACoreInvalidData(LAString("Index " + indexID + " not found").getCString(), __FILE__, __LINE__);

    return LAMathInterpolationUtilities::interpolate(mExpiries, mMatrix[idx], expiry, etrading::toInterpolationEnum(interpolationType.c_str()));
}
