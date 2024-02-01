////////////////////////////////////////////////////////////////////////////////
//
// X128PP.hh : xoshiro128++ (std::uniform_random_bit_generator)
//
// Copyright (c) 2024 Brett Hale.
// SPDX-License-Identifier: BSD-2-Clause
//
////////////////////////////////////////////////////////////////////////////////


#ifndef X128PP_HH_
#define X128PP_HH_


#include <cstdint>
#include <array>

#include <random>


////////////////////////////////////////////////////////////////////////////////
//
// X128PP class:


class X128PP
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

    static constexpr auto rol (result_type u, unsigned k) {
        u = clamp(u), k %= 32; // idiomatic rotate:
        return (k) ? clamp((u << k) | (u >> (32 - k))) : (u);
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

    // equivalent to (2^64) generator function calls:

    constexpr void jump () noexcept;

    // equivalent to (2^96) generator function calls:

    constexpr void long_jump () noexcept;


    // github.com/skeeto/hash-prospector : triple32

    static constexpr auto mix (result_type u) {
        u = clamp(u);
        u = clamp((u ^ (u >> 17)) * UINT32_C(0xed5ad4bb));
        u = clamp((u ^ (u >> 11)) * UINT32_C(0xac4c1b51));
        u = clamp((u ^ (u >> 15)) * UINT32_C(0x31848bab));
        return (u ^ (u >> 14));
    }

    // (re)initialize the PRNG with a 32 bit seed:

    constexpr X128PP (result_type seed = 0) noexcept
        : state {} { init(seed); }

    constexpr void init (result_type seed) noexcept {
        for (auto & si : state) // splitmix:
            si = mix(seed += UINT32_C(0x9e3779b9));
    }

    // (re)initialize the PRNG with a 128 bit state, generated
    // using a std::random_device:

    X128PP (std::random_device & rdev)
        : state {} { init(rdev); }

    void init (std::random_device & rdev) {
        decltype(state) rbuf;
        std::uniform_int_distribution<result_type> d {0, max()};
        for (auto & ri : rbuf) ri = d(rdev);
        state = rbuf;
    }
};


////////////////////////////////////////////////////////////////////////////////
//
// X128PP jump function implementations:


constexpr auto
jump (const X128PP & prng) noexcept
{
    auto jrng {prng}; jrng.jump(); return jrng;
}


constexpr void
X128PP::jump () noexcept
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
long_jump (const X128PP & prng) noexcept
{
    auto jrng {prng}; jrng.long_jump(); return jrng;
}


constexpr void
X128PP::long_jump () noexcept
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


#endif // X128PP_HH_
