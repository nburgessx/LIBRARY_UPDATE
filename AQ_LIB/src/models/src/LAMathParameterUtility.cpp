#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <LAMathParameterUtility.h>
#include <LAMathInterpolationUtilities.h>
#include <AQLDataMatrix.h>
#include <AQLMathDefine.h>
#include <LAMathDateUtilities.h>
#include <LAPriceCashFlowGenerator.h>
#include <LAMathBaseFuncUtility.h>
#include <AQLFunctionUtilities.h>
#include <AQLCoreUtil.h>

void LAMathParameterObject::SetParameterMatrix(AQLDataInstance* dataInstance, const AQLString& matID, const AQLString& convID, AQLStringMatrix& mat)
{
    const AQLObject& conventions = dataInstance->getObjectPool().getObject(convID, ENCHKTYPE_ISDEFINED ).get();
    const AQLPriceDataCalendar& cal = dynamic_cast<const AQLPriceDataCalendar& >(conventions.getData(PRICING_DATA_FIXINGCALENDAR, ISDEFINED).get());
    const AQLPriceDataSlidingRule& sr = dynamic_cast<const AQLPriceDataSlidingRule& >(conventions.getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED).get());
    const AQLDate asOfDate = dynamic_cast<const AQLDataDate& >(conventions.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get()).get();

    size_t nRows = mat.size();
    size_t nCols = mat[0].size();

    if(nRows <= 1 || nCols <= 1)
        throw AQLCoreInvalidData("The size of matrix is small!", __FILE__, __LINE__);

    AQLStringVector expiryTerms;
    DoubleVector expiryVec;
    vector<AQLDate> expiryDates;
    for(size_t i = 1; i < nRows; i++)
    {
        expiryTerms.push_back(mat[i][0]);
        AQLDate expiryDate = CalendarAdvance(asOfDate, mat[i][0], sr, cal);
        expiryDates.push_back(expiryDate);
        expiryVec.push_back(ModelTime(asOfDate, expiryDate));
    }

    AQLStringVector indexVec;
    for(size_t i = 1; i < nCols; i++)
        indexVec.push_back(mat[0][i]);

    DoubleMatrix mat_per(nCols - 1, DoubleVector(nRows - 1));
    for (size_t i = 0; i < nCols - 1; i++)
        for (size_t j = 0; j < nRows - 1; j++)
            mat_per[i][j] = mat[j + 1][i + 1].getDoubleValue();

    AQLObjectPool &objPool = dataInstance->getObjectPool();
    AQLObjectHolder objHolder = objPool.getObject(matID, ENCHKTYPE_NOCHECK);
    if(!objHolder.isDefined())
    {
        AQLObject* e = new AQLObject();
        e->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(matID);
        e->add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asOfDate));
        e->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar(cal));
        e->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule(sr));
        e->add(AQ_DATAMATRIX, new AQLDataDoubleMatrix(mat_per));
        e->add(AQ_EXPIRYTERMS, new AQLDataStrings(expiryTerms));
        e->add(AQ_EXPIRYDATES, new AQLDataDates(expiryDates));
        e->add(AQ_EXPIRYVECTOR, new AQLDataDoubles(expiryVec));
        e->add(AQ_INDEXVECTOR, new AQLDataStrings(indexVec));
        objPool.set(matID , e);
    }
    else if(objHolder.isDefined())
    {
        AQLDataHolder* dh;

        dh = &objHolder.getData(CALIBRATION_DATA_ASOFDATE, ISDEFINED);
        AQLDataDate& asOfDate_att = dynamic_cast<AQLDataDate &>(dh->get());
        asOfDate_att.set(asOfDate);

        dh = &objHolder.getData(CALIBRATION_DATA_CALENDAR, ISDEFINED);
        AQLPriceDataCalendar& cal_att = dynamic_cast<AQLPriceDataCalendar &>(dh->get());
        cal_att = cal;

        dh = &objHolder.getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED);
        AQLPriceDataSlidingRule& sr_att = dynamic_cast<AQLPriceDataSlidingRule &>(dh->get());
        sr_att = sr;

        dh = &objHolder.getData(AQ_DATAMATRIX, ISDEFINED);
        AQLDataDoubleMatrix& mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
        mat_att.set(mat_per);

        dh = &objHolder.getData(AQ_EXPIRYTERMS, ISDEFINED);
        AQLDataStrings& expiryTerms_att = dynamic_cast<AQLDataStrings &>(dh->get());
        expiryTerms_att.set(expiryTerms);

        dh = &objHolder.getData(AQ_EXPIRYDATES, ISDEFINED);
        AQLDataDates& expiryDates_att = dynamic_cast<AQLDataDates &>(dh->get());
        expiryDates_att.set(expiryDates);

        dh = &objHolder.getData(AQ_EXPIRYVECTOR, ISDEFINED);
        AQLDataDoubles& expiryVec_att = dynamic_cast<AQLDataDoubles &>(dh->get());
        expiryVec_att.set(expiryVec);

        dh = &objHolder.getData(AQ_INDEXVECTOR, ISDEFINED);
        AQLDataStrings& tenorVec_att = dynamic_cast<AQLDataStrings &>(dh->get());
        tenorVec_att.set(indexVec);
    }
}

AQLStringMatrix LAMathParameterObject::ParameterMatrix(AQLDataInstance* dataInstance, const AQLString& matID)
{
    AQLObjectPool& objPool = dataInstance->getObjectPool();
    const DoubleMatrix& dataMatrix = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_DATAMATRIX, ISDEFINED).get()).get();
    const AQLStringVector& expiryVec = dynamic_cast<const AQLDataStrings& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_EXPIRYTERMS, ISDEFINED).get()).get();
    const AQLStringVector& indexVec = dynamic_cast<const AQLDataStrings& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_INDEXVECTOR, ISDEFINED).get()).get();

    // Reconstruct matrix
    size_t nExpiries = expiryVec.size();
    size_t nIndexes = indexVec.size();
    if (dataMatrix.size() != nIndexes || dataMatrix[0].size() != nExpiries)
        throw AQLCoreInvalidData("Invalid parameter matrix sizes", __FILE__, __LINE__);

    AQLStringMatrix matrix(nExpiries + 1, AQLStringVector(nIndexes + 1));
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
                matrix[i][1 + j] = AQLString(n2s(dataMatrix[j][i - 1]).c_str());
        }
    }

    return matrix;
}

double LAMathParameterObject::LookUpParameterMatrix(AQLDataInstance* dataInstance, const AQLString& matID, const AQLString& expPoint,
                                                 const AQLString& idxPoint, AQLString interpolationType)
{
    AQLObjectPool& objPool = dataInstance->getObjectPool();
    const DoubleMatrix& dataMatrix = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_DATAMATRIX, ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_EXPIRYVECTOR, ISDEFINED).get()).get();
    const AQLStringVector& indexVec = dynamic_cast<const AQLDataStrings& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_INDEXVECTOR, ISDEFINED).get()).get();
    const AQLPriceDataSlidingRule& paySlr = dynamic_cast<const AQLPriceDataSlidingRule& >(objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).
        get().getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED).get());
    const AQLPriceDataCalendar& fixCal = dynamic_cast<const AQLPriceDataCalendar& >(objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().
        getData(CALIBRATION_DATA_CALENDAR, ISDEFINED).get());
    const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(matID, ENCHKTYPE_ISDEFINED).
        get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get()).get();

    AQLDate expDate =  GetDate(asOfDate, expPoint, paySlr, fixCal);
    return Lookup(asOfDate, expDate, idxPoint, expiryVec, indexVec, dataMatrix, interpolationType);
}

double LAMathParameterObject::LookUpParameterMatrix(AQLDataInstance* dataInstance, const AQLString& matID, AQLDate expDate,
                                                 const AQLString& idxPoint, AQLString interpolationType)
{
    AQLObjectPool& objPool = dataInstance->getObjectPool();
    const DoubleMatrix& dataMatrix = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_DATAMATRIX, ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_EXPIRYVECTOR, ISDEFINED).get()).get();
    const AQLStringVector& indexVec = dynamic_cast<const AQLDataStrings& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_INDEXVECTOR, ISDEFINED).get()).get();
    const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(matID, ENCHKTYPE_ISDEFINED).
        get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get()).get();

    return Lookup(asOfDate, expDate, idxPoint, expiryVec, indexVec, dataMatrix, interpolationType);
}

AQLStringVector LAMathParameterObject::ParameterMatrixTerms(AQLDataInstance* dataInstance, const AQLString& matID)
{
    AQLObjectPool& objPool = dataInstance->getObjectPool();
    const AQLStringVector& terms = dynamic_cast<const AQLDataStrings& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_EXPIRYTERMS, ISDEFINED).get()).get();
    return terms;
}

AQLStringVector LAMathParameterObject::ParameterMatrixIndexes(AQLDataInstance* dataInstance, const AQLString& matID)
{
    AQLObjectPool& objPool = dataInstance->getObjectPool();
    const AQLStringVector& indexes = dynamic_cast<const AQLDataStrings& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED ).get().getData(AQ_INDEXVECTOR, ISDEFINED).get()).get();
    return indexes;
}

AQLDate LAMathParameterObject::GetDate(const AQLDate& asOfDate, AQLString term, const AQLPriceDataSlidingRule& slr, const AQLPriceDataCalendar& cal)
{
    return CalendarAdvance(asOfDate, term, slr, cal);
}

double LAMathParameterObject::Lookup(AQLDate asOfDate, AQLDate expDate, const AQLString& idxPoint,
                                  const DoubleVector& expiryVec, const AQLStringVector& indexVec,
                                  const DoubleMatrix& dataMatrix, AQLString interpolationType)
{
    double expPoint_d = ModelTime(asOfDate, expDate);
    MFParameterMatrix parameterMatrix(dataMatrix, expiryVec, indexVec);
    return parameterMatrix.LookUp(expPoint_d, idxPoint, interpolationType);
}

void LAMathParameterObject::SetMatrixAxis(const AQLString& title, const AQLStringVector& terms, const AQLStringVector& indexes,
                                       AQLStringMatrix& matrix)
{
    size_t nTerms = terms.size();
    size_t nIndexes = indexes.size();
    matrix = AQLStringMatrix(nTerms + 1, AQLStringVector(nIndexes + 1));
    matrix[0][0] = title;
    for (size_t i = 0; i < nTerms; i++)
        matrix[1 + i][0] = terms[i];
    for (size_t j = 0; j < nIndexes; j++)
        matrix[0][1 + j] = indexes[j];
}

void LAMathParameterObject::SetMatrixData(const DoubleMatrix& data, AQLStringMatrix& matrix)
{
    size_t nRows = matrix.size();
    size_t nCols = matrix[0].size();
    if (data.size() != nRows - 1 || data[0].size() != nCols - 1)
        throw AQLCoreInvalidData("Invalid sizes in data matrix", __FILE__, __LINE__);

    for (size_t i = 0; i < data.size(); i++)
        for (size_t j = 0; j < data[0].size(); j++)
            matrix[1 + i][1 + j] = AQLString(n2s(data[i][j]).c_str());
}

MFParameterMatrix::MFParameterMatrix(const DoubleMatrix& mat_, const DoubleVector& mExpiries_, const AQLStringVector& mIndexes_)
: mMatrix(mat_), mExpiries(mExpiries_), mIndexes(mIndexes_)
{
}

double MFParameterMatrix::LookUp(double expiry, const AQLString& indexID, AQLString interpolationType)
{
    if(expiry < 0.0)
        throw AQLCoreInvalidData("Expiry is negative", __FILE__, __LINE__);

    size_t idx;
    if (!(TryFind(indexID, mIndexes, idx)))
        throw AQLCoreInvalidData(AQLString("Index " + indexID + " not found").getCString(), __FILE__, __LINE__);

    return LAMathInterpolationUtilities::interpolate(mExpiries, mMatrix[idx], expiry, etrading::toInterpolationEnum(interpolationType.c_str()));
}
