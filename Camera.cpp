#pragma once


#include <Windows.h>
#include <gl/GL.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Camera.h"
//
//#pragma comment (lib,"opengl32.lib")

//struct SCamera camera_main = { 0,0,1.7,0,0 };

//void Camera_main_Apply()
//{
//    glRotatef(-camera_main.Xrot, 1, 0, 0);
//    glRotatef(-camera_main.Zrot, 0, 0, 1);
//    glTranslatef(-camera_main.x, -camera_main.y, -camera_main.z);
//}
//
//
//void Camera_main_Rotation(float xAngle, float zAngle)
//{
//    camera_main.Zrot += zAngle;
//    if (camera_main.Zrot < 0) camera_main.Zrot += 360;
//    if (camera_main.Zrot > 360) camera_main.Zrot -= 360;
//    camera_main.Xrot += xAngle;
//    if (camera_main.Xrot < 0) camera_main.Xrot = 0;
//    if (camera_main.Xrot > 180) camera_main.Xrot = 180;
//}
//
//void Camera_AutoMoveByMouse(int centerX, int centerY,float speed)
//{
//    POINT cur;
//    POINT base = { centerX,centerY };
//    GetCursorPos(&cur);
//    Camera_main_Rotation((base.y - cur.y) * speed, (base.x - cur.x) *speed);
//    SetCursorPos(base.x, base.y);
//}
//
//void Camera_MoveDirection(int forwardMove, int rightMove, float speed)
//{
//    float ugol = -camera_main.Zrot / 180 * M_PI;
//    if (forwardMove > 0)
//        ugol += rightMove > 0 ? M_PI_4 : (rightMove < 0 ? -M_PI_4 : 0);
//    if (forwardMove < 0)
//        ugol += M_PI + (rightMove > 0 ? -M_PI_4 : (rightMove < 0 ? M_PI_4 : 0));
//    if (forwardMove == 0)
//    {
//        ugol += rightMove > 0 ? M_PI_2 : -M_PI_2;
//        if (rightMove == 0) speed=0;
//    }
//    if (speed != 0)
//    {
//        camera_main.x += sin(ugol) * speed;
//        camera_main.y += cos(ugol) * speed;
//    }
//}

