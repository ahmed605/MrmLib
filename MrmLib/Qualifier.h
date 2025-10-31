#pragma once
#include "Qualifier.g.h"

#include <common/Base.h>
#include <common/file/MrmFiles.h>
#include <common/MrmProfileData.h>
#include <DefObject.h>
#include <Results.h>
#include <Atoms.h>
#include <Checksums.h>
#include <readers/MrmManagers.h>
#include <MrmEnvironment.h>
#include <Platform/Base.h>
#include <readers/MrmReaders.h>

#include <namespaces.h>

namespace winrt::MrmLib::implementation
{
    struct Qualifier : QualifierT<Qualifier>
    {
    private:
		mrm::QualifierResult m_qualifier;

    public:
        Qualifier() = default;
		Qualifier(mrm::QualifierResult&& qualifier);

        winrt::MrmLib::QualifierAttribute Attribute();
        winrt::MrmLib::QualifierOperator Operator();
        uint64_t CustomOperator();
        winrt::MrmLib::QualifierValueType ValueType();
        hstring StringValue();
        uint32_t AttributeValue();
        winrt::Windows::Foundation::IInspectable Value();
        int32_t Priority();
        double FallbackScore();
    };
}
