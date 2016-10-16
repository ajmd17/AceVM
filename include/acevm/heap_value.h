#ifndef HEAP_VALUE_H
#define HEAP_VALUE_H

#include <type_traits>
#include <typeinfo>
#include <cstdint>

enum HeapValueFlags {
    GC_MARKED = 0x01,
};

class HeapValue {
public:
    HeapValue();
    HeapValue(const HeapValue &other) = delete;
    ~HeapValue();

    inline bool operator==(const HeapValue &other) const 
    { 
        if (!((intptr_t)m_holder ^ (intptr_t)other.m_holder) && m_holder != nullptr) {
            return (*m_holder) == (*other.m_holder);
        }
        
        return false;
    }

    inline intptr_t GetId() const { return (intptr_t)m_ptr; }

    inline int &GetFlags() { return m_flags; }
    inline int GetFlags() const { return m_flags; }

    template <typename T>
    inline void Assign(const T &value)
    {
        typedef typename std::decay<T>::type U;

        if (m_holder != nullptr) {
            delete m_holder;
        }

        auto holder = new DerivedHolder<U>(value);
        m_ptr = reinterpret_cast<void*>(&holder->m_value);
        m_holder = holder;
    }

    template <typename T>
    inline bool TypeCompatible() const
    {
        typedef typename std::decay<T>::type U;

        return m_holder->TypeInfo() == typeid(U);
    }

    template <typename T>
    inline T &Get()
    {
        typedef typename std::decay<T>::type U;

        if (!TypeCompatible<T>()) {
            throw std::bad_cast();
        }

        return *reinterpret_cast<U*>(m_ptr);
    }

    template <typename T>
    inline const T &Get() const
    {
        typedef typename std::decay<T>::type U;

        if (!TypeCompatible<T>()) {
            throw std::bad_cast();
        }
        
        return *reinterpret_cast<const U*>(m_ptr);
    }

    inline bool IsNull() const
    {
        return m_holder == nullptr;
    }

    inline const std::type_info &TypeInfo() const
    {
        return m_holder->TypeInfo();
    }

private:
    // base class for an 'any' holder with pure virtual functions
    struct BaseHolder {
        virtual ~BaseHolder() = default;
        virtual bool operator==(const BaseHolder &other) const = 0;
        virtual const std::type_info &TypeInfo() const = 0;
    };

    // derived class that can hold any time
    template <typename T>
    struct DerivedHolder : public BaseHolder {
        explicit DerivedHolder(const T &value)
            : m_value(value)
        {
        }

        virtual bool operator==(const BaseHolder &other) const
        {
            const auto *other_casted = dynamic_cast<const DerivedHolder<T>*>(&other);
            return (other_casted != nullptr && 
                other_casted->m_value == m_value);
        }

        virtual const std::type_info &TypeInfo() const
        {
            return typeid(m_value);
        }

        T m_value;
    };

    BaseHolder *m_holder;
    void *m_ptr;
    int m_flags;
};

#endif