#pragma once

#include <node/protocol/types.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/string.hpp>
#include <fc/platform_independence.hpp>
#include <fc/fwd.hpp>
#include <fc/io/raw_fwd.hpp>
#include <fc/log/logger.hpp>

#include <node/protocol/sph_blake.h>
#include <node/protocol/sph_bmw.h>
#include <node/protocol/sph_groestl.h>
#include <node/protocol/sph_jh.h>
#include <node/protocol/sph_keccak.h>
#include <node/protocol/sph_skein.h>
#include <node/protocol/sph_luffa.h>
#include <node/protocol/sph_cubehash.h>
#include <node/protocol/sph_shavite.h>
#include <node/protocol/sph_simd.h>
#include <node/protocol/sph_echo.h>

#include <string.h>
#include <cmath>
#include <fc/variant.hpp>

namespace node { namespace protocol {

    typedef struct x11state_st 
    {
        sph_blake512_context     ctx_blake;
		sph_bmw512_context       ctx_bmw;
		sph_groestl512_context   ctx_groestl;
		sph_jh512_context        ctx_jh;
		sph_keccak512_context    ctx_keccak;
		sph_skein512_context     ctx_skein;
		sph_luffa512_context     ctx_luffa;
		sph_cubehash512_context  ctx_cubehash;
		sph_shavite512_context   ctx_shavite;
		sph_simd512_context      ctx_simd;
		sph_echo512_context      ctx_echo;
		vector< uint512_t >      hash;

    } x11_CTX;


    void shift_l( const char* in, char* out, std::size_t n, unsigned int i);
    void shift_r( const char* in, char* out, std::size_t n, unsigned int i);

    /**
	 * Finds the X11 Hash of an input data value.
     * 
     * X11 Hash Algorithm completes 11 consecutive 512 Bit Hashes
     * on input data in the following order:
     * 
     * 1 - Blake
     * 2 - BMW
     * 3 - Groestl
     * 4 - JH
     * 5 - Keccak
     * 6 - Skein
     * 7 - Luffa
     * 8 - CubeHash
     * 9 - Shavite
     * 10 - Simd
     * 11 - Echo
     * 
	 * X11 Hash Design derived from Dash Core Codebase.
	 * https://github.com/dashpay/dash
	 */
    class x11 
    {
    public:
        x11();
        explicit x11( const string& hex_str );
        explicit x11( uint64_t a, uint64_t b, uint64_t c, uint64_t d );
        explicit x11( uint128_t a );
        explicit x11( const char *data, size_t size );

        string str()const;
        operator string()const;

        char*    data()const;
        size_t data_size()const { return 256 / 8; }

        static x11 max_value()
        {
            const uint64_t max64 = std::numeric_limits<uint64_t>::max();
            return x11( max64, max64, max64, max64 );
        }

        static x11 hash( const char* d, uint32_t dlen );
        static x11 hash( const string& );
        static x11 hash( const x11& );

        template<typename T>
        static x11 hash( const T& t ) 
        { 
            x11::encoder e; 
            fc::raw::pack(e,t);
            return e.result(); 
        } 

        class encoder 
        {
        public:
            encoder();
            ~encoder();

            void write( const char* d, uint32_t dlen );
            void put( char c ) { write( &c, 1 ); }
            void reset();
            x11 result();

        private:
            struct                   impl;

            fc::fwd< impl, 2664 >    my;
        };

        template<typename T>
        inline friend T& operator<<( T& ds, const x11& ep ) 
        {
            ds.write( ep.data(), sizeof(ep) );
            return ds;
        }

        template<typename T>
        inline friend T& operator>>( T& ds, x11& ep ) 
        {
            ds.read( ep.data(), sizeof(ep) );
            return ds;
        }
        friend x11    operator << ( const x11& h1, uint32_t i       );
        friend x11    operator >> ( const x11& h1, uint32_t i       );
        friend bool   operator == ( const x11& h1, const x11& h2 );
        friend bool   operator != ( const x11& h1, const x11& h2 );
        friend x11    operator ^  ( const x11& h1, const x11& h2 );
        friend bool   operator >= ( const x11& h1, const x11& h2 );
        friend bool   operator >  ( const x11& h1, const x11& h2 ); 
        friend bool   operator <  ( const x11& h1, const x11& h2 ); 

        uint32_t pop_count()const
        {
        return (uint32_t)(__builtin_popcountll(_hash[0]) +
                            __builtin_popcountll(_hash[1]) +
                            __builtin_popcountll(_hash[2]) +
                            __builtin_popcountll(_hash[3])); 
        }

        /**
         * Count leading zero bits
         */
        uint16_t clz()const;

        /**
         * Approximate (log_2(x) + 1) * 2**24.
         *
         * Detailed specs:
         * - Return 0 when x == 0.
         * - High 8 bits of result simply counts nonzero bits.
         * - Low 24 bits of result are the 24 bits of input immediately after the most significant 1 in the input.
         * - If above would require reading beyond the end of the input, zeros are used instead.
         */
        uint32_t approx_log_32()const;

        void set_to_inverse_approx_log_32( uint32_t x );
        static double inverse_approx_log_32_double( uint32_t x );

        uint64_t hash64(const char* buf, size_t len);

        uint128_t to_uint128()const;

        uint64_t _hash[4];
    };

} }  // node:protocol

namespace fc {

    void to_variant( const node::protocol::x11& bi, fc::variant& v );

    void from_variant( const fc::variant& v, node::protocol::x11& bi );
}

namespace std {

    template<>
    struct hash< node::protocol::x11 >
    {
        size_t operator()( const node::protocol::x11& s )const
        {
            return *((size_t*)&s);
        }
    };
}

namespace boost {

    template<>
    struct hash< node::protocol::x11 >
    {
        size_t operator()( const node::protocol::x11& s )const
        {
            return *((size_t*)&s);
        }
    };
}

#include <fc/reflect/reflect.hpp>

FC_REFLECT_TYPENAME( node::protocol::x11 )
FC_REFLECT_TYPENAME( node::protocol::x11_CTX )