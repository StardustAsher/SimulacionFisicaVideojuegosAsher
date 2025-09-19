#include "Vector3D.h"
#include <cmath>
#include <stdexcept>

// Constructors
Vector3D::Vector3D() : x(0), y(0), z(0) {}
Vector3D::Vector3D(float x, float y, float z) : x(x), y(y), z(z) {}

// Arithmetic operators
Vector3D Vector3D::operator+(const Vector3D& other) const {
    return Vector3D(x + other.x, y + other.y, z + other.z);
}

Vector3D Vector3D::operator-(const Vector3D& other) const {
    return Vector3D(x - other.x, y - other.y, z - other.z);
}

Vector3D Vector3D::operator*(float scalar) const {
    return Vector3D(x * scalar, y * scalar, z * scalar);
}

Vector3D Vector3D::operator/(float scalar) const {
    if (scalar == 0.0f)
        throw std::runtime_error("Division by zero in Vector3D");
    return Vector3D(x / scalar, y / scalar, z / scalar);
}

Vector3D& Vector3D::operator=(const Vector3D& other)
{
	x = other.x; y = other.y; z = other.z;
	return *this;
}

Vector3D& Vector3D::operator+=(const Vector3D& other) {
    x += other.x; y += other.y; z += other.z;
    return *this;
}

Vector3D& Vector3D::operator*=(float scalar) {
    x *= scalar; y *= scalar; z *= scalar;
    return *this;
}

Vector3D& Vector3D::operator/=(float scalar) {
    if (scalar == 0.0f)
        throw std::runtime_error("Division by zero in Vector3D");
    x /= scalar; y /= scalar; z /= scalar;
    return *this;
}

// Comparison operators
bool Vector3D::operator==(const Vector3D& other) const {
    return x == other.x && y == other.y && z == other.z;
}

bool Vector3D::operator!=(const Vector3D& other) const {
    return !(*this == other);
}

// Vector operations
float Vector3D::dot(const Vector3D& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vector3D Vector3D::cross(const Vector3D& other) const {
    return Vector3D(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

float Vector3D::magnitude() const {
    return std::sqrt(x * x + y * y + z * z);
}

float Vector3D::magnitudeSquared() const {
    return x * x + y * y + z * z;
}

Vector3D Vector3D::normalize() const {
    float mag = magnitude();
    if (mag == 0.0f)
        return Vector3D(0, 0, 0);
    return *this / mag;
}

float Vector3D::distance(const Vector3D& other) const {
    return (*this - other).magnitude();
}

// Stream output
std::ostream& operator<<(std::ostream& os, const Vector3D& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}
