#include "Transform.h"
#include "Matrix4x4.h"
#include "kMath.h"

struct CameraForGPU {
	Vector3 worldPosition;
};

#pragma once
class Camera {
public:
	Camera();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	// Getter
	const Matrix4x4& GetWorldMatrix() const { 
		Matrix4x4 world = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		return world; 
	}
	// Getter
	const Matrix4x4& GetViewMatrix() const { return viewMatrix; }
	// Getter
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix; }
	// Getter
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix; }
	// Getter(Rotate)
	const Vector3& GetRotate() const { return transform.rotate; }
	// Getter(Translate)
	const Vector3& GetTranslate() const { return transform.translate; }
	// Getter(farClipDistance)
	const float& GetFarClipDistance() const { return farClipDistance; }
	// Getter(fovY)
	const float& GetfovY() const { return fovY; }
	// Getter(Transform)
	const Transform& GetTransform() const { return transform; }

	// Setter(Rotate)
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	// Setter(Translate)
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }
	// Setter(farClipDistance)
	void SetFarClipDistance(const float& farClip) { farClipDistance = farClip; }
	// Setter(fovY)
	void SetFovY(const float& fov) { fovY = fov; }
	// Setter(Transform)
	void SetTransform(const Transform& transform) { this->transform = transform; }
	// Setter(Parent)
	void SetParent(const Matrix4x4& worldMatrix) { 
		parent = worldMatrix; 
		isParent = true;
	}
	// Delete Parent(Parent 解除)
	void DeleteParent() {
		isParent = false;
	}

private:
	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;

	Matrix4x4 parent;
	bool isParent = false;

	Matrix4x4 projectionMatrix;
	float fovY;
	float aspect;
	float nearClipDistance;
	float farClipDistance;

	Matrix4x4 viewProjectionMatrix;
};
