#pragma once

#include "Vector.h"
#include "Quaternion.h"
#include "Matrix.h"

namespace Matcha
{
	class Transform
	{
	public:
		enum class Space
		{
			World,
			Self
		};

	public:
		Transform();

		void Translate(const Vector3& translation);
		void Translate(float x, float y, float z);

		void Rotate(const Vector3& eulers, Space space = Space::Self);
		void Rotate(float x, float y, float z, Space space = Space::Self);
		void Rotate(const Vector3& axis, float angle, Space space = Space::Self);

		void SetPosition(const Vector3& position);
		void SetPosition(float x, float y, float z);
		Vector3& GetPosition();
		const Vector3& GetPosition() const;

		void SetRotation(const Quaternion& rotation);
		void SetRotationEuler(const Vector3& eulers);
		void SetRotationEuler(float x, float y, float z);
		const Quaternion& GetRotation() const;
		Vector3 GetRotationEuler() const;

		void SetScale(const Vector3& scale);
		void SetScale(float x, float y, float z);
		const Vector3& GetScale() const;

		Vector3 GetForward() const;
		Vector3 GetRight() const;
		Vector3 GetUp() const;

		Matrix4 GetModelMatrix() const;

		Transform& operator*=(const Transform& rhs);

	private:
		Vector3 mPosition;
		Quaternion mRotation;
		Vector3 mScale;
	};
}