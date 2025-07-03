// EPOS Elliptic Curve Diffie-Hellman (ECDH) Component Implementation

#include "diffie_hellman.h"

__BEGIN_SYS

// Class attributes
//TODO: base point is dependent of SECRET_SIZE
const unsigned char Diffie_Hellman::_default_base_point_x[SECRET_SIZE] =
{
    (unsigned char)'\x86', (unsigned char)'\x5B', (unsigned char)'\x2C', (unsigned char)'\xA5',
    (unsigned char)'\x7C', (unsigned char)'\x60', (unsigned char)'\x28', (unsigned char)'\x0C',
    (unsigned char)'\x2D', (unsigned char)'\x9B', (unsigned char)'\x89', (unsigned char)'\x8B',
    (unsigned char)'\x52', (unsigned char)'\xF7', (unsigned char)'\x1F', (unsigned char)'\x16'
};

const unsigned char Diffie_Hellman::_default_base_point_y[SECRET_SIZE] =
{
    (unsigned char)'\x83', (unsigned char)'\x7A', (unsigned char)'\xED', (unsigned char)'\xDD',
    (unsigned char)'\x92', (unsigned char)'\xA2', (unsigned char)'\x2D', (unsigned char)'\xC0',
    (unsigned char)'\x13', (unsigned char)'\xEB', (unsigned char)'\xAF', (unsigned char)'\x5B',
    (unsigned char)'\x39', (unsigned char)'\xC8', (unsigned char)'\x5A', (unsigned char)'\xCF'
};


const unsigned char Diffie_Hellman::curve_p_buffer[SECRET_SIZE] =
{
    (unsigned char)'\xFF', (unsigned char)'\xFF', (unsigned char)'\xFF', (unsigned char)'\xFF', // 0xFF 0xFF 0xFF 0xFF = 0xFFFFFFFF (decimal: 4294967295)
    (unsigned char)'\xFF', (unsigned char)'\xFF', (unsigned char)'\xFF', (unsigned char)'\xFF', // 0xFF 0xFF 0xFF 0xFF = 0xFFFFFFFF (decimal: 4294967295)
    (unsigned char)'\xFF', (unsigned char)'\xFF', (unsigned char)'\xFF', (unsigned char)'\xFF', // 0xFF 0xFF 0xFF 0xFF = 0xFFFFFFFF (decimal: 4294967295)
    (unsigned char)'\xFD', (unsigned char)'\xFF', (unsigned char)'\xFF', (unsigned char)'\xFF'  // 0xFD 0xFF 0xFF 0xFF = 0xFDFFFFFF (decimal: 4261412863)
};

const unsigned char Diffie_Hellman::curve_b_buffer[SECRET_SIZE] =
{
    (unsigned char)'\xD3', (unsigned char)'\x5E', (unsigned char)'\xEE', (unsigned char)'\x2C',  // 0xD3 0x5E 0xEE 0x2C = 0xD35EEE2C (decimal: 3544230444)
    (unsigned char)'\x3C', (unsigned char)'\x99', (unsigned char)'\x24', (unsigned char)'\xD8', // 0x3C 0x99 0x24 0xD8 = 0x3C9924D8 (decimal: 1014205912)
    (unsigned char)'\x3D', (unsigned char)'\xF4', (unsigned char)'\x79', (unsigned char)'\x10', // 0x3D 0xF4 0x79 0x10 = 0x3DF47910 (decimal: 1038348816)
    (unsigned char)'\xC1', (unsigned char)'\x79', (unsigned char)'\x75', (unsigned char)'\xE8' // 0xC1 0x79 0x75 0xE8 = 0xC17975E8 (decimal: 3241690856)
};

// Class methods
Diffie_Hellman::Diffie_Hellman(const Elliptic_Curve_Point & base_point) : _base_point(base_point)
{
    generate_keypair();
}

Diffie_Hellman::Diffie_Hellman()
{
    new (&_base_point.x) Bignum(_default_base_point_x, SECRET_SIZE);
    new (&_base_point.y) Bignum(_default_base_point_y, SECRET_SIZE);
    _base_point.z = 1;
    generate_keypair();
}

Diffie_Hellman::Shared_Key Diffie_Hellman::shared_key(Elliptic_Curve_Point public_key)
{
    db<Diffie_Hellman>(TRC) << "Diffie_Hellman::shared_key(pub=" << public_key << ",priv=" << _private << ")" << std::endl;
    new (&_base_point.x) Bignum(_default_base_point_x, SECRET_SIZE);
    new (&_base_point.y) Bignum(_default_base_point_y, SECRET_SIZE);
    _base_point.z = 1;
    db<Diffie_Hellman>(INF) << "Diffie_Hellman: base point = " << _base_point << std::endl;

    public_key *= _private;
    public_key.x ^= public_key.y;

    db<Diffie_Hellman>(INF) << "Diffie_Hellman: shared key = " << public_key.x << std::endl;
    return public_key.x;
}

// Validate point: y^2 ≡ x^3 + ax + b (mod p)
bool Diffie_Hellman::is_valid_point(const Elliptic_Curve_Point& point) {
    // Check if x, y are in [0, p-1]
    

    Bignum curve_p = Bignum(curve_p_buffer, SECRET_SIZE);
    Elliptic_Curve_Point::Coordinate x = point.x;
    Elliptic_Curve_Point::Coordinate y = point.y;

    db<Bignum>(INF) << "Diffie_Hellman::is_valid_point(x=" << x << ",y=" << y << ",p=" << curve_p << ")" << std::endl;
    if (x >= curve_p || y >= curve_p) {
        db<Bignum>(ERR) << "Point coordinates out of range, x=" << x << ", y=" << y << ", p=" << curve_p << std::endl;
        return false;
    }

    Bignum curve_b = Bignum(curve_b_buffer, SECRET_SIZE); // b = 0xB4050A850C1B0A8D
    db<Bignum>(INF) << "Diffie_Hellman::is_valid_point(curve_b=" << curve_b << ")" << std::endl;

    Bignum left = y * y;                    // y^2
    Bignum right = x * x * x;               // x^3

    db<Bignum>(INF) << "Diffie_Hellman::is_valid_point(left=" << left << ",right=" << right << ")" << std::endl;

    right -= (x * 3);                       // x^3 - 3x (since a = -3)
    right += curve_b;                       // x^3 - 3x + b
    left -= right;                          // y^2 - (x^3 - 3x + b)

    bool is_valid = (left == Bignum(0)); // Should be 0 mod p
    if (!is_valid) {
        db<Bignum>(ERR) << "Point does not satisfy curve equation" << std::endl;
    }
    return is_valid;
}

void Diffie_Hellman::Elliptic_Curve_Point::operator*=(const Coordinate & b)
{
    db<Diffie_Hellman>(TRC) << "Diffie_Hellman::Elliptic_Curve_Point::operator*=(b=" << b << ") = " << *this << std::endl;
    // Finding last '1' bit of b
    static const unsigned int bits_in_digit = sizeof(typename Coordinate::Digit) * 8;

    typename Coordinate::Digit now;
    unsigned int b_len = sizeof(Coordinate) / sizeof(typename Coordinate::Digit);
    for(; (b_len > 1) && (b[b_len - 1] == 0); b_len--);
    if((b_len == 0) || (b[b_len - 1] == 0)) {
        x = 0;
        y = 0;
        z = 0;
        return;
    }

    now = b[b_len - 1];

    bool bin[bits_in_digit]; // Binary representation of 'now'
    unsigned int current_bit = bits_in_digit;

    Elliptic_Curve_Point pp(*this);

    for(int i = bits_in_digit - 1; i >= 0; i--) {
        if(now % 2)
            current_bit = i + 1;
        bin[i] = now % 2;
        now /= 2;
    }

    for(int i = b_len - 1; i >= 0; i--) {
        for(; current_bit < bits_in_digit; current_bit++) {
            jacobian_double();
            if(bin[current_bit])
                add_jacobian_affine(pp);
        }
        if(i > 0) {
            now = b[i-1];
            for(int j = bits_in_digit-1; j >= 0; j--) {
                bin[j] = now % 2;
                now /= 2;
            }
            current_bit = 0;
        }
    }

    Coordinate Z;
    z.invert();
    Z = z;
    Z *= z;

    x *= Z;
    Z *= z;

    y *= Z;
    z = 1;
}

void Diffie_Hellman::Elliptic_Curve_Point::jacobian_double()
{
    Coordinate B, C(x), aux(z);

    aux *= z; C -= aux;
    aux += x; C *= aux;
    C *= 3;

    z *= y; z *= 2;

    y *= y; B = y;

    y *= x; y *= 4;

    B *= B; B *= 8;

    x = C; x *= x;
    aux = y; aux *= 2;
    x -= aux;

    y -= x; y *= C;
    y -= B;
}

void Diffie_Hellman::Elliptic_Curve_Point::add_jacobian_affine(const Elliptic_Curve_Point &b)
{
    Coordinate A(z), B, C, X, Y, aux, aux2;

    A *= z;

    B = A;

    A *= b.x;

    B *= z; B *= b.y;

    C = A; C -= x;

    B -= y;

    X = B; X *= B;
    aux = C; aux *= C;

    Y = aux;

    aux2 = aux; aux *= C;
    aux2 *= 2; aux2 *= x;
    aux += aux2; X -= aux;

    aux = Y; Y *= x;
    Y -= X; Y *= B;
    aux *= y; aux *= C;
    Y -= aux;

    z *= C;

    x = X; y = Y;
}

__END_SYS