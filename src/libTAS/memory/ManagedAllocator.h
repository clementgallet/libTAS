/*
 * (c) 2015- Hourglass Resurrection Team
 * Hourglass Resurrection is licensed under GPL v2.
 * Refer to the file COPYING.txt in the project root.
 */

#ifndef LIBTAS_MANAGEDALLOCATOR_H_INCL
#define LIBTAS_MANAGEDALLOCATOR_H_INCL

#include "MemoryManager.h"
#include <map>
#include <set>
#include <vector>
#include <string>
#include "../logging.h"

template<class T>
class ManagedAllocator
{
    public:
        typedef T               value_type;
        typedef T*              pointer;
        typedef const T*        const_pointer;
        typedef T&              reference;
        typedef const T&        const_reference;
        typedef std::size_t     size_type;
        typedef std::ptrdiff_t  difference_type;

        template<class U>
            struct rebind
            {
                typedef ManagedAllocator<U> other;
            };

        
           ManagedAllocator() {}
           ManagedAllocator(const ManagedAllocator& alloc) {}
           template<class U>
           ManagedAllocator(const ManagedAllocator<U>& alloc) {}
           ~ManagedAllocator() {}
           
        pointer address(reference x) const
        {
            return &x;
        }
        const_pointer address(const_reference x) const
        {
            return &x;
        }

        pointer allocate(size_type n, const_pointer hint = 0)
        {
            debuglogstdio(LCF_MEMORY, "%s call with length %d and sizeof %d", __func__, n, sizeof(value_type));
            static const int flags = MemoryManager::ALLOC_WRITE;
            return reinterpret_cast<pointer>(memorymanager.allocate(n * sizeof(value_type), flags));
        }
        void deallocate(pointer p, size_type n)
        {
            debuglogstdio(LCF_MEMORY, "%s call with pointer %p, length %d and sizeof %d", __func__, p, n, sizeof(value_type));
            memorymanager.deallocate(p);
        }

        size_type max_size() const
        {
            /*
             * Copy Microsoft's own implementation of max_size.
             */
            return static_cast<size_type>(-1) / sizeof(value_type);
        }

        template<class U, class... Args>
            void construct(U* p, Args&&... args)
            {
                /*
                 * Placement-new, will not attempt to allocate space.
                 */
                ::new (reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...);
            }
        template<class U>
            void destroy(U* p)
            {
                p->~U();
            }
};

/*
 * Comparators for the ManagedAllocator.
 * since it's a stateless allocator, they're equal by default.
 */
template<class T1, class T2>
bool operator==(const ManagedAllocator<T1>&, const ManagedAllocator<T2>&)
{
    return true;
}

template<class T1, class T2>
bool operator!=(const ManagedAllocator<T1>&, const ManagedAllocator<T2>&)
{
    return false;
}

/*
 * Safe containers, using the regular std containers bare is not allowed.
 */
namespace safe {
    template<class key, class value, class compare = std::less<key>>
        using map = std::map<key, value, compare, ManagedAllocator<std::pair<const key, value>>>;

    template<class key, class compare = std::less<key>>
        using set = std::set<key, compare, ManagedAllocator<key>>;

    template<class value>
        using vector = std::vector<value, ManagedAllocator<value>>;

    using string = std::basic_string<char, std::char_traits<char>, ManagedAllocator<char>>;
}

#endif

