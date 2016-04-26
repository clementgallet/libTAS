/*
 * (c) 2015- Hourglass Resurrection Team
 * Hourglass Resurrection is licensed under GPL v2.
 * Refer to the file COPYING.txt in the project root.
 */

#ifndef LIBTAS_MANAGEDALLOCATOR_H_INCL
#define LIBTAS_MANAGEDALLOCATOR_H_INCL

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

    /*
     * Microsoft cannot follow standards with nothrow-declaring stuff... sigh...
     */
    __nothrow ManagedAllocator() {}
    __nothrow ManagedAllocator(const ManagedAllocator& alloc) {}
    template<class U>
    __nothrow ManagedAllocator(const ManagedAllocator<U>& alloc) {}
    ~ManagedAllocator() {}

    pointer address(reference x) const
    {
        ENTER();
        return &x;
    }
    const_pointer address(const_reference x) const
    {
        ENTER();
        return &x;
    }

    pointer allocate(size_type n, const_pointer hint = 0)
    {
        ENTER(n);
        static const UINT flags = MemoryManager::ALLOC_WRITE;
        return reinterpret_cast<pointer>(MemoryManager::Allocate(n * sizeof(value_type), flags));
    }
    void deallocate(pointer p, size_type n)
    {
        ENTER(p);
        MemoryManager::Deallocate(p);
    }

    size_type max_size() const
    {
        ENTER();
        /*
         * Copy Microsoft's own implementation of max_size.
         */
        return static_cast<size_type>(-1) / sizeof(value_type);
    }

    template<class U, class... Args>
    void construct(U* p, Args&&... args)
    {
        ENTER(p);
        /*
         * Placement-new, will not attempt to allocate space.
         */
        ::new (reinterpret_cast<LPVOID>(p)) U(std::forward<Args>(args)...);
    }
    template<class U>
    void destroy(U* p)
    {
        ENTER(p);
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

template<class key, class value, class compare = std::less<key>>
using SafeMap = std::map<key, value, compare, ManagedAllocator<std::pair<const key, value>>>;

template<class key, class compare = std::less<key>>
using SafeSet = std::set<key, compare, ManagedAllocator<key>>;

template<class value>
using SafeVector = std::vector<value, ManagedAllocator<value>>;

#endif

