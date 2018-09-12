// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_MEMORY_H_INCLUDED__
#define __IRR_MEMORY_H_INCLUDED__

#include "irr/core/math/irrMath.h"
#include "irr/void_t.h"
#include <typeinfo>
#include <cstddef>


#define _IRR_SIMD_ALIGNMENT                 16u // change to 32 or 64 for AVX or AVX2 compatibility respectively, might break BaW file format!
//! Default alignment for a type
#define _IRR_DEFAULT_ALIGNMENT(_obj_type)   (std::alignment_of<_obj_type>::value>(_IRR_SIMD_ALIGNMENT) ? std::alignment_of<_obj_type>::value:(_IRR_SIMD_ALIGNMENT))

#define _IRR_MIN_MAP_BUFFER_ALIGNMENT       64u// GL_MIN_MAP_BUFFER_ALIGNMENT


//! Very useful for enabling compiler optimizations
#if defined(_MSC_VER)
    #define _IRR_ASSUME_ALIGNED(ptr, alignment) \
    __assume((reinterpret_cast<size_t>(ptr) & ((alignment) - 1)) == 0)
#elif (__GNUC__ * 100 + __GNUC_MINOR__) >= 407 // ||(CLANG&&__has_builtin(__builtin_assume_aligned))
    #define _IRR_ASSUME_ALIGNED(ptr, alignment) \
    (ptr) = static_cast<decltype(ptr)>(__builtin_assume_aligned((ptr), (alignment)))
#else
    #define _IRR_ASSUME_ALIGNED(ptr,alignment)
#endif

//! Utility so we don't have to write out _IRR_ASSUME_ALIGNED(ptr,_IRR_SIMD_ALIGNMENT) constantly
#define _IRR_ASSUME_SIMD_ALIGNED(ptr) _IRR_ASSUME_ALIGNED(ptr,_IRR_SIMD_ALIGNMENT)


//! You can swap these out for whatever you like, jemalloc, tcmalloc etc. but make them noexcept
#ifdef _IRR_WINDOWS_
    #define _IRR_ALIGNED_MALLOC(size,alignment)     ::_aligned_malloc(size,alignment)
    #define _IRR_ALIGNED_FREE(addr)                 ::_aligned_free(addr)
#else

namespace irr
{
namespace impl
{
    inline void* aligned_malloc(size_t size, size_t alignment)
    {
        if (size == 0) return nullptr;
        void* p;
        if (::posix_memalign(&p, alignment<alignof(std::max_align_t) ? alignof(std::max_align_t):alignment, size) != 0) p = nullptr;
        return p;
    }
}
}
    #define _IRR_ALIGNED_MALLOC(size,alignment)     irr::impl::aligned_malloc(size,alignment)
    #define _IRR_ALIGNED_FREE(addr)                 ::free(addr)
#endif


//! TODO: FINAL Allow overrides of Global New and Delete ???
#ifdef _IRR_ALLOW_GLOBAL_NEW_TO_THROW
#else
#endif

//TOTO Now: Create a irr::AllocatedByStaticAllocator<StaticAllocator> class
//TOTO Now: Create a irr::AllocatedByDynamicAllocation class with a static function new[] like operator that takes an DynamicAllocator* parameter


//! TODO: Implement StaticAllocator<ALIGN> that respects custom alignment with boost::align and progress the defines
#define _IRR_ALLOCATE_W_ALLOCATOR //incomplete
#define _IRR_FREE_W_ALLOCATOR //incomplete

//! TODO: Inline function (with template) or macro?
#define _IRR_NEW_ALIGNED_W_ALLOCATOR(_obj_type,_align,_static_allocator)                new _obj_type //incomplete
#define _IRR_DELETE_ALIGNED_W_ALLOCATOR(_obj,_static_allocator)                         delete _obj //incomplete

#define _IRR_NEW_ALIGNED_ARRAY_W_ALLOCATOR(_obj_type,count,_align,_static_allocator)    new _obj_type[count] //incomplete
#define _IRR_DELETE_ALIGNED_ARRAY_W_ALLOCATOR(_obj,_static_allocator)                   delete [] _obj //incomplete


#define _IRR_DEFAULT_ALLOCATOR //!put std:: global alloc allocators //incomplete
#define _IRR_NEW_ALIGNED(_obj_type,_align)                                              _IRR_NEW_ALIGNED_W_ALLOCATOR(_obj_type,_align,_IRR_DEFAULT_ALLOCATOR)
#define _IRR_DELETE_ALIGNED(_obj)                                                       _IRR_DELETE_ALIGNED_W_ALLOCATOR(_obj,_IRR_DEFAULT_ALLOCATOR)

#define _IRR_NEW_ALIGNED_ARRAY(_obj_type,count,_align)                                  _IRR_NEW_ALIGNED_ARRAY_W_ALLOCATOR(_obj_type,count,_align,_IRR_DEFAULT_ALLOCATOR)
#define _IRR_DELETE_ALIGNED_ARRAY(_obj)                                                 _IRR_DELETE_ALIGNED_ARRAY_W_ALLOCATOR(_obj,_IRR_DEFAULT_ALLOCATOR)

//use these by default instead of new and delete
#define _IRR_NEW(_obj_type)                                                             _IRR_NEW_ALIGNED(_obj_type,_IRR_DEFAULT_ALIGNMENT(_obj_type))
#define _IRR_DELETE(_obj)                                                               _IRR_DELETE_ALIGNED(_obj)

#define _IRR_NEW_ARRAY(_obj_type,count)                                                 _IRR_NEW_ALIGNED_ARRAY(_obj_type,count,_IRR_DEFAULT_ALIGNMENT(_obj_type))
#define _IRR_DELETE_ARRAY(_obj)                                                         _IRR_DELETE_ALIGNED_ARRAY(_obj)

//! Extra Utility Macros for when you don't want to always have to deduce the alignment but want to use a specific allocator
#define _IRR_NEW_W_ALLOCATOR(_obj_type,_static_allocator)                               _IRR_NEW_ALIGNED_W_ALLOCATOR(_obj_type,_IRR_DEFAULT_ALIGNMENT(_obj_type),_static_allocator)
#define _IRR_DELETE_W_ALLOCATOR(_obj,_static_allocator)                                 _IRR_DELETE_ALIGNED_W_ALLOCATOR(_obj,_static_allocator)

#define _IRR_NEW_ARRAY_W_ALLOCATOR(_obj_type,_static_allocator)                         _IRR_NEW_ALIGNED_ARRAY_W_ALLOCATOR(_obj_type,_IRR_DEFAULT_ALIGNMENT(_obj_type),_static_allocator)
#define _IRR_DELETE_ARRAY_W_ALLOCATOR(_obj,_static_allocator)                           _IRR_DELETE_ALIGNED_ARRAY_W_ALLOCATOR(_obj,_static_allocator)


namespace irr
{

//! Alignments can only be PoT in C++11 and in GPU APIs, so this is useful if you need to pad
constexpr inline size_t alignUp(size_t value, size_t alignment)
{
    return (value + alignment - 1ull) & ~(alignment - 1ull);
}

//! Down-rounding counterpart
constexpr inline size_t alignDown(size_t value, size_t alignment)
{
    return (value - 1ull) & ~(alignment - 1ull);
}

//! Valid alignments are power of two
constexpr inline bool is_alignment(size_t value)
{
    return core::isPoT(value);
}


//! Special Class For providing deletion for things like C++11 smart-pointers
struct alligned_delete
{
    template<class T>
    void operator()(T* ptr) const noexcept(noexcept(ptr->~T()))
    {
        if (ptr)
        {
            ptr->~T();
            _IRR_ALIGNED_FREE(ptr);
        }
    }
};

//! Inherit from this class if you want to make sure all the derivations are aligned.
/** Any class derived from this, even indirectly will be declared as aligned to
`object_alignment`, unfortunately this is only enforced for objects on the stack.
To make sure your object is aligned on heaps as well you need to inherit from
`AlignedAllocOverrideBase` or one of its aliases instead. **/
template<size_t object_alignment=_IRR_SIMD_ALIGNMENT>
class IRR_FORCE_EBO alignas(object_alignment) AlignedBase
{
    static_assert(is_alignment(object_alignment),"Alignments must be PoT and positive!");
    static_assert(object_alignment<=128,"Pending migration to GCC 7+ highest alignment on c++ class is 128 bytes");
};

namespace impl
{
    //! Variadic template class for metaprogrammatically resolving max alignment resulting from multiple inheritance of AlignedBase. PLEASE DO NOT USE ANYWHERE ELSE.
    template <class... Ts>
    class IRR_FORCE_EBO ResolveAlignment
    {
    };

    //! Specialization of ResolveAlignment for recursively resolving the alignment of many types.
    template <class T, class... Ts>
    class IRR_FORCE_EBO ResolveAlignment<T, Ts...> :  public T
    {
        private:
            struct DummyForConditional {typedef void most_aligned_type;};
            static_assert(std::alignment_of<typename DummyForConditional::most_aligned_type>::value<=1,"Why is void not aligned to 1 or 0 in your compiler?");

            //! In this section we get the type with maximum alignment for the N-1 recursion.
            constexpr static bool isNextRecursionEmpty = std::is_same<ResolveAlignment<Ts...>,ResolveAlignment<> >::value;
            typedef typename std::conditional<isNextRecursionEmpty,DummyForConditional,ResolveAlignment<Ts...> >::type::most_aligned_type otherType;

            //! Bool telling is whether our current type, `T`, is larger than the max type of the recursion.
            constexpr static bool isTLargestType = std::alignment_of<T>::value>std::alignment_of<otherType>::value;
        public:
            //! The maximally aligned type for this recursion of N template parameters
            typedef typename std::conditional<isTLargestType,T,otherType>::type most_aligned_type;
        private:
            //! Fallback to use in-case most_aligned_type does not declare its own new and delete (fallback is a specialization of this class for single element).
            using DefaultAlignedAllocationOverriden = ResolveAlignment<AlignedBase<std::alignment_of<most_aligned_type>::value> >;

            //! Some meta-functions to allow us for static checking of metaprogrammatically derived most_aligned_type
            template<class U> using operator_new_t                  = decltype(U::operator new(0ull));
            template<class U> using operator_new_array_t            = decltype(U::operator new[](0ull));
            template<class U> using operator_delete_t               = decltype(U::operator delete(nullptr));
            template<class U> using operator_delete_array_t         = decltype(U::operator delete[](nullptr));
            template<class U> using operator_delete_w_size_t        = decltype(U::operator delete(nullptr,0ull));
            template<class U> using operator_delete_array_w_size_t  = decltype(U::operator delete[](nullptr,0ull));

            template<class,class=void> struct has_new_operator                  : std::false_type {};
            template<class,class=void> struct has_new_array_operator            : std::false_type {};
            template<class,class=void> struct has_delete_operator               : std::false_type {};
            template<class,class=void> struct has_delete_array_operator         : std::false_type {};
            template<class,class=void> struct has_delete_operator_w_size        : std::false_type {};
            template<class,class=void> struct has_delete_array_operator_w_size  : std::false_type {};
            template<class U> struct has_new_operator<U,void_t<operator_new_t<U> > >                                    : std::is_same<operator_new_t<U>,void*> {};
            template<class U> struct has_new_array_operator<U,void_t<operator_new_array_t<U> > >                        : std::is_same<operator_new_array_t<U>,void*> {};
            template<class U> struct has_delete_operator<U,void_t<operator_delete_t<U> > >                              : std::is_same<operator_delete_t<U>,void> {};
            template<class U> struct has_delete_array_operator<U,void_t<operator_delete_array_t<U> > >                  : std::is_same<operator_delete_array_t<U>,void> {};
            template<class U> struct has_delete_operator_w_size<U,void_t<operator_delete_w_size_t<U> > >                : std::is_same<operator_delete_w_size_t<U>,void> {};
            template<class U> struct has_delete_array_operator_w_size<U,void_t<operator_delete_array_w_size_t<U> > >    : std::is_same<operator_delete_array_w_size_t<U>,void> {};
        public:
            /** Now we could override the new and delete operators always with the same thing, and allocate aligned to `std::alignment_of<most_aligned_type>::value`,
            however we want to call the most aligned class' new and delete operators (if such exist) so its overrides actually matter.
            **/
            inline static void* operator new(size_t size) noexcept
            {
                return std::conditional<has_new_operator<most_aligned_type>::value,most_aligned_type,DefaultAlignedAllocationOverriden>::type::operator new(size);
            }
            static inline void* operator new[](size_t size) noexcept
            {
                return std::conditional<has_new_array_operator<most_aligned_type>::value,most_aligned_type,DefaultAlignedAllocationOverriden>::type::operator new[](size);
            }

            static inline void operator delete(void* ptr) noexcept
            {
                std::conditional<has_delete_operator<most_aligned_type>::value,most_aligned_type,DefaultAlignedAllocationOverriden>::type::operator delete(ptr);
            }
            static inline void operator delete[](void* ptr) noexcept
            {
                std::conditional<has_delete_array_operator<most_aligned_type>::value,most_aligned_type,DefaultAlignedAllocationOverriden>::type::operator delete[](ptr);
            }
            static inline void operator delete(void* ptr, size_t size) noexcept
            {
                std::conditional<has_delete_operator_w_size<most_aligned_type>::value,most_aligned_type,DefaultAlignedAllocationOverriden>::type::operator delete(ptr,size);
            }
            static inline void operator delete[](void* ptr, size_t size) noexcept
            {
                std::conditional<has_delete_array_operator_w_size<most_aligned_type>::value,most_aligned_type,DefaultAlignedAllocationOverriden>::type::operator delete[](ptr,size);
            }

    };

    //! Specialization for the base case of a single parameter whose type is AlignedBase (needed to specify default new and delete)
    /** Note regarding C++17, we don't overload the alignment on the versions with std::align_val_t.
    Why? Because we want to respect and not f-up the explicitly requested alignment. **/
    template <size_t object_alignment>
    class IRR_FORCE_EBO ResolveAlignment<AlignedBase<object_alignment> > :  public AlignedBase<object_alignment>
    {
        public:
            //! The maximally aligned type for this recursion of N template parameters
            typedef AlignedBase<object_alignment> most_aligned_type;
        private:
            //
        public:
            inline static void* operator new(size_t size) noexcept
            {
                //std::cout << "Alloc aligned to " << object_alignment << std::endl;
                return _IRR_ALIGNED_MALLOC(size,object_alignment);
            }
            static inline void* operator new[](size_t size) noexcept
            {
                //std::cout << "Alloc aligned to " << object_alignment << std::endl;
                return _IRR_ALIGNED_MALLOC(size,object_alignment);
            }

            static inline void operator delete(void* ptr) noexcept
            {
                //std::cout << "Delete aligned to " << object_alignment << std::endl;
                _IRR_ALIGNED_FREE(ptr);
            }
            static inline void  operator delete[](void* ptr) noexcept
            {
                //std::cout << "Delete aligned to " << object_alignment << std::endl;
                _IRR_ALIGNED_FREE(ptr);
            }
            static inline void operator delete(void* ptr, size_t size) noexcept {operator delete(ptr);} //roll back to own operator with no size
            static inline void operator delete[](void* ptr, size_t size) noexcept {operator delete[](ptr);} //roll back to own operator with no size

    };
}


//! This is a base class for overriding default memory management.
template <size_t _in_alignment>
using AllocationOverrideBase = impl::ResolveAlignment<AlignedBase<_in_alignment> >;

using AllocationOverrideDefault = AllocationOverrideBase<_IRR_SIMD_ALIGNMENT>;


//! Put in class if the compiler is complaining about ambiguous references to new and delete operators. Needs to be placed in the public section of methods
#define _IRR_RESOLVE_NEW_DELETE_AMBIGUITY(...) \
            static inline void* operator new(size_t size)                noexcept {return (impl::ResolveAlignment<__VA_ARGS__>::operator new(size));} \
            static inline void* operator new[](size_t size)              noexcept {return impl::ResolveAlignment<__VA_ARGS__>::operator new[](size);} \
            static inline void operator delete(void* ptr)                noexcept {impl::ResolveAlignment<__VA_ARGS__>::operator delete(ptr);} \
            static inline void operator delete[](void* ptr)              noexcept {impl::ResolveAlignment<__VA_ARGS__>::operator delete[](ptr);} \
            static inline void operator delete(void* ptr, size_t size)   noexcept {impl::ResolveAlignment<__VA_ARGS__>::operator delete(ptr,size);} \
            static inline void operator delete[](void* ptr, size_t size) noexcept {impl::ResolveAlignment<__VA_ARGS__>::operator delete[](ptr,size);}

}


#endif // __IRR_MACROS_H_INCLUDED__