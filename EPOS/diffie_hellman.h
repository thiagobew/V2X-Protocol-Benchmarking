// EPOS Elliptic Curve Diffie-Hellman (ECDH) Component Declarations

#ifndef __diffie_hellman_h
#define __diffie_hellman_h

#include "bignum.h"
#include "cipher.h"
#include <ostream>

__BEGIN_SYS

class Diffie_Hellman
{
public:
	static const unsigned int SECRET_SIZE = Cipher::KEY_SIZE;
	static const unsigned int PUBLIC_KEY_SIZE = 2 * SECRET_SIZE;

private:
    typedef _UTIL::Bignum<SECRET_SIZE> Bignum;

	class Elliptic_Curve_Point
	{
    public:
        typedef Diffie_Hellman::Bignum Coordinate;

		Elliptic_Curve_Point() __attribute__((noinline)) { }
		Elliptic_Curve_Point(const Coordinate & _x, const Coordinate & _y) { x = _x; y = _y; z = 1; }

		void operator*=(const Coordinate & b);

		friend Debug &operator<<(Debug &out, const Elliptic_Curve_Point &a) {
			out << "{x=" << a.x << ",y=" << a.y << ",z=" << a.z << "}";
			return out;
		}

	private:
		void jacobian_double();
		void add_jacobian_affine(const Elliptic_Curve_Point &b);

    public:
        Coordinate x, y, z = 1; // z = 1 means affine coordinates
	};

public:
    typedef Elliptic_Curve_Point Public_Key;
    typedef Bignum Shared_Key;
    typedef Bignum Private_Key;

	Diffie_Hellman();
	Diffie_Hellman(const Elliptic_Curve_Point & base_point);
	Diffie_Hellman(const Elliptic_Curve_Point & public_key, const Bignum & private_key) : _public(public_key), _private(private_key) { }

	Elliptic_Curve_Point public_key() { return _public; }

	Shared_Key shared_key(Elliptic_Curve_Point public_key);
	static Shared_Key shared_key(Elliptic_Curve_Point public_key, Bignum priv_key);
	static bool is_valid_point(const Elliptic_Curve_Point& point);

private:
	void generate_keypair() {
		db<Diffie_Hellman>(TRC) << "Diffie_Hellman::generate_keypair()" << std::endl;

		_private.randomize();

		db<Diffie_Hellman>(INF) << "Diffie_Hellman Private: " << _private << std::endl;
		db<Diffie_Hellman>(INF) << "Diffie_Hellman Base Point: " << _base_point << std::endl;

		_public = _base_point;
		_public *= _private;

		db<Diffie_Hellman>(INF) << "Diffie_Hellman Public: " << _public << std::endl;
	}

private:
	Private_Key _private;
	Elliptic_Curve_Point _base_point;
	Elliptic_Curve_Point _public;
	static const unsigned char _default_base_point_x[SECRET_SIZE];
	static const unsigned char _default_base_point_y[SECRET_SIZE];

	static const unsigned char curve_b_buffer[SECRET_SIZE];
	
	static const unsigned char curve_p_buffer[SECRET_SIZE];
};

__END_SYS

#endif
