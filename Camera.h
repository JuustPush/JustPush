#pragma once


struct{
	float x, y, z;
	float Xrot, Zrot;
}camera_main = { 0,0,1.7,0,0 };


void Camera_main_Apply();
void Camera_main_Rotation(float xAngle, float zAngle);
void Camera_AutoMoveByMouse(int centerX, int centerY, float speed);
void Camera_MoveDirection(int forwardMove, int rightMove, float speed);
