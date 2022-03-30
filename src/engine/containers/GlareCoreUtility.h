#ifndef _GLARE_UTILITY_H_
#define _GLARE_UTILITY_H_

#include <utility>  // std::pair
#include <assert.h>
#include <cstring>
#include <vector>


#define GLARE_MEMCPY std::memcpy
#define GLARE_MEMSET std::memset

// Assert -----------------------------------------------------------------------------------------
#ifndef GLARE_FINAL
    #define GLARE_ASSERT(_COND, _MSG) do { if(!(_COND)) { printf(_MSG); assert(false); } } while(0)
#else
    #define GLARE_ASSERT(_COND, _MSG)
#endif
// Assert -----------------------------------------------------------------------------------------

// Inline -----------------------------------------------------------------------------------------
#if defined(GLARE_FINAL)
    #define GLARE_INLINE inline
#else
    #if defined(WIN32)
        #define GLARE_INLINE __forceinline
    #elif defined(__GNUG__)
        #define GLARE_INLINE __attribute__((always_inline))
    #else
        #define GLARE_INLINE inline
    #endif
#endif // GLARE_FINAL
// Inline -----------------------------------------------------------------------------------------

// No Inline --------------------------------------------------------------------------------------
#if defined(GLARE_FINAL)
    #define GLARE_NO_INLINE
#else
    #if defined(WIN32)
        #define GLARE_NO_INLINE __declspec(noinline)
    #elif defined(__GNUG__)
        #define GLARE_NO_INLINE __attribute__ ((noinline))
    #else
        #define GLARE_NO_INLINE
    #endif
#endif // GLARE_FINAL
// No Inline --------------------------------------------------------------------------------------

#define GLARE_PAIR                  std::pair
#define GLARE_VECTOR                std::vector
//#define GLARE_LOG(_str, ...)        std::printf(_str##"\n")
#define GLARE_LOG                   std::printf


namespace glare
{
    // Courtesy of std::xfunctional //////////////////////////////////////
    // base class for unary functions
    template<class _Arg, class _Result>
    struct unary_function
    {
        typedef _Arg argument_type;
        typedef _Result result_type;
    };

    // base class for binary functions
    template<class _Arg1, class _Arg2, class _Result>
    struct binary_function
    {
        typedef _Arg1 first_argument_type;
        typedef _Arg2 second_argument_type;
        typedef _Result result_type;
    };

    // functor for operator+
    template<class _Ty>
    struct plus : public binary_function<_Ty, _Ty, _Ty>
    {
        _Ty operator()(const _Ty& _Left, const _Ty& _Right) const
        {   // apply operator+ to operands
            return (_Left + _Right);
        }
    };

    // functor for operator-
    template<class _Ty>
    struct minus : public binary_function<_Ty, _Ty, _Ty>
    {
        _Ty operator()(const _Ty& _Left, const _Ty& _Right) const
        {   // apply operator- to operands
            return (_Left - _Right);
        }
    };

    // functor for operator*
    template<class _Ty>
    struct multiplies : public binary_function<_Ty, _Ty, _Ty>
    {
        _Ty operator()(const _Ty& _Left, const _Ty& _Right) const
        {   // apply operator* to operands
            return (_Left * _Right);
        }
    };

    // functor for operator/
    template<class _Ty>
    struct divides : public binary_function<_Ty, _Ty, _Ty>
    {
        _Ty operator()(const _Ty& _Left, const _Ty& _Right) const
        {   // apply operator/ to operands
            return (_Left / _Right);
        }
    };

    // functor for operator%
    template<class _Ty>
    struct modulus : public binary_function<_Ty, _Ty, _Ty>
    {
        _Ty operator()(const _Ty& _Left, const _Ty& _Right) const
        {   // apply operator% to operands
            return (_Left % _Right);
        }
    };

    // functor for unary operator-
    template<class _Ty>
    struct negate : public unary_function<_Ty, _Ty>
    {
        _Ty operator()(const _Ty& _Left) const
        {   // apply operator- to operand
            return (-_Left);
        }
    };

    // functor for operator==
    template<class _Ty>
    struct equal_to : public binary_function<_Ty, _Ty, bool>
    {
        bool operator()(const _Ty& _Left, const _Ty& _Right) const
        {   // apply operator== to operands
            return (_Left == _Right);
        }
    };

    // functor for operator!=
    template<class _Ty>
    struct not_equal_to : public binary_function<_Ty, _Ty, bool>
    {
        bool operator()(const _Ty& _Left, const _Ty& _Right) const
        {   // apply operator!= to operands
            return (_Left != _Right);
        }
    };

    // functor for operator>
    template<class _Ty>
    struct greater : public binary_function<_Ty, _Ty, bool>
    {
        bool operator()(const _Ty& _Left, const _Ty& _Right) const
        {   // apply operator> to operands
            return (_Left > _Right);
        }
    };

    // functor for operator<
    template<class _Ty>
    struct less : public binary_function<_Ty, _Ty, bool>
    {
        bool operator()(const _Ty& _Left, const _Ty& _Right) const
        { // apply operator< to operands
            return (_Left < _Right);
        }
    };

    // functor for operator>=
    template<class _Ty>
    struct greater_equal : public binary_function<_Ty, _Ty, bool>
    {
        bool operator()(const _Ty& _Left, const _Ty& _Right) const
        {   // apply operator>= to operands
            return (_Left >= _Right);
        }
    };

    // functor for operator<=
    template<class _Ty>
    struct less_equal : public binary_function<_Ty, _Ty, bool>
    {
        bool operator()(const _Ty& _Left, const _Ty& _Right) const
        {   // apply operator<= to operands
            return (_Left <= _Right);
        }
    };

    // functor for operator&&
    template<class _Ty>
    struct logical_and : public binary_function<_Ty, _Ty, bool>
    {
        bool operator()(const _Ty& _Left, const _Ty& _Right) const
        {   // apply operator&& to operands
            return (_Left && _Right);
        }
    };

    // functor for operator||
    template<class _Ty>
    struct logical_or : public binary_function<_Ty, _Ty, bool>
    {
        bool operator()(const _Ty& _Left, const _Ty& _Right) const
        {   // apply operator|| to operands
            return (_Left || _Right);
        }
    };

    // functor for unary operator!
    template<class _Ty>
    struct logical_not : public unary_function<_Ty, bool>
    {
        bool operator()(const _Ty& _Left) const
        {   // apply operator! to operand
            return (!_Left);
        }
    };

} // End of namespace.
#endif
