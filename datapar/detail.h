/*  This file is part of the Vc library. {{{
Copyright © 2016 Matthias Kretz <kretz@kde.org>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the names of contributing organizations nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

}}}*/

#ifndef VC_DATAPAR_DETAIL_H_
#define VC_DATAPAR_DETAIL_H_

#include "macros.h"
#include "flags.h"
#include "type_traits.h"

namespace Vc_VERSIONED_NAMESPACE::detail
{
// integer type aliases{{{1
using uchar = unsigned char;
using schar = signed char;
using ushort = unsigned short;
using uint = unsigned int;
using ulong = unsigned long;
using llong = long long;
using ullong = unsigned long long;

// equal_int_type{{{1
/**
 * \internal
 * Type trait to find the equivalent integer type given a(n) (un)signed long type.
 */
template <class T, size_t = sizeof(T)> struct equal_int_type;
template <> struct equal_int_type< long, 4> { using type =    int; };
template <> struct equal_int_type< long, 8> { using type =  llong; };
template <> struct equal_int_type<ulong, 4> { using type =   uint; };
template <> struct equal_int_type<ulong, 8> { using type = ullong; };
template <class T> using equal_int_type_t = typename equal_int_type<T>::type;

// unused{{{1
template <class T> static constexpr void unused(T && ) {}

// execute_on_index_sequence{{{1
template <typename F, size_t... I>
void execute_on_index_sequence(F && f, std::index_sequence<I...>)
{
    auto &&x = {(f(I), 0)...};
    unused(x);
}

template <typename R, typename F, size_t... I>
R execute_on_index_sequence_with_return(F && f, std::index_sequence<I...>)
{
    return R{f(I)...};
}

// execute_n_times{{{1
template <size_t N, typename F> void execute_n_times(F && f)
{
    execute_on_index_sequence(std::forward<F>(f), std::make_index_sequence<N>{});
}

// generate_from_n_evaluations{{{1
template <size_t N, typename R, typename F> R generate_from_n_evaluations(F && f)
{
    return execute_on_index_sequence_with_return<R>(std::forward<F>(f),
                                                    std::make_index_sequence<N>{});
}

// may_alias{{{1
template <typename T> struct may_alias_impl {
    typedef T type Vc_MAY_ALIAS;
};
/**\internal
 * Helper may_alias<T> that turns T into the type to be used for an aliasing pointer. This
 * adds the may_alias attribute to T (with compilers that support it). But for MaskBool this
 * attribute is already part of the type and applying it a second times leads to warnings/errors,
 * therefore MaskBool is simply forwarded as is.
 */
#ifdef Vc_ICC
template <typename T> using may_alias [[gnu::may_alias]] = T;
#else
template <typename T> using may_alias = typename may_alias_impl<T>::type;
#endif

    // traits forward declaration{{{1
    /**
     * \internal
     * Defines the implementation of a given <T, Abi>.
     */
template <class T, class Abi> struct traits;

// get_impl(_t){{{1
template <class T> struct get_impl;
template <class T> using get_impl_t = typename get_impl<T>::type;

// next_power_of_2{{{1
/**
 * \internal
 * Returns the next power of 2 larger than or equal to \p x.
 */
static constexpr std::size_t next_power_of_2(std::size_t x)
{
    return (x & (x - 1)) == 0 ? x : next_power_of_2((x | (x >> 1)) + 1);
}

// private_init{{{1
/**
 * \internal
 * Tag used for private init constructor of datapar and mask
 */
static constexpr struct private_init_t {} private_init = {};

// (cmp_)return_type{{{1
template <class L, class R> struct return_type_impl;
template <class L> struct return_type_impl<L, L> {
    using type = L;
};

template <class L, class R> using return_type = typename return_type_impl<L, R>::type;
template <class L, class R> using cmp_return_type = typename return_type<L, R>::mask_type;

// is_aligned(_v){{{1
template <class Flag, size_t Alignment> struct is_aligned;
template <size_t Alignment>
struct is_aligned<flags::vector_aligned_tag, Alignment> : public std::true_type {
};
template <size_t Alignment>
struct is_aligned<flags::element_aligned_tag, Alignment> : public std::false_type {
};
template <size_t GivenAlignment, size_t Alignment>
struct is_aligned<flags::overaligned_tag<GivenAlignment>, Alignment>
    : public std::integral_constant<bool, (GivenAlignment >= Alignment)> {
};
template <class Flag, size_t Alignment>
constexpr bool is_aligned_v = is_aligned<Flag, Alignment>::value;

// when_(un)aligned{{{1
/**
 * \internal
 * Implicitly converts from flags that specify alignment
 */
template <size_t Alignment>
class when_aligned
{
public:
    constexpr when_aligned(flags::vector_aligned_tag) {}
    template <size_t Given, class = std::enable_if_t<(Given >= Alignment)>>
    constexpr when_aligned(flags::overaligned_tag<Given>)
    {
    }
};
template <size_t Alignment>
class when_unaligned
{
public:
    constexpr when_unaligned(flags::element_aligned_tag) {}
    template <size_t Given, class = std::enable_if_t<(Given < Alignment)>>
    constexpr when_unaligned(flags::overaligned_tag<Given>)
    {
    }
};

//}}}1
}  // namespace Vc_VERSIONED_NAMESPACE::detail
#endif  // VC_DATAPAR_DETAIL_H_

// vim: foldmethod=marker
