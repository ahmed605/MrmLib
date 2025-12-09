#include "pch.h"
#include "Qualifier.h"
#include "Qualifier.g.cpp"

namespace winrt::MrmLib::implementation
{
    Qualifier::Qualifier(mrm::QualifierResult&& qualifier, mrm::AtomPoolGroup* pPoolGroup, mrm::MrmPlatformVersionInternal version)
    {
        mrm::Atom attribute;
        check_hresult(qualifier.GetOperand1Attribute(&attribute));
        check_hresult(mrm::WindowsClientProfileBase::RemapQualifierIndexToCore(version, attribute, reinterpret_cast<mrm::CoreEnvironment::QualifierIndex*>(&m_attribute)));

        if (m_attribute == QualifierAttribute::Custom)
        {
            mrm::StringResult strResult;
            check_hresult(pPoolGroup->GetString(attribute, &strResult));
            auto result = strResult.GetStringResult();
            m_attributeName = result->cchBuf ? hstring(result->pRef, result->cchBuf - 1) : hstring(result->pRef);
        }
        else
        {
            m_attributeName = mrm::CoreEnvironment::QualifierNames[(int)m_attribute];
        }

        if (!qualifier.OperatorIsCustom()) [[likely]]
        {
            mrm::ICondition::ConditionOperator op = { };
            check_hresult(qualifier.GetOperator(&op));
            m_operator = static_cast<QualifierOperator>(op);
        }
        else [[unlikely]]
        {
            m_operator = QualifierOperator::Custom;

            mrm::Atom op;
            check_hresult(qualifier.GetCustomOperator(&op));

            mrm::StringResult strResult;
            check_hresult(pPoolGroup->GetString(op, &strResult));
            auto result = strResult.GetStringResult();
            m_customOperator = result->cchBuf ? hstring(result->pRef, result->cchBuf - 1) : hstring(result->pRef);
        }

        mrm::StringResult strResult;
        check_hresult(qualifier.GetOperand2Literal(&strResult));
        auto result = strResult.GetStringResult();
        m_stringValue = result->cchBuf ? hstring(result->pRef, result->cchBuf - 1) : hstring(result->pRef);

        m_priority = qualifier.GetPriority();

        double score = 0.0;
        check_hresult(qualifier.GetFallbackScore(&score));
        m_fallbackScore = score;
    }

    Qualifier::Qualifier(QualifierAttribute const& attribute, QualifierOperator const& _operator, hstring const& qualifierValue, int32_t priority, double fallbackScore)
    {
        m_attribute = attribute;
        m_attributeName = mrm::CoreEnvironment::QualifierNames[(int)m_attribute];
        m_operator = _operator;
        m_stringValue = qualifierValue;
        m_priority = priority;
        m_fallbackScore = fallbackScore;
    }

    winrt::MrmLib::Qualifier Qualifier::Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, winrt::MrmLib::QualifierOperator const& _operator, hstring const& qualifierValue, int32_t priority, double fallbackScore)
    {
        return winrt::make<implementation::Qualifier>(qualifierAttribute, _operator, qualifierValue, priority, fallbackScore);
    }

    winrt::MrmLib::Qualifier Qualifier::Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, winrt::MrmLib::QualifierOperator const& _operator, hstring const& qualifierValue, double fallbackScore)
    {
        return winrt::make<implementation::Qualifier>(qualifierAttribute, _operator, qualifierValue, 0, fallbackScore);
    }

    winrt::MrmLib::Qualifier Qualifier::Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, winrt::MrmLib::QualifierOperator const& _operator, hstring const& qualifierValue)
    {
        return winrt::make<implementation::Qualifier>(qualifierAttribute, _operator, qualifierValue, 0, 0.0);
    }

    winrt::MrmLib::Qualifier Qualifier::Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, hstring const& qualifierValue, int32_t priority, double fallbackScore)
    {
        return winrt::make<implementation::Qualifier>(qualifierAttribute, QualifierOperator::Match, qualifierValue, priority, fallbackScore);
    }

    winrt::MrmLib::Qualifier Qualifier::Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, hstring const& qualifierValue, double fallbackScore)
    {
        return winrt::make<implementation::Qualifier>(qualifierAttribute, QualifierOperator::Match, qualifierValue, 0, fallbackScore);
    }

    winrt::MrmLib::Qualifier Qualifier::Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, hstring const& qualifierValue)
    {
        return winrt::make<implementation::Qualifier>(qualifierAttribute, QualifierOperator::Match, qualifierValue, 0, 0.0);
    }

    winrt::MrmLib::QualifierAttribute Qualifier::Attribute()
    {
        /*mrm::Atom attribute;
        check_hresult(m_qualifier.GetOperand1Attribute(&attribute));

        return static_cast<winrt::MrmLib::QualifierAttribute>(attribute.GetUInt64());*/

        return m_attribute;
    }

    hstring Qualifier::AttributeName()
    {
        return m_attributeName;
    }

    winrt::MrmLib::QualifierOperator Qualifier::Operator()
    {
        /*if (m_qualifier.OperatorIsCustom())
            return QualifierOperator::Custom;

        mrm::ICondition::ConditionOperator op = { };
        check_hresult(m_qualifier.GetOperator(&op));
        return static_cast<winrt::MrmLib::QualifierOperator>(op);*/

        return m_operator;
    }

    hstring Qualifier::CustomOperator()
    {
        /*if (!m_qualifier.OperatorIsCustom())
            return 0;

        mrm::Atom op;
        check_hresult(m_qualifier.GetCustomOperator(&op));
        return static_cast<uint32_t>(op.GetUInt64());*/

        return m_customOperator;
    }

    hstring Qualifier::Value()
    {
        return m_stringValue;
    }

    int32_t Qualifier::Priority()
    {
        /* 
        return m_qualifier.GetPriority();
        */

        return m_priority;
    }

    double Qualifier::FallbackScore()
    {
        /*double score = 0.0;
        check_hresult(m_qualifier.GetFallbackScore(&score));
        return score;*/

        return m_fallbackScore;
    }
}
