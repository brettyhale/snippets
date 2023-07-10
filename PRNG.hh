////////////////////////////////////////////////////////////////////////////////
//
// PRNG.hh : [GL0 library] PRNG (uniform random bit generator)
//
// Copyright (c) 2023 Brett Hale.
// SPDX-License-Identifier: BSD-2-Clause
//
////////////////////////////////////////////////////////////////////////////////


#ifndef GL0_PRNG_HH_
#define GL0_PRNG_HH_

#ifndef GL0_NAMESPACE_
#define GL0_NAMESPACE_
#define GL0_NAMESPACE_INIT_ namespace GL0 {
#define GL0_NAMESPACE_FINI_ }
#endif

#include <cstdint>
#include <array>

#include <random>
#include <limits> // std::numeric_limits<>::digits


GL0_NAMESPACE_INIT_


////////////////////////////////////////////////////////////////////////////////
//
// GL0::PRNG class:


class PRNG
{
public:

    // xoshiro128++ PRNG (satisfies: UniformRandomBitGenerator)

    // based on the 'xoshiro128plusplus.c' implementation by
    // David Blackman and Sebastiano Vigna: prng.di.unimi.it

    using result_type = std::uint_fast32_t;

    static constexpr result_type result_max =
        static_cast<result_type>(UINT32_C(0xffffffff));

    static constexpr result_type min () { return (0); }
    static constexpr result_type max () { return result_max; }


    std::array<result_type, 4> state; // (128 bits)

    static constexpr auto clamp (result_type u) {
        return (u & result_max); }

    static constexpr auto rol (result_type u, int k) {
        u = clamp(u); // idiomatic rotate:
        return clamp((u << (k & 31)) | (u >> ((- k) & 31)));
    }

    // xoshiro128++ generator function:

    result_type operator () () noexcept {
        auto r = rol(state[0] + state[3], (7)) + state[0];
        auto t = clamp(state[1] << (9));
        state[2] ^= state[0], state[3] ^= state[1];
        state[1] ^= state[2], state[0] ^= state[3];
        state[2] ^= t, state[3] = rol(state[3], (11));
        return clamp(r);
    }

    // equivalent to (2^64) calls to generator:

    constexpr void jump () noexcept;

    // equivalent to (2^96) calls to generator:

    constexpr void long_jump () noexcept;


    // github.com/skeeto/hash-prospector : triple32

    static constexpr auto mix (result_type u) {
        u = clamp(u);
        u = clamp((u ^ (u >> 17)) * UINT32_C(0xed5ad4bb));
        u = clamp((u ^ (u >> 11)) * UINT32_C(0xac4c1b51));
        u = clamp((u ^ (u >> 15)) * UINT32_C(0x31848bab));
        return (u ^ (u >> 14));
    }

    // (re)initialize the PRNG with a 32-bit seed:

    constexpr PRNG (result_type s = 0) noexcept
        : state {} { seed(s); }

    constexpr void seed (result_type s) noexcept {
        for (auto & si : state) // splitmix:
            si = mix(s = clamp(s + UINT32_C(0x9e3779b9)));
    }

    // reinitialize the PRNG with a 128 bit state IV, generated
    // using a std::random_device:

    void init (std::random_device &);

};


////////////////////////////////////////////////////////////////////////////////


GL0_NAMESPACE_FINI_


////////////////////////////////////////////////////////////////////////////////
//
// GL0::PRNG jump function implementations:


constexpr auto
jump (const GL0::PRNG & prng) noexcept
{
    auto jrng {prng}; jrng.jump(); return jrng;
}


constexpr void
GL0::PRNG::jump () noexcept
{
    decltype(state) jbuf {};

    constexpr decltype(state) jtab {
        0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b };

    for (auto j : jtab)
    {
        for (int b = 0; b < (32); operator () (), b++)
        {
            if (j & (UINT32_C(1) << b))
                for (unsigned int i = 0; i < jbuf.size(); i++)
                    jbuf[i] ^= state[i];
        }
    }

    state = jbuf;
}


constexpr auto
long_jump (const GL0::PRNG & prng) noexcept
{
    auto jrng {prng}; jrng.long_jump(); return jrng;
}


constexpr void
GL0::PRNG::long_jump () noexcept
{
    decltype(state) jbuf {};

    constexpr decltype(state) jtab {
        0xb523952e, 0x0b6f099f, 0xccf5a0ef, 0x1c580662 };

    for (auto j : jtab)
    {
        for (int b = 0; b < (32); operator () (), b++)
        {
            if (j & (UINT32_C(1) << b))
                for (unsigned int i = 0; i < jbuf.size(); i++)
                    jbuf[i] ^= state[i];
        }
    }

    state = jbuf;
}


////////////////////////////////////////////////////////////////////////////////
//
// GL0::PRNG init function implementations:


inline void
GL0::PRNG::init (std::random_device & rdev)
{
    decltype(state) rbuf;

    if (std::numeric_limits<decltype(rdev())>::digits < (32))
    {
        for (auto & ri : rbuf)
        {
            auto lo = static_cast<result_type>(rdev());
            auto hi = static_cast<result_type>(rdev());

            ri = clamp((hi << (16)) | (lo & (0xffff)));
        }
    }
    else
    {
        for (auto & ri : rbuf)
            ri = clamp(static_cast<result_type>(rdev()));
    }

    // note: it shouldn't be necessary to hash values generated
    // using a conforming std::random_device - furthermore, hash
    // functions are not bijective, excluding possible states.

    state = rbuf;
}


////////////////////////////////////////////////////////////////////////////////


#endif // GL0_PRNG_HH_
