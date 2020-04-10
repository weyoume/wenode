#include <node/protocol/x11.hpp>
#include <fc/exception/exception.hpp>
#include <fc/crypto/hmac.hpp>
#include <fc/fwd_impl.hpp>

namespace node { namespace protocol {

	static void shift_l( const uint8_t* in, uint8_t* out, std::size_t n, unsigned int i) 
	{
        if (i < n) {
            memcpy( out, in + i, n-i );
        } else {
            i = n;
        }
        memset( out + (n-i), 0, i );
    }

    void shift_l( const char* in, char* out, std::size_t n, unsigned int i)
	{
        const uint8_t* in8 = (uint8_t*) in;
        uint8_t* out8 = (uint8_t*) out;

        if (i >= 8) {
            shift_l( in8, out8, n, i / 8 );
            i &= 7;
            in8 = out8;
        }

        std::size_t p;
        for( p = 0; p < n-1; ++p )
            out8[p] = (in8[p] << i) | (in8[p+1]>>(8-i));
        out8[p] = in8[p] << i;
    }

    static void shift_r( const uint8_t* in, uint8_t* out, std::size_t n, unsigned int i) 
	{
        if (i < n) {
            memcpy( out+i, in, n-i );
        } else {
            i = n;
        }
        memset( out, 0, i );
    }

    void shift_r( const char* in, char* out, std::size_t n, unsigned int i) 
	{
        const uint8_t* in8 = (uint8_t*) in;
        uint8_t* out8 = (uint8_t*) out;

        if (i >= 8) {
            shift_r( in8, out8, n, i / 8 );
            i &= 7;
            in8 = out8;
        }

        std::size_t p;
        for( p = n-1; p > 0; --p )
            out8[p] = (in8[p] >> i) | (in8[p-1]<<(8-i));
        out8[p] = in8[p] >> i;
    }


	x11::x11() { memset(_hash, 0, sizeof(_hash)); }

	x11::x11(const char *data, size_t size)
	{
		if (size != sizeof(_hash))
        {
            FC_THROW_EXCEPTION( fc::exception, "x11: size mismatch");
        }
			
		memcpy(_hash, data, size);
	}

	x11::x11(const string &hex_str)
	{
		fc::from_hex(hex_str, (char *)_hash, sizeof(_hash));
	}

	string x11::str() const
	{
		return fc::to_hex((char *)_hash, sizeof(_hash));
	}

	x11::operator string() const { return str(); }

	char *x11::data() const { return (char *)&_hash[0]; }

	struct x11::encoder::impl
	{
		x11_CTX ctx;
	};

	x11::encoder::~encoder() {}
	x11::encoder::encoder()
	{
		reset();
	}

	x11 x11::hash( const char *d, uint32_t dlen )
	{
		encoder e;
		e.write(d, dlen);
		return e.result();
	}

	x11 x11::hash( const string &s )
	{
		return hash( s.c_str(), s.size() );
	}

	x11 x11::hash( const x11 &s )
	{
		return hash( s.data(), sizeof(s._hash) );
	}

	void x11::encoder::write( const char *d, uint32_t dlen )
	{
		my->ctx.hash.reserve(11);
		static const char* pblank;

		sph_blake512( &my->ctx.ctx_blake, ( dlen == 0 ? pblank : static_cast<const void*>( &d[0])), dlen * sizeof(d[0]) );
		sph_blake512_close( &my->ctx.ctx_blake, static_cast<void*>( &my->ctx.hash[0]));

		sph_bmw512( &my->ctx.ctx_bmw, static_cast<const void*>( &my->ctx.hash[0] ), 64 );
		sph_bmw512_close( &my->ctx.ctx_bmw, static_cast<void*>( &my->ctx.hash[1] ) );

		sph_groestl512( &my->ctx.ctx_groestl, static_cast<const void*>( &my->ctx.hash[1]), 64 );
		sph_groestl512_close( &my->ctx.ctx_groestl, static_cast<void*>( &my->ctx.hash[2]));

		sph_skein512( &my->ctx.ctx_skein, static_cast<const void*>( &my->ctx.hash[2]), 64 );
		sph_skein512_close( &my->ctx.ctx_skein, static_cast<void*>( &my->ctx.hash[3]));

		sph_jh512( &my->ctx.ctx_jh, static_cast<const void*>( &my->ctx.hash[3]), 64 );
		sph_jh512_close( &my->ctx.ctx_jh, static_cast<void*>( &my->ctx.hash[4]));

		sph_keccak512( &my->ctx.ctx_keccak, static_cast<const void*>( &my->ctx.hash[4]), 64 );
		sph_keccak512_close( &my->ctx.ctx_keccak, static_cast<void*>( &my->ctx.hash[5]));

		sph_luffa512( &my->ctx.ctx_luffa, static_cast<void*>( &my->ctx.hash[5]), 64 );
		sph_luffa512_close( &my->ctx.ctx_luffa, static_cast<void*>( &my->ctx.hash[6]));

		sph_cubehash512( &my->ctx.ctx_cubehash, static_cast<const void*>( &my->ctx.hash[6]), 64 );
		sph_cubehash512_close( &my->ctx.ctx_cubehash, static_cast<void*>( &my->ctx.hash[7]));

		sph_shavite512( &my->ctx.ctx_shavite, static_cast<const void*>( &my->ctx.hash[7]), 64 );
		sph_shavite512_close( &my->ctx.ctx_shavite, static_cast<void*>( &my->ctx.hash[8]));

		sph_simd512( &my->ctx.ctx_simd, static_cast<const void*>( &my->ctx.hash[8]), 64 );
		sph_simd512_close( &my->ctx.ctx_simd, static_cast<void*>( &my->ctx.hash[9]));

		sph_echo512( &my->ctx.ctx_echo, static_cast<const void*>( &my->ctx.hash[9]), 64 );
		sph_echo512_close( &my->ctx.ctx_echo, static_cast<void*>( &my->ctx.hash[10]));
	}

	x11 x11::encoder::result()
	{
		x11 h;
		memcpy( h.data(), &my->ctx.hash[10], 32 );
		return h;
	}

	void x11::encoder::reset()
	{
		memset( &my->ctx, char(0), sizeof(my->ctx) );

		sph_blake512_init( &my->ctx.ctx_blake );
		sph_bmw512_init( &my->ctx.ctx_bmw );
		sph_groestl512_init( &my->ctx.ctx_groestl );
		sph_skein512_init( &my->ctx.ctx_skein );
		sph_jh512_init( &my->ctx.ctx_jh );
		sph_keccak512_init( &my->ctx.ctx_keccak );
		sph_luffa512_init( &my->ctx.ctx_luffa );
		sph_cubehash512_init( &my->ctx.ctx_cubehash );
		sph_shavite512_init( &my->ctx.ctx_shavite );
		sph_simd512_init( &my->ctx.ctx_simd );
		sph_echo512_init( &my->ctx.ctx_echo );
	}

	x11 operator<<(const x11 &h1, uint32_t i)
	{
		x11 result;
		shift_l(h1.data(), result.data(), result.data_size(), i);
		return result;
	}
	x11 operator>>(const x11 &h1, uint32_t i)
	{
		x11 result;
		shift_r(h1.data(), result.data(), result.data_size(), i);
		return result;
	}
	x11 operator^(const x11 &h1, const x11 &h2)
	{
		x11 result;
		result._hash[0] = h1._hash[0] ^ h2._hash[0];
		result._hash[1] = h1._hash[1] ^ h2._hash[1];
		result._hash[2] = h1._hash[2] ^ h2._hash[2];
		result._hash[3] = h1._hash[3] ^ h2._hash[3];
		return result;
	}
	bool operator>=(const x11 &h1, const x11 &h2)
	{
		return memcmp(h1._hash, h2._hash, sizeof(h1._hash)) >= 0;
	}
	bool operator>(const x11 &h1, const x11 &h2)
	{
		return memcmp(h1._hash, h2._hash, sizeof(h1._hash)) > 0;
	}
	bool operator<(const x11 &h1, const x11 &h2)
	{
		return memcmp(h1._hash, h2._hash, sizeof(h1._hash)) < 0;
	}
	bool operator!=(const x11 &h1, const x11 &h2)
	{
		return memcmp(h1._hash, h2._hash, sizeof(h1._hash)) != 0;
	}
	bool operator==(const x11 &h1, const x11 &h2)
	{
		return memcmp(h1._hash, h2._hash, sizeof(h1._hash)) == 0;
	}

	uint32_t x11::approx_log_32() const
	{
		uint16_t lzbits = clz();
		if (lzbits >= 0x100)
			return 0;
		uint8_t nzbits = 0xFF - lzbits;
		size_t offset = (size_t)(lzbits >> 3);
		uint8_t *my_bytes = (uint8_t *)data();
		size_t n = data_size();
		uint32_t y = (uint32_t(my_bytes[offset]) << 0x18) | (uint32_t(offset + 1 < n ? my_bytes[offset + 1] : 0) << 0x10) | (uint32_t(offset + 2 < n ? my_bytes[offset + 2] : 0) << 0x08) | (uint32_t(offset + 3 < n ? my_bytes[offset + 3] : 0));
		//
		// lzbits&7 == 7 : 00000001 iff nzbits&7 == 0
		// lzbits&7 == 6 : 0000001x iff nzbits&7 == 1
		// lzbits&7 == 5 : 000001xx iff nzbits&7 == 2
		//
		y >>= (nzbits & 7);
		y ^= 1 << 0x18;
		y |= uint32_t(nzbits) << 0x18;
		return y;
	}

	void x11::set_to_inverse_approx_log_32(uint32_t x)
	{
		uint8_t nzbits = uint8_t(x >> 0x18);
		_hash[0] = 0;
		_hash[1] = 0;
		_hash[2] = 0;
		_hash[3] = 0;
		if (nzbits == 0)
			return;
		uint8_t x0 = uint8_t((x)&0xFF);
		uint8_t x1 = uint8_t((x >> 0x08) & 0xFF);
		uint8_t x2 = uint8_t((x >> 0x10) & 0xFF);
		uint8_t *my_bytes = (uint8_t *)data();
		my_bytes[0x1F] = x0;
		my_bytes[0x1E] = x1;
		my_bytes[0x1D] = x2;
		my_bytes[0x1C] = 1;

		if (nzbits <= 0x18)
		{
			(*this) = (*this) >> (0x18 - nzbits);
		}
		else
			(*this) = (*this) << (nzbits - 0x18);
		return;
	}

	double x11::inverse_approx_log_32_double(uint32_t x)
	{
		uint8_t nzbits = uint8_t(x >> 0x18);
		if (nzbits == 0)
			return 0.0;
		uint32_t b = 1 << 0x18;
		uint32_t y = (x & (b - 1)) | b;
		return std::ldexp(y, int(nzbits) - 0x18);
	}

	uint16_t x11::clz() const
	{
		const uint8_t *my_bytes = (uint8_t *)data();
		size_t size = data_size();
		size_t lzbits = 0;
		static const uint8_t char2lzbits[] = {
				// 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
				8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
				2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

		size_t i = 0;

		while (true)
		{
			uint8_t c = my_bytes[i];
			lzbits += char2lzbits[c];
			if (c != 0)
				break;
			++i;
			if (i >= size)
				return 0x100;
		}

		return lzbits;
	}

	uint64_t x11::hash64( const char *buf, size_t len )
	{
		x11 sha_value = x11::hash( buf, len );
		return sha_value._hash[0];
	}

	

} }  // node::protocol

namespace fc {

	void to_variant(const node::protocol::x11 &bi, variant &v)
	{
		v = std::vector<char>((const char *)&bi, ((const char *)&bi) + sizeof(bi));
	}
	void from_variant(const fc::variant &v, node::protocol::x11 &bi)
	{
		std::vector<char> ve = v.as<std::vector<char>>();
		if (ve.size())
		{
			memcpy( &bi, ve.data(), fc::min<size_t>(ve.size(), sizeof(bi)));
		}
		else
			memset( &bi, char(0), sizeof(bi));
	}

	template <>
	unsigned int fc::hmac<node::protocol::x11>::internal_block_size() const { return 64; }
	
}  // fc