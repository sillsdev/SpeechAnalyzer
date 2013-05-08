// ZTransform.h: interface for the CZTransform class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ZTRANSFORM_H
#define ZTRANSFORM_H

class CProcess;

template<class _Ty>
class CZTransformGeneric
{
public:
    CZTransformGeneric();
    CZTransformGeneric(const CZTransformGeneric<_Ty> & value);
    CZTransformGeneric(const int32 nOrder, const _Ty * const numeratorCoef, const _Ty * const denominatorCoef);
    virtual ~CZTransformGeneric();

    CZTransformGeneric<_Ty> & operator *=(const CZTransformGeneric<_Ty> & value);
    CZTransformGeneric<_Ty> & operator =(const CZTransformGeneric<_Ty> & value);

    _Ty Tick(_Ty dInput);
    bool IsIdentity();

private:
    void SetTransform( const int32 nOrder, const _Ty * const numeratorCoef, const _Ty * const denominatorCoef);
    void GetTransform( int32 & nOrder, const _Ty * numeratorCoef, const _Ty * denominatorCoef);
    int32 GetOrder() const;

    _Ty GetNumerator(int32 nPower) const;
    _Ty GetDenominator(int32 nPower) const;

    // Implementation
public:
    int32 m_nOrder;
    _Ty * m_pNumerator;
    _Ty * m_pDenominator;
    _Ty * m_pState;

    std::vector<CZTransformGeneric<_Ty>> m_cStages;
};

template<class _Ty> CZTransformGeneric<_Ty>::CZTransformGeneric()
{
    m_pNumerator = NULL;
    m_pDenominator = NULL;
    m_pState = NULL;

    SetTransform(0, NULL, NULL);
}

/**
* copy constructor
*/
template<class _Ty> CZTransformGeneric<_Ty>::CZTransformGeneric(const CZTransformGeneric<_Ty> & value)
{
    m_pNumerator = NULL;
    m_pDenominator = NULL;
    m_pState = NULL;

    SetTransform(value.m_nOrder, value.m_pNumerator, value.m_pDenominator);
    m_cStages = value.m_cStages;
}

/**
* normal constructor
*/
template<class _Ty> CZTransformGeneric<_Ty>::CZTransformGeneric(const int32 nOrder, const _Ty * const numeratorCoef, const _Ty * const denominatorCoef)
{
    m_pNumerator = NULL;
    m_pDenominator = NULL;
    m_pState = NULL;
    SetTransform( nOrder, numeratorCoef, denominatorCoef);
}

template<class _Ty> CZTransformGeneric<_Ty>::~CZTransformGeneric()
{
    delete [] m_pNumerator;
    delete [] m_pDenominator;
    delete [] m_pState;
}

/**
* Creates the numerator and denominator arrays
* @param nOrder the length of the vector
*/
template<class _Ty> void CZTransformGeneric<_Ty>::SetTransform(const int32 nOrder, const _Ty * const numeratorCoef, const _Ty * const denominatorCoef)
{
    delete [] m_pNumerator;
    delete [] m_pDenominator;
    delete [] m_pState;

    m_nOrder = nOrder;

    m_pNumerator = new _Ty[m_nOrder+1];
    m_pDenominator = new _Ty[m_nOrder+1];
    m_pState = new _Ty[m_nOrder+1];

    for (int32 i = 0; i <= m_nOrder; i++)
    {
        if (numeratorCoef!=NULL)
        {
            m_pNumerator[i] = numeratorCoef[i];
        }
        else
        {
            m_pNumerator[i] = (i == 0) ? 1.0 : 0.0;
        }

        if (denominatorCoef!=NULL)
        {
            m_pDenominator[i] = denominatorCoef[i];
        }
        else
        {
            m_pDenominator[i] = (i == 0) ? 1.0 : 0.0;
        }

        m_pState[i] = 0.0;
    }

	ASSERT(m_pDenominator[0] == 1.0);	// The denominator zero lag coefficient must be 1

    m_cStages.clear();
}

template<class _Ty> CZTransformGeneric<_Ty> & CZTransformGeneric<_Ty>::operator *=(const CZTransformGeneric<_Ty> & value)
{
    if (m_nOrder + value.m_nOrder > 2)
    {
        m_cStages.push_back(value);
        return *this;
    }

    // preserve the original values
    _Ty * pNumerator = m_pNumerator;
    _Ty * pDenominator = m_pDenominator;
    _Ty * pState = m_pState;
    int32 nOldOrder = m_nOrder;

    m_nOrder += value.m_nOrder;
    m_pNumerator = new _Ty[m_nOrder+1];
    m_pDenominator = new _Ty[m_nOrder+1];
    m_pState = new _Ty[m_nOrder+1];

    for (int32 i=m_nOrder; i>=0; i--)
    {
        _Ty numerator = 0.;
        _Ty denominator = 0.;

        for (int32 j=0; j<=i; j++)
        {
            if (i-j > nOldOrder)
            {
                continue;
            }
            if (j > value.m_nOrder)
            {
                break;
            }
            numerator += pNumerator[i-j]*value.GetNumerator(j);
            denominator += pDenominator[i-j]*value.GetDenominator(j);
        }
        m_pNumerator[i] = numerator;
        m_pDenominator[i] = denominator;
        m_pState[i] = 0.0;

        // reduce the order of the filter
        if ((i == m_nOrder) && (numerator == 0.0) && (denominator == 0.0))
        {
            m_nOrder--;
        }
    }

    m_cStages.insert(m_cStages.end(), value.m_cStages.begin(), value.m_cStages.end());

    // these are the only references to this memory
    delete [] pNumerator;
    delete [] pDenominator;
    delete [] pState;

    return *this;
}

template<class _Ty> _Ty CZTransformGeneric<_Ty>::Tick( _Ty dInput)
{
    _Ty partialOutput = 0.0;
    _Ty partialState = dInput;

    for (int32 i = m_nOrder; i>0; i--)
    {
        partialOutput += m_pState[i]*m_pNumerator[i];
        partialState -= m_pState[i]*m_pDenominator[i];
    }

    partialOutput += partialState*m_pNumerator[0];

    // advance delay line
    for (int32 i = m_nOrder; i>1; i--)
    {
        m_pState[i] = m_pState[i-1];
    }

    if (m_nOrder)   // Special reduced case no state info
    {
        m_pState[1] = partialState;
    }

    for (int32 i=0; i<int32(m_cStages.size()); i++)
    {
        partialOutput = m_cStages[i].Tick(partialOutput);
    }

    return partialOutput;
}

template<class _Ty> bool CZTransformGeneric<_Ty>::IsIdentity()
{
    return (GetOrder() == 0) && (m_pNumerator[0] == 1.);
}

template<class _Ty> int32 CZTransformGeneric<_Ty>::GetOrder() const
{
    int32 nOrder = m_nOrder;

    for (int32 i=0; i<int32(m_cStages.size()); i++)
    {
        nOrder += m_cStages[i].GetOrder();
    }

    return nOrder;
}

template<class _Ty> CZTransformGeneric<_Ty> & CZTransformGeneric<_Ty>::operator =(const CZTransformGeneric<_Ty> & value)
{
    _Ty one = 1;
    SetTransform(0, &one, NULL);
    *this *= value;
    return *this;
}

template<class _Ty> _Ty CZTransformGeneric<_Ty>::GetNumerator(int32 nPower) const
{
    return (nPower > m_nOrder) ? 0 : m_pNumerator[nPower];
}

template<class _Ty> _Ty CZTransformGeneric<_Ty>::GetDenominator(int32 nPower) const
{
    return (nPower > m_nOrder) ? 0 : m_pDenominator[nPower];
}

typedef CZTransformGeneric<double> CZTransform;

#endif
