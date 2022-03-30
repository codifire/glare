#ifndef GLARE_ALLOCATORS
#define GLARE_ALLOCATORS

#include <memory>
#include <limits>

//#define GLARE_USE_STD_ALLOCATOR

namespace glare
{
    //std::allocator<>
    template<typename T>
    class Allocator
    {
    public:
        typedef T value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

    public:
        template<typename U>
        struct rebind{
            typedef Allocator<U> other;
        };

    public:
        inline explicit Allocator() {}
        inline ~Allocator() {}
        inline Allocator(Allocator const&) {}

        template<typename U>
        inline explicit Allocator(Allocator<U> const&) {}

        inline pointer address(reference r) { return &r; }
        inline const_pointer address(const_pointer r) { return &r; }

        // Either 0 or a value previously obtained by another call to allocate and not yet freed with deallocate.
        // When it is not 0, this value may be used as a hint to improve performance by allocating the new block 
        // near the one specified. The address of an adjacent element is often a good choice.
        inline pointer allocate(size_type cnt, std::allocator<void>::const_pointer hint=0)
        {
            return 
            #if defined(GLARE_USE_STD_ALLOCATOR)
                std::allocator<value_type>().allocate(cnt, hint);
            #else
                // Notice that calling this function directly does not construct an object. Allocates memory by calling: 
                // operator new (sizeof(MyClass)) 
                // but does not call MyClass's constructor.
                reinterpret_cast<pointer>(::operator new(cnt * sizeof(T)));
            #endif
        }

        // n: Number of elements allocated on the call to allocator::allocate for this block of storage.
        inline void deallocate(pointer p, size_type n)
        {
            #if defined(GLARE_USE_STD_ALLOCATOR)
                std::allocator<value_type>().deallocate(p, n);
            #else
                ::operator delete(p);
            #endif
        }

        inline void construct(pointer p)
        {
            new (p) T;
        }

        inline void construct(pointer p, const_reference o)
        {
            new (p) T(o);
        }

        template<typename Other>
        inline void construct(pointer p, Other& ref)
        {
            new (p) T(ref);
        }

        inline void destroy(pointer p) { p->~T(); }

        inline bool operator==(Allocator const&) {return true;}
        inline bool operator!=(Allocator const& a) {return !operator==(a);}

        //    size
        inline size_type max_size() const { 
            return std::numeric_limits<size_type>::max() / sizeof(T);
        }
    };
} // namespace

#define default_allocator glare::Allocator
#endif
