#ifndef HL_RES_REF_H_INCLUDED
#define HL_RES_REF_H_INCLUDED
#include "hl_text.h"
#include <utility>
#include <cassert>

namespace hl
{
struct res_base
{
    std::string name;

    res_base() noexcept = default;

    res_base(std::string name) noexcept :
        name(std::move(name)) {}
};

template<typename T>
class res_ref
{
protected:
    bool m_hasRes;
    union
    {
        std::string m_name;
        T* m_ptr;
    };

    inline void in_ensure_has_res() const
    {
        // NOTE: This function will be optimized out of Release builds.
        assert(m_hasRes &&
            "Attempted to retrieve the resource from a res_ref that doesn't have a resource! "
            "You can check if a res_ref has a resource in it by calling res_ref::has_res()");
    }

    void in_destroy() noexcept
    {
        if (!m_hasRes)
        {
            m_name.~basic_string();
        }
    }

public:
    inline bool has_res() const noexcept
    {
        return m_hasRes;
    }

    inline const std::string& name() const noexcept
    {
        return (m_hasRes) ? m_ptr->name : m_name;
    }

    inline std::string& name() noexcept
    {
        return (m_hasRes) ? m_ptr->name : m_name;
    }

    inline T& res() const noexcept
    {
        in_ensure_has_res();
        return *m_ptr;
    }

    inline T& operator*() const noexcept
    {
        in_ensure_has_res();
        return *m_ptr;
    }

    inline T* operator->() const noexcept
    {
        in_ensure_has_res();
        return m_ptr;
    }

    res_ref& operator=(const res_ref& other)
    {
        if (&other != this)
        {
            in_destroy();

            if (!other.m_hasRes)
            {
                new (&m_name) std::string(other.m_name);
            }
            else
            {
                m_ptr = other.m_ptr;
            }

            m_hasRes = other.m_hasRes;
        }

        return *this;
    }

    res_ref& operator=(res_ref&& other) noexcept
    {
        if (&other != this)
        {
            in_destroy();

            if (!other.m_hasRes)
            {
                new (&m_name) std::string(std::move(other.m_name));
            }
            else
            {
                m_ptr = other.m_ptr;
            }

            m_hasRes = other.m_hasRes;
        }

        return *this;
    }

    res_ref() noexcept :
        m_hasRes(false),
        m_name() {}

    res_ref(const char* name) :
        m_hasRes(false),
        m_name(name) {}

    res_ref(std::string name) noexcept :
        m_hasRes(false),
        m_name(std::move(name)) {}

    res_ref(T& res) noexcept :
        m_hasRes(true),
        m_ptr(&res) {}

    res_ref(const res_ref& other) :
        m_hasRes(other.m_hasRes)
    {
        if (!other.m_hasRes)
        {
            new (&m_name) std::string(other.m_name);
        }
        else
        {
            m_ptr = other.m_ptr;
        }
    }

    res_ref(res_ref&& other) noexcept :
        m_hasRes(other.m_hasRes)
    {
        if (!other.m_hasRes)
        {
            new (&m_name) std::string(std::move(other.m_name));
        }
        else
        {
            m_ptr = other.m_ptr;
        }
    }

    inline ~res_ref()
    {
        in_destroy();
    }
};

HL_API std::string get_res_name(const nchar* filePath);

HL_API std::string get_res_name(const nstring& filePath);
} // hl
#endif
