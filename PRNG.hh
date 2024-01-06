////////////////////////////////////////////////////////////////////////////////
//
// PRNG.hh : [GL0 library] (std::uniform_random_bit_generator)
//
// Copyright (c) 2024 Brett Hale.
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


GL0_NAMESPACE_INIT_


////////////////////////////////////////////////////////////////////////////////
//
// GL0::PRNG class:


class PRNG
{
public:

    // xoshiro256++ PRNG (satisfies: UniformRandomBitGenerator)

    // based on the 'xoshiro256plusplus.c' implementation by
    // David Blackman and Sebastiano Vigna: prng.di.unimi.it

    using result_type = std::uint_fast64_t;

    static constexpr result_type result_max =
        static_cast<result_type>(UINT64_C(0xffffffffffffffff));

    static constexpr result_type min () { return (0); }
    static constexpr result_type max () { return result_max; }


    std::array<result_type, 4> state; // (256 bits)

    static constexpr auto clamp (result_type u) {
        return (u & result_max); }

    static constexpr auto rol (result_type u, unsigned k) {
        u = clamp(u), k %= 64; // idiomatic rotate:
        return (k) ? clamp((u << k) | (u >> (64 - k))) : (u);
    }

    // xoshiro256++ generator function:

    result_type operator () () noexcept {
        auto r = rol(state[0] + state[3], (23)) + state[0];
        auto t = clamp(state[1] << (17));
        state[2] ^= state[0], state[3] ^= state[1];
        state[1] ^= state[2], state[0] ^= state[3];
        state[2] ^= t, state[3] = rol(state[3], (45));
        return clamp(r);
    }

    // equivalent to (2^128) calls to generator:

    constexpr void jump () noexcept;

    // equivalent to (2^192) calls to generator:

    constexpr void long_jump () noexcept;


    // Stafford's Mix13 variant:

    static constexpr auto mix (result_type u) {
        u = clamp(u); u ^= (u >> 30);
        u *= UINT64_C(0xbf58476d1ce4e5b9);
        u = clamp(u); u ^= (u >> 27);
        u *= UINT64_C(0x94d049bb133111eb);
        u = clamp(u); u ^= (u >> 31);
        return u;
    }

    // (re)initialize the PRNG with a 64 bit seed:

    constexpr PRNG (result_type seed = 0) noexcept
        : state {} { init(seed); }

    constexpr void init (result_type seed) noexcept {
        for (auto & si : state) // splitmix:
            si = mix(seed += UINT64_C(0x9e3779b97f4a7c15));
    }

    // (re)initialize the PRNG with a 256 bit state, generated
    // using a std::random_device:

    PRNG (std::random_device & rdev)
        : state {} { init(rdev); }

    void init (std::random_device & rdev) {
        decltype(state) rbuf;
        std::uniform_int_distribution<result_type> d {0, max()};
        for (auto & ri : rbuf) ri = d(rdev);
        state = rbuf;
    }
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
        UINT64_C(0x180ec6d33cfd0aba), UINT64_C(0xd5a61266f0c9392c),
        UINT64_C(0xa9582618e03fc9aa), UINT64_C(0x39abdc4529b1661c)
    };

    for (auto j : jtab)
    {
        for (int b = 0; b < (64); operator () (), b++)
        {
            if (j & (UINT64_C(1) << b))
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
        UINT64_C(0x76e15d3efefdcbbf), UINT64_C(0xc5004e441c522fb3),
        UINT64_C(0x77710069854ee241), UINT64_C(0x39109bb02acbe635)
    };

    for (auto j : jtab)
    {
        for (int b = 0; b < (64); operator () (), b++)
        {
            if (j & (UINT64_C(1) << b))
                for (unsigned int i = 0; i < jbuf.size(); i++)
                    jbuf[i] ^= state[i];
        }
    }

    state = jbuf;
}


////////////////////////////////////////////////////////////////////////////////


#endif // GL0_PRNG_HH_
