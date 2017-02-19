#include "stdafx.h"

Vector3::Vector3( const float * v )
{
	assert( v != NULL );

	x = v[0];
	y = v[1];
	z = v[2];
}

float Vector3::L2Norm() const
{
	return sqrt( SQR( x ) + SQR( y ) + SQR( z ) );
}

float Vector3::SqrL2Norm() const
{
	return SQR( x ) + SQR( y ) + SQR( z );
}

void Vector3::Normalize()
{
	const float norm = SqrL2Norm();

	if ( norm != 0 )
	{
		const float rn = 1 / sqrt( norm );

		x *= rn;
		y *= rn;
		z *= rn;
	}
}

Vector3 Vector3::CrossProduct( const Vector3 & v ) const
{
	return Vector3(
		y * v.z - z * v.y,
		z * v.x - x * v.z,
		x * v.y - y * v.x );
}

Vector3 Vector3::Abs() const
{
	return Vector3( abs( x ), abs( y ), abs( z ) );		
}

Vector3 Vector3::Max( const float a ) const
{
	return Vector3( MAX( x, a ), MAX( y, a ), MAX( z, a ) );
}

float Vector3::DotProduct( const Vector3 & v ) const
{
	return x * v.x + y * v.y + z * v.z;
}

float Vector3::PosDotProduct(const Vector3 &v) const {
	Vector3 n = *this;

	float dot = n.DotProduct(v);
	if (dot < 0) {
		dot = n.DotProduct(-v);
	}

	return dot;
}

Vector3 Vector3::Reflect(Vector3 normal) {
	Vector3 i = *this;
	i.Normalize();
	normal.Normalize();
	return i - 2 * normal.DotProduct(i) * normal;
}

Vector3 Vector3::Refract(Vector3 normal, float n1, float n2) {
	// Normalize vectors
	Vector3 dir = *this;
	dir.Normalize();
	normal.Normalize();

	// Refracted vector [Snell's law]
	float r = (n1 / n2);
	float c = dir.DotProduct(-normal);

	// Check if normal is not flipped
	if (c < 0) {
		c = dir.DotProduct(normal);
	}

	return r * dir + (r * c - sqrt(1 - SQR(r) * (1 - SQR(c)))) * normal;
}

/*Vector3 Vector3::Rotate( const float phi )
{
	const float cos_phi = cos( phi );
	const float sin_phi = sin( phi );

	return Vector2( x * cos_phi + y * sin_phi,
		-x * sin_phi + y * cos_phi );
}*/

char Vector3::LargestComponent( const bool absolute_value )
{
	const Vector3 d = ( absolute_value )? Vector3( abs( x ), abs( y ), abs( z ) ) : *this;

	if ( d.x > d.y )
	{
		if ( d.x > d.z )
		{			
			return 0 ;
		}
		else
		{
			return 2;
		}
	}
	else
	{
		if ( d.y > d.z )
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}

	return -1;
}

void Vector3::Print()
{
	printf( "(%0.3f, %0.3f, %0.3f)\n", x, y, z ); 
	//printf( "_point %0.3f,%0.3f,%0.3f\n", x, y, z );
}

// --- operátory ------

Vector3 operator-( const Vector3 & v )
{
	return Vector3( -v.x, -v.y, -v.z );
}

Vector3 operator+( const Vector3 & u, const Vector3 & v )
{
	return Vector3( u.x + v.x, u.y + v.y, u.z + v.z );
}

Vector3 operator-( const Vector3 & u, const Vector3 & v )
{
	return Vector3( u.x - v.x, u.y - v.y, u.z - v.z );
}

Vector3 operator*( const Vector3 & v, const float a )
{
	return Vector3( a * v.x, a * v.y, a * v.z );
}

Vector3 operator*( const float a, const Vector3 & v )
{
	return Vector3( a * v.x, a * v.y, a * v.z  ); 		
}

Vector3 operator*( const Vector3 & u, const Vector3 & v )
{
	return Vector3( u.x * v.x, u.y * v.y, u.z * v.z );
}

Vector3 operator/( const Vector3 & v, const float a )
{
	return v * ( 1 / a );
}

void operator+=( Vector3 & u, const Vector3 & v )
{
	u.x += v.x;
	u.y += v.y;	
	u.z += v.z;	
}

void operator-=( Vector3 & u, const Vector3 & v )
{
	u.x -= v.x;
	u.y -= v.y;
	u.z -= v.z;
}

void operator*=( Vector3 & v, const float a )
{
	v.x *= a;
	v.y *= a;
	v.z *= a;
}

void operator/=( Vector3 & v, const float a )
{
	const float r = 1 / a;

	v.x *= r;
	v.y *= r;
	v.z *= r;
}


