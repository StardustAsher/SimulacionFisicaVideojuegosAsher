#pragma once

#include <iostream>

class Vector3D
{
public:
    float x;
    float y;
    float z;

    // Constructors
    Vector3D();
    Vector3D(float x, float y, float z);

	float GetX() const { return x; }
	float GetY() const { return y; }
	float GetZ() const { return z; }

    // Arithmetic operators
    Vector3D operator+(const Vector3D& other) const;
    Vector3D operator-(const Vector3D& other) const;
    Vector3D operator*(float scalar) const;
    Vector3D operator/(float scalar) const;

	Vector3D& operator=(const Vector3D& other);
    Vector3D& operator+=(const Vector3D& other);
    Vector3D& operator*=(float scalar);
    Vector3D& operator/=(float scalar);

    // Comparison operators
    bool operator==(const Vector3D& other) const;
    bool operator!=(const Vector3D& other) const;

    // Vector operations
    float dot(const Vector3D& other) const;
    Vector3D cross(const Vector3D& other) const;
    float magnitude() const;
    float magnitudeSquared() const;
    Vector3D normalize() const;
    float distance(const Vector3D& other) const;

    // Stream output
    friend std::ostream& operator<<(std::ostream& os, const Vector3D& v);
};
