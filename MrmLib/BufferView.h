#pragma once

#include <Windows.h>
#include <RoBuffer.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <wil/result.h>

namespace MrmLib
{
    // https://github.com/microsoft/winrtc/blob/master/WebRtcWrapper/webrtc.AudioTrackSink.cpp
    // https://github.com/microsoft/CsWinRT/blob/master/src/cswinrt/strings/additions/Windows.Storage.Streams/WindowsRuntimeBuffer.cs
    struct BufferView : ::winrt::implements<BufferView,
                                            ::winrt::Windows::Storage::Streams::IBuffer,
                                            ::Windows::Storage::Streams::IBufferByteAccess,
                                            ::winrt::Windows::Foundation::IClosable,
                                            IMarshal>
    {
    private:
        const uint8_t* data_;
        size_t capacity_;

        winrt::com_ptr<IMarshal> m_marshaler;

        HRESULT EnsureMarshaler()
        {
            if (!m_marshaler)
            {
                return RoGetBufferMarshaler(m_marshaler.put());
            }

            return S_OK;
        }

    public:
        BufferView(const uint8_t* data, size_t capacity)
            : data_(data),
              capacity_(capacity)
        {
        }

        ~BufferView() noexcept
        {
        }

        void Close() noexcept
        {
        }

        uint32_t Capacity() const noexcept
        {
            return static_cast<uint32_t>(capacity_);
        }

        uint32_t Length() const noexcept
        {
            return static_cast<uint32_t>(capacity_);
        }

        void Length(uint32_t value)
        {
            UNREFERENCED_PARAMETER(value);
            throw winrt::hresult_illegal_method_call(L"The buffer is read-only");
        }

        STDMETHODIMP Buffer(uint8_t** value) noexcept final
        {
            if (!value)
            {
                return E_POINTER;
            }

            *value = const_cast<uint8_t*>(data_);
            return S_OK;
        }

        STDMETHODIMP GetUnmarshalClass(REFIID riid, void* pv, DWORD dwDestContext, void* pvDestContext, DWORD mshlflags, CLSID* pclsid) noexcept final
        {
            RETURN_IF_FAILED(EnsureMarshaler());
            return m_marshaler->GetUnmarshalClass(riid, pv, dwDestContext, pvDestContext, mshlflags, pclsid);
        }

        STDMETHODIMP GetMarshalSizeMax(REFIID riid, void* pv, DWORD dwDestContext, void* pvDestContext, DWORD mshlflags, DWORD* pcbSize) noexcept final
        {
            RETURN_IF_FAILED(EnsureMarshaler());
            return m_marshaler->GetMarshalSizeMax(riid, pv, dwDestContext, pvDestContext, mshlflags, pcbSize);
        }

        STDMETHODIMP MarshalInterface(IStream* pStm, REFIID riid, void* pv, DWORD dwDestContext, void* pvDestContext, DWORD mshlflags) noexcept final
        {
            RETURN_IF_FAILED(EnsureMarshaler());
            return m_marshaler->MarshalInterface(pStm, riid, pv, dwDestContext, pvDestContext, mshlflags);
        }

        STDMETHODIMP UnmarshalInterface(IStream* pStm, REFIID riid, void** ppv) noexcept final
        {
            UNREFERENCED_PARAMETER(pStm);
            UNREFERENCED_PARAMETER(riid);
            UNREFERENCED_PARAMETER(ppv);
            return E_NOTIMPL;
        }

        STDMETHODIMP ReleaseMarshalData(IStream* pStm) noexcept final
        {
            UNREFERENCED_PARAMETER(pStm);
            return E_NOTIMPL;
        }

        STDMETHODIMP DisconnectObject(DWORD dwReserved) noexcept final
        {
            UNREFERENCED_PARAMETER(dwReserved);
            return E_NOTIMPL;
        }
    };

#ifdef ENABLE_BUFFER_OVER_ARRAY
    struct BufferOverArray : ::winrt::implements<BufferOverArray,
                                            ::winrt::Windows::Storage::Streams::IBuffer,
                                            ::Windows::Storage::Streams::IBufferByteAccess,
                                            ::winrt::Windows::Foundation::IClosable,
                                            IMarshal>
    {
    private:
        winrt::com_array<uint8_t> m_array;
        bool m_closed = false;

        winrt::com_ptr<IMarshal> m_marshaler;

        HRESULT EnsureMarshaler()
        {
            if (!m_marshaler)
            {
                return RoGetBufferMarshaler(m_marshaler.put());
            }

            return S_OK;
        }

    public:
        BufferOverArray(winrt::com_array<uint8_t>&& array)
            : m_array(std::move(array))
        {
        }

        ~BufferOverArray() noexcept
        {
        }

        void Close() noexcept
        {
            if (!m_closed)
            {
                m_closed = true;
                m_array = { };
            }
        }

        uint32_t Capacity() const noexcept
        {
            return static_cast<uint32_t>(m_array.size());
        }

        uint32_t Length() const noexcept
        {
            return static_cast<uint32_t>(m_array.size());
        }

        void Length(uint32_t value)
        {
            UNREFERENCED_PARAMETER(value);
            throw winrt::hresult_illegal_method_call(L"The buffer is read-only");
        }

        STDMETHODIMP Buffer(uint8_t** value) noexcept final
        {
            if (!value)
            {
                return E_POINTER;
            }

            *value = const_cast<uint8_t*>(m_array.begin());
            return S_OK;
        }

        STDMETHODIMP GetUnmarshalClass(REFIID riid, void* pv, DWORD dwDestContext, void* pvDestContext, DWORD mshlflags, CLSID* pclsid) noexcept final
        {
            RETURN_IF_FAILED(EnsureMarshaler());
            return m_marshaler->GetUnmarshalClass(riid, pv, dwDestContext, pvDestContext, mshlflags, pclsid);
        }

        STDMETHODIMP GetMarshalSizeMax(REFIID riid, void* pv, DWORD dwDestContext, void* pvDestContext, DWORD mshlflags, DWORD* pcbSize) noexcept final
        {
            RETURN_IF_FAILED(EnsureMarshaler());
            return m_marshaler->GetMarshalSizeMax(riid, pv, dwDestContext, pvDestContext, mshlflags, pcbSize);
        }

        STDMETHODIMP MarshalInterface(IStream* pStm, REFIID riid, void* pv, DWORD dwDestContext, void* pvDestContext, DWORD mshlflags) noexcept final
        {
            RETURN_IF_FAILED(EnsureMarshaler());
            return m_marshaler->MarshalInterface(pStm, riid, pv, dwDestContext, pvDestContext, mshlflags);
        }

        STDMETHODIMP UnmarshalInterface(IStream* pStm, REFIID riid, void** ppv) noexcept final
        {
            UNREFERENCED_PARAMETER(pStm);
            UNREFERENCED_PARAMETER(riid);
            UNREFERENCED_PARAMETER(ppv);
            return E_NOTIMPL;
        }

        STDMETHODIMP ReleaseMarshalData(IStream* pStm) noexcept final
        {
            UNREFERENCED_PARAMETER(pStm);
            return E_NOTIMPL;
        }

        STDMETHODIMP DisconnectObject(DWORD dwReserved) noexcept final
        {
            UNREFERENCED_PARAMETER(dwReserved);
            return E_NOTIMPL;
        }
    };
#endif // ENABLE_BUFFER_OVER_ARRAY
}