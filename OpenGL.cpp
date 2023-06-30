#include "Camera.h"
#include <string>
#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <ctime>
#define _USE_MATH_DEFINES
#include <math.h>
#include <mmsystem.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image-master/stb_image.h"

#pragma comment (lib,"opengl32.lib")
#pragma comment (lib,"winmm.lib")

// Модуль камеры
void Camera_main_Apply()
{
    glRotatef(-camera_main.Xrot, 1, 0, 0);
    glRotatef(-camera_main.Zrot, 0, 0, 1);
    glTranslatef(-camera_main.x, -camera_main.y, -camera_main.z);
}

void Camera_main_Rotation(float xAngle, float zAngle)
{
    camera_main.Zrot += zAngle;
    if (camera_main.Zrot < 0) camera_main.Zrot += 360;
    if (camera_main.Zrot > 360) camera_main.Zrot -= 360;
    camera_main.Xrot += xAngle;
    if (camera_main.Xrot < 0) camera_main.Xrot = 0;
    if (camera_main.Xrot > 180) camera_main.Xrot = 180;
}

void Camera_AutoMoveByMouse(int centerX, int centerY, float speed)
{
    POINT cur;
    POINT base = { centerX,centerY };
    GetCursorPos(&cur);
    Camera_main_Rotation((base.y - cur.y) * speed, (base.x - cur.x) * speed);
    SetCursorPos(base.x, base.y);
}

void Camera_MoveDirection(int forwardMove, int rightMove, float speed)
{
    float ugol = -camera_main.Zrot / 180 * M_PI;
    if (forwardMove > 0)
        ugol += rightMove > 0 ? M_PI_4 : (rightMove < 0 ? -M_PI_4 : 0);
    if (forwardMove < 0)
        ugol += M_PI + (rightMove > 0 ? -M_PI_4 : (rightMove < 0 ? M_PI_4 : 0));
    if (forwardMove == 0)
    {
        ugol += rightMove > 0 ? M_PI_2 : -M_PI_2;
        if (rightMove == 0) speed = 0;
    }
    if (speed != 0)
    {
        camera_main.x += sin(ugol) * speed;
        camera_main.y += cos(ugol) * speed;
    }
}

// конец модуля камеры
float light = 1;

HWND hwnd;
POINTFLOAT* mas = NULL;
int cnt;
float scaleY=1;
float curX;

int width1, height1;

POINTFLOAT vertices[] = {
    {0,0},
    {1,0},
    {1,1},
    {0,1}
};

POINTFLOAT pf3_1[] = {
        {-0.85,0.85},
        {0.85,0.85},
        {0.85,0.85},
        {-0.85,-0.85},
        {-0.85,-0.85},
        {0.85,-0.85}
};

float Colors[] = { 1,0,0, 0,1,0, 0,0,1, 1,1,0 };
 
GLuint index[] = { 1,2,3, 3,0,1 };

float vert[] = { 1,1,0, -1,0,-1, 1,-1,0 };

float vert2[] = { 1,1,0, -1,0,1, 1,-1,0 };

float vert3[] = { 1,1,0, 1,-1,0, -1,-1,0, -1,1,0 };

float pyramid[] = { 0,0,2, 1,1,0, 1,-1,0, -1,-1,0, -1,1,0, 1,1,0 };
float xAlfa = 80;
float zAlfa = 0;
POINTFLOAT pos1 = { 0,0 };
float k;

float kube[] = { 0,0,0, 0,1,0, 1,1,0, 1,0,0, 0,0,1, 0,1,1, 1,1,1, 1,0,1 };
GLuint kubeInd[] = { 0,1,2, 2,3,0, 4,5,6, 6,7,4, 3,2,5, 6,7,3, 0,1,5, 5,4,0, 1,2,6, 6,5,1,
                    0,3,7, 7,4,0 };

BOOL showMask = FALSE;

#include "Main.h"

void Recipe_Add(int items[3][3],int itemOut)
{
    recipeCnt++;
    recipe =(TRecipe*) realloc(recipe, sizeof(TRecipe) * recipeCnt);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            recipe[recipeCnt - 1].items[i][j] = items[i][j];
    recipe[recipeCnt - 1].itemOut = itemOut;
}

void Recipe_Check()
{
    for (int k = 0; k < recipeCnt; k++)
    {
        BOOL checkOK = TRUE;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                if (craft_menu.items[i][j].type != recipe[k].items[i][j])
                    checkOK = FALSE;
        if (checkOK)
        {
            craft_menu.itemOut.type = recipe[k].itemOut;
            break;
        }
        else
            craft_menu.itemOut.type = 0;
    }
}


BOOL IsPointInSlot(TSlot slot, int x, int y)
{
    return ((x > slot.x) && (x < slot.x + slot.width)
        && (y > slot.y) && (y < slot.y + slot.height));
}

void Anim_Set(TAnim* anm, TObject* obj)
{
    if (anm->obj != NULL) return;
    anm->obj = obj;
    anm->cnt = 10;
    anm->dx = (camera_main.x - obj->x) / (float)anm->cnt;
    anm->dy = (camera_main.y - obj->y) / (float)anm->cnt;
    anm->dz = ((camera_main.z - obj->scale-0.2)-obj->z) / (float)anm->cnt;
}


void Anim_Move(TAnim* anm)
{
    if (anm->obj != NULL)
    {
        anm->obj->x += anm->dx;
        anm->obj->y += anm->dy;
        anm->obj->z += anm->dz;
        anm->cnt--;
        if (anm->cnt < 1)
        {
            int i;
            for  (i= 0; i < bagSize; i++)
                if (bag[i].type <= 0)
                {
                    bag[i].type = anm->obj->type;
                    break;
                }
            if (i <bagSize)
            {
                anm->obj->x = rand() % mapWe;
                anm->obj->y = rand() % mapHe;
            }

            anm->obj->z = Map_GetHeight(anm->obj->x, anm->obj->y);
            anm->obj = NULL;
        }
    }
}

void Tree_Show(TObjGroup obj)
{
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, cube);
        glColor3f(0.7, 0.7, 0.7);
        glNormal3f(0, 0, 1);
        glBindTexture(GL_TEXTURE_2D, obj.type);
        for (int i = 0; i < obj.itemsCnt; i++)
        {
            if (obj.items[i].type == 1) glTexCoordPointer(2, GL_FLOAT, 0, cubeUVlog);
            else glTexCoordPointer(2, GL_FLOAT, 0, cubeUVleaf);
            glPushMatrix();
            glTranslatef(obj.items[i].x, obj.items[i].y, obj.items[i].z);
            glDrawElements(GL_TRIANGLES, cubeIndCnt, GL_UNSIGNED_INT, cubeInd);
            glPopMatrix();
        }
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Tree_Create(TObjGroup* obj, int type, float x, float y)
{
    obj->type = type;
    float z = Map_GetHeight(x + 0.5, y + 0.5) - 0.5;
    int logs = 6;
    int leafs = 5 * 5 * 2 - 2 + 3 * 3 * 2;

    obj->itemsCnt = logs+leafs;
    obj->items = (TObject*)malloc(sizeof(TObject) * obj->itemsCnt);

    for (int i = 0; i < logs; i++)
    {
        obj->items[i].type = 1;
        obj->items[i].x = x;
        obj->items[i].y = y;
        obj->items[i].z = z + i;
    }
    int posTree = logs;
    for (int k=0;k<2;k++)
        for(int i=x-2;i<=x+2;i++)
            for (int j=y-2;j<=y+2;j++)
                if ((i != x) || (j != y))
                {
                    obj->items[posTree].type = 2;
                    obj->items[posTree].x = i;
                    obj->items[posTree].y = j;
                    obj->items[posTree].z = z + logs - 2 + k;
                    posTree++;
                }

    for (int k = 0; k < 2; k++)
        for (int i = x - 1; i <= x + 1; i++)
            for (int j = y - 1; j <= y + 1; j++)
                {
                    obj->items[posTree].type = 2;
                    obj->items[posTree].x = i;
                    obj->items[posTree].y = j;
                    obj->items[posTree].z = z + logs + k;
                    posTree++;
                }
}

#define sqr(a) (a)*(a)
void CalcNormals(TCell a, TCell b, TCell c, TCell* n)
{
    float wrki;
    TCell v1, v2;

    v1.x = a.x - b.x;
    v1.y = a.y - b.y;
    v1.z = a.z - b.z;
    v2.x = b.x - c.x;
    v2.y = b.y - c.y;
    v2.z = b.z - c.z;

    n->x = (v1.y * v2.z - v1.z * v2.y);
    n->y = (v1.z * v2.x - v1.x * v2.z);
    n->z = (v1.x * v2.y - v1.y * v2.x);
    wrki = sqrt(sqr(n->x) + sqr(n->y) + sqr(n->z));
    n->x /= wrki;
    n->y /= wrki;
    n->z /= wrki;
}


BOOL IsCoordInMap(float x, float y)
{
    return (x >= 0) && (x < mapWe) && (y >= 0) && (y < mapHe);
}

void Map_CreateHill(int posX, int posY, int Rad, int height)
{
    for (int i=posX-Rad;i<=posX+Rad;i++)
        for (int j=posY-Rad;j<=posY+Rad;j++)
            if (IsCoordInMap(i, j))
            {
                float len = sqrt(pow(posX - i, 2) + pow(posY - j, 2));
                if (len < Rad)
                {
                    len = len / Rad * M_PI_2;
                    map[i][j].z += cos(len) * height;
                }
            }
}

float Map_GetHeight(float x, float y)
{
    if (!IsCoordInMap(x, y)) return 0;
    int cX = (int)x;
    int cY = (int)y;
    float h1 = ( (1 - (x - cX)) * map[cX][cY].z   +  (x - cX) * map[cX+1][cY].z   );
    float h2 = ( (1 - (x - cX)) * map[cX][cY+1].z +  (x - cX) * map[cX + 1][cY+1].z );
    return (1 - (y - cY)) * h1 + (y - cY) * h2;

}

void Game_Create()
{
    memset(&craft_menu, 0, sizeof(craft_menu));
    craft_menu.show = FALSE;
}

void Map_terrain_Init()
{
    for (int i = 0; i < bagSize; i++)
    {
        bag[i].type = 0;
    }
    Load_Texture("textures/pole.png", &tex_pole);
    Load_Texture("textures/trava.png", &tex_trava);
    Load_Texture("textures/flower.png", &tex_flower);
    Load_Texture("textures/flower2.png", &tex_flower2);
    Load_Texture("textures/grib.png", &tex_grib);
    Load_Texture("textures/tree.png", &tex_tree);
    Load_Texture("textures/tree2.png", &tex_tree2);
    Load_Texture("textures/wood.png", &tex_wood);

    Load_Texture("textures/speed.png", &tex_ico_speed);
    Load_Texture("textures/eye.png", &tex_ico_eye);

    Load_Texture("textures/mortar.png", &tex_ico_mortar);
    Load_Texture("textures/potion_eye.png", &tex_ico_potion_eye);
    Load_Texture("textures/potion_speed.png", &tex_ico_potion_speed);
    Load_Texture("textures/potion_life.png", &tex_ico_potion_life);

    bag[0].type = tex_ico_mortar;
    
    int Recipe_tex_ico_potion_eye[3][3] = {
    (int)tex_flower2,    0,          tex_flower2,
    0,              tex_grib,   0,
    tex_flower2,    0,          tex_flower2
    };
    Recipe_Add(Recipe_tex_ico_potion_eye, tex_ico_potion_eye);
    int Recipe_tex_ico_potion_speed[3][3] = {
    tex_flower,     tex_flower, tex_flower,
    0,              0,          0,
    tex_flower,     tex_flower, tex_flower
    };
    Recipe_Add(Recipe_tex_ico_potion_speed, tex_ico_potion_speed);
    int Recipe_tex_ico_potion_life[3][3] = {
    0,              tex_grib,   0,
    tex_grib,       tex_grib,   tex_grib,
    0,              tex_grib,   0
    };
    Recipe_Add(Recipe_tex_ico_potion_life, tex_ico_potion_life);


    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.60);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    for (int i = 0; i < mapWe; i++)
        for (int j = 0; j < mapHe; j++)
        {
            map[i][j].x = i;
            map[i][j].y = j;
            map[i][j].z = (rand() % 10) * 0.02;

            mapUV[i][j].u = i;
            mapUV[i][j].v = j;
        }
    for (int i = 0; i < mapWe - 1; i++)
    {
        int pos = i * mapHe;
        for (int j = 0; j < mapHe - 1; j++)
        {
            mapInd[i][j][0] = pos;
            mapInd[i][j][1] = pos+1;
            mapInd[i][j][2] = pos+1+mapHe;
            mapInd[i][j][3] = pos+1+mapHe;
            mapInd[i][j][4] = pos+mapHe;
            mapInd[i][j][5] = pos;
            pos++;
        }
    }
    for (int i = 0; i < 10; i++)
        Map_CreateHill(rand() % mapWe, rand() % mapHe, rand() % 50, rand() % 10);

    for (int i = 0; i < mapWe - 1; i++)
        for (int j = 0; j < mapHe - 1; j++)
            CalcNormals(map[i][j], map[i + 1][j], map[i][j + 1], &mapNormal[i][j]);
    
    int travaN = 2000;
    int gribN = 30;
    int treeN = 40;
    plantCnt = travaN + gribN + treeN;
    plantMas = (TObject*)realloc(plantMas, sizeof(*plantMas) * plantCnt);
    for (int i = 0; i < plantCnt; i++)
    {
        if (i < travaN) {
            plantMas[i].type = rand() % 10 != 0 ? tex_trava : (rand()%2 == 0?tex_flower : tex_flower2);
            plantMas[i].scale = 0.7 + (rand() % 5) * 0.1;
        }
        else if (i < (travaN + gribN))
        {
            plantMas[i].type = tex_grib;
            plantMas[i].scale = 0.2 + (rand() % 10) * 0.01;
        }
        else
        {
            plantMas[i].type = rand() % 2 == 0 ? tex_tree : tex_tree2;
            plantMas[i].scale = 4 + (rand() % 14);
        }
        plantMas[i].x = rand() % mapWe;
        plantMas[i].y = rand() % mapHe;
        plantMas[i].z = Map_GetHeight(plantMas[i].x, plantMas[i].y);
    }

    treeCnt = 50;
    tree = (TObjGroup*)realloc(tree, sizeof(*tree) * treeCnt);
    for (int i = 0; i < treeCnt; i++)
        Tree_Create(tree + i, tex_wood, rand() % mapWe, rand() % mapHe);
}

void Buff_Timer(TBuff* buff)
{
    if (buff->time > 0)
    {
        buff->time--;
        if (buff->time <= 0)
            buff->timeMax = 0;
    }
}

void Map_Proc()
{
    hunger++;
    if (hunger > 1000)
    {
        hunger = 0;
        health--;
        if (health < 1)
            PostQuitMessage(0);
    }
    Buff_Timer(&buffs.speed);
    Buff_Timer(&buffs.eye);
}

void Buff_Show(int x, int y, int scale, TBuff buff, int texID)
{
    if (buff.time > 0)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, bagRect);
        glTexCoordPointer(2, GL_FLOAT, 0, bagRectUV);
            glPushMatrix();
            glTranslatef(x, y, 0);
            glScalef(scale, scale, 1);
            glColor3f(1, 1, 1);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texID);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glScalef(1, 1 - (buff.time / (float)buff.timeMax), 1);
            glColor4f(1, 1, 1, 0.5);
            glDisable(GL_ALPHA_TEST);
                glDisable(GL_TEXTURE_2D);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glEnable(GL_ALPHA_TEST);
            glPopMatrix();
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}

void Map_Terrain_Show()
{
    float sz = 0.1;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-scrKoef * sz, scrKoef * sz, -sz, sz, sz * 2, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);

    Map_Proc();

    static float alfa = 0;
    alfa += 0.03;
    if (alfa > 180) alfa -= 360;

    #define abs(a) ((a)>0?(a): -(a))
    float kcc = 1 - (abs(alfa) / 180);

    #define sakat 40.0
    float k = 90 - abs(alfa);
    k = (sakat - abs(k));
    k = k < 0 ? 0 : k / sakat;

    if (selectMode) glClearColor(0, 0, 0, 0);
    else glClearColor(0.6f*kcc, 0.8f*kcc, 1.0f*kcc, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (selectMode)
    {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
    }
    else
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
    }
    Anim_Move(&animation);

    glPushMatrix();   
    if (!selectMode)
    {
        glPushMatrix();
            glRotatef(-camera_main.Xrot, 1, 0, 0);
            glRotatef(-camera_main.Zrot, 0, 0, 1);
            glRotatef(alfa, 0, 1, 0);
            glTranslatef(0, 0, 20);
            glDisable(GL_DEPTH_TEST);
                glDisable(GL_TEXTURE_2D);
                glColor3f(1, 1 - k * 0.8, 1 - k);
                glEnableClientState(GL_VERTEX_ARRAY);
                    glVertexPointer(3, GL_FLOAT, 0, sun);
                    glScalef(3, 3, 3);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                glDisableClientState(GL_VERTEX_ARRAY);
                glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);

        glPopMatrix();
    }
        Camera_main_Apply();
        
        glPushMatrix();
            glRotatef(alfa, 0, 1, 0);
            GLfloat position[] = { 0,0,1,0 };
            glLightfv(GL_LIGHT0, GL_POSITION, position);
            float mas[] = { 1 + k * 2,1,1,0 };
            glLightfv(GL_LIGHT0, GL_DIFFUSE, mas);

            float clr = kcc * 0.15 + 0.05;
            float mas0[] = { clr,clr,clr,0 };
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, mas0);
        glPopMatrix();

        if (!selectMode)
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);
                glVertexPointer(3, GL_FLOAT, 0, map);
                glTexCoordPointer(2, GL_FLOAT, 0, mapUV);
                glColor3f(0.7, 0.7, 0.7);
                glNormalPointer(GL_FLOAT, 0, mapNormal);
                glBindTexture(GL_TEXTURE_2D, tex_pole);
                glDrawElements(GL_TRIANGLES, mapIndCnt, GL_UNSIGNED_INT, mapInd);

            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
        }
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            glVertexPointer(3, GL_FLOAT, 0, plant);
            glTexCoordPointer(2, GL_FLOAT, 0, plantUV);
            glColor3f(0.7, 0.7, 0.7);
            glNormal3f(0, 0, 1);
            selectMasCnt = 0;
            int selectColor = 1;
            for (int i = 0; i < plantCnt; i++)
            {
                if (selectMode)
                {
                    if ((plantMas[i].type == tex_tree) || (plantMas[i].type == tex_tree2))
                        continue;
                    static int radius = 3;
                    if ((plantMas[i].x > camera_main.x - radius)
                        && (plantMas[i].x < camera_main.x + radius)
                        && (plantMas[i].y > camera_main.y - radius)
                        && (plantMas[i].y < camera_main.y + radius))
                    {
                        glColor3ub(selectColor, 0, 0);
                        selectMas[selectMasCnt].colorIndex = selectColor;
                        selectMas[selectMasCnt].plantMas_Index = i;
                        selectMasCnt++;
                        selectColor++;
                        if (selectColor >= 255)
                            break;
                    }
                    else
                        continue;
                }
                else
                {
                    if ((plantMas[i].type == tex_grib) && (buffs.eye.time > 0))
                        glDisable(GL_LIGHTING);
                }
                glBindTexture(GL_TEXTURE_2D, plantMas[i].type);
                glPushMatrix();
                    glTranslatef(plantMas[i].x, plantMas[i].y, plantMas[i].z);
                    glScalef(plantMas[i].scale, plantMas[i].scale, plantMas[i].scale);
                    glDrawElements(GL_TRIANGLES, plantIndCnt, GL_UNSIGNED_INT, plantInd);
                glPopMatrix();
                if (!selectMode)
                {
                    if ((plantMas[i].type == tex_grib) && (buffs.eye.time > 0))
                        glEnable(GL_LIGHTING);
                }
            }
            
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            if (!selectMode)
                for (int i = 0; i < treeCnt; i++)
                Tree_Show(tree[i]);

    glPopMatrix();
}

void Player_Take(HWND hwnd)
{
    selectMode = TRUE;
    Map_Terrain_Show();
    selectMode = FALSE;

    RECT rct;
    GLubyte clr[3];
    GetClientRect(hwnd, &rct);
    glReadPixels(rct.right / 2.0, rct.bottom / 2.0, 1, 1, GL_RGB,
        GL_UNSIGNED_BYTE, clr);
    if (clr[0] > 0)
    {
        for (int i = 0; i < selectMasCnt; i++)
            if (selectMas[i].colorIndex == clr[0])
            {
                Anim_Set(&animation, plantMas + selectMas[i].plantMas_Index);
            }
    }

}

typedef struct {
    TColor clr;
}TCell_1;

#define pW 40
#define pH 40
TCell_1 map1[pW][pH];

void Map_Init()
{
    for (int i = 0; i < pW; i++)
        for (int  j = 0; j < pH; j++)
        {
            float dc = (rand() % 20) * 0.01;
            map1[i][j].clr.r = 0.31 + dc;
            map1[i][j].clr.g = 0.5 + dc;
            map1[i][j].clr.b = 0.13 + dc;
        }
}

#define EnemyCnt 40
struct {
    float x, y, z;
    BOOL active;
}enemy[EnemyCnt];

void Enemy_Init()
{
    for (int i = 0; i < EnemyCnt; i++)
    {
        enemy[i].active = TRUE;
        enemy[i].x = rand() % pW;
        enemy[i].y = rand() % pH;
        enemy[i].z = rand() % 5;
    }
}

void Enemy_Show()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, kube);
    for (int i = 0; i < EnemyCnt; i++)
    {
        if (!enemy[i].active) continue;
        glPushMatrix();
            glTranslatef(enemy[i].x, enemy[i].y, enemy[i].z);
            if (showMask)
                glColor3ub(255 - i, 0, 0);
            else
                glColor3ub(244, 60, 43);          
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, kubeInd);
        glPopMatrix();
    }
    glDisableClientState(GL_VERTEX_ARRAY);
}

struct {
    float x, y, z;
    float Xrot, Zrot;
} camera = {0,0,1.7,70,-40};

void Camera_Apply()
{
    glRotatef(-camera.Xrot, 1, 0, 0);
    glRotatef(-camera.Zrot, 0, 0, 1);
    glTranslatef(-camera.x, -camera.y, -camera.z);
}

void Camera_Rotation(float xAngle, float zAngle)
{
    camera.Zrot += zAngle;
    if (camera.Zrot < 0) camera.Zrot += 360;
    if (camera.Zrot > 360) camera.Zrot -= 360;
    camera.Xrot += xAngle;
    if (camera.Xrot < 0) camera.Xrot = 0;
    if (camera.Xrot > 180) camera.Xrot = 180;
}

void Player_Shooter_Move()
{
    if (GetForegroundWindow() != hwnd) return;

    float ugol = -camera.Zrot / 180 * M_PI;
    float speed = 0;
    if (GetKeyState('W') < 0) speed = 0.1;
    if (GetKeyState('S') < 0) speed = -0.1;
    if (GetKeyState('A') < 0) { speed = 0.1; ugol -= M_PI * 0.5; }
    if (GetKeyState('D') < 0) { speed = 0.1; ugol += M_PI * 0.5; }

    if (speed != 0)
    {
        camera.x += sin(ugol) * speed;
        camera.y += cos(ugol) * speed;
    }

    POINT cur;
    static POINT base = { 400,300 };
    GetCursorPos(&cur);
    Camera_Rotation((base.y - cur.y) / 5.0, (base.x - cur.x) / 5.0);
    SetCursorPos(base.x, base.y);
}

void WndResize(int x, int y);

void Game_Shooter_Move()
{
    Player_Shooter_Move();
}

void Game_Shooter_Init()
{
    glEnable(GL_DEPTH_TEST);
    Map_Init();
    Enemy_Init();

    RECT rct;
    GetClientRect(hwnd, &rct);
    WndResize(rct.right, rct.bottom);
}

void Game_Shooter_Show()
{
    if (showMask) glClearColor(0, 0, 0, 0);
    else glClearColor(0.6, 0.8, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
        Camera_Apply(); 
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, kube);
        for (int i=0;i<pW;i++)
            for (int j = 0; j < pH; j++)
            {
                glPushMatrix();
                    glTranslatef(i, j, 0);
                    if (showMask)
                        glColor3f(0, 0, 0);
                    else
                        glColor3f(map1[i][j].clr.r, map1[i][j].clr.g, map1[i][j].clr.b);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, kubeInd);
                glPopMatrix();
            }
        glDisableClientState(GL_VERTEX_ARRAY);
        Enemy_Show();
    glPopMatrix();
}


void Player_Shoot()
{
    PlaySoundA("SS.wav", NULL, SND_ASYNC);
    showMask = TRUE;
    Game_Shooter_Show();
    showMask = FALSE;

    RECT rct;
    GLubyte clr[3];
    GetClientRect(hwnd, &rct);
    glReadPixels(rct.right / 2.0, rct.bottom / 2.0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, clr);
    if (clr[0] > 0)
        enemy[255-clr[0]].active = FALSE;
    
}

void CraftMenu_Resize(int scale)
{
    craft_menu.width = scale * 6;
    craft_menu.height = scale * 4;
    craft_menu.x = (scrSize.x - craft_menu.width) * 0.5;
    craft_menu.y = (scrSize.y - craft_menu.height) * 0.5;
    int scale05 = scale * 0.5;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
        {
            craft_menu.items[i][j].x = craft_menu.x + scale05 + i * scale;
            craft_menu.items[i][j].y = craft_menu.y + scale05 + j * scale;
            craft_menu.items[i][j].width = scale;
            craft_menu.items[i][j].height = scale;
        }
    craft_menu.itemOut.x = craft_menu.x + scale05 + 4 * scale;
    craft_menu.itemOut.y = craft_menu.y + scale05 + 1 * scale;
    craft_menu.itemOut.width = scale;
    craft_menu.itemOut.height = scale;
}

void WndResize(int x,int y)
{
    glViewport(0, 0, x, y);
    scrSize.x = x;
    scrSize.y = y;
    scrKoef = x / (float)y;

    CraftMenu_Resize(50);
}

void Cell_Show(int x, int y, int scaleX,int scaleY,int type)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, bagRect);
    glTexCoordPointer(2, GL_FLOAT, 0, bagRectUV);

    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scaleX, scaleY, 1);
        glColor3ub(110, 95, 73);

        glDisable(GL_TEXTURE_2D);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        if (type > 0)
        {
            glColor3f(1, 1, 1);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, type);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        glColor3ub(160, 146, 116);
        glLineWidth(3);
        glDisable(GL_TEXTURE_2D);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}

void Bag_Show(int x, int y, int scale)
{
    for (int i = 0; i < bagSize; i++)
        Cell_Show(x + i * scale, y, scale, scale, bag[i].type);
}

void HandItem_Show()
{
    if ((handItemType > 0)&&(!mouseBind))
        Cell_Show(mousePos.x, mousePos.y, 50, 50, handItemType);
}

int Bag_GetCnt(int type)
{
    int cnt = 0;
    for (int i = 0; i< bagSize; i++)
        if (bag[i].type == type)
            cnt++;
    return cnt;
}

void Bag_DelCnt(int type, int cnt)
{
    for (int i = 0; i < bagSize; i++)
        if (bag[i].type == type)
        {
            bag[i].type = -1;
            cnt--;
            if (cnt <= 0)return;
        }
}

void Bag_Click(int x,int y,int scale, int mx, int my,int button)
{
    if ((my < y) || (my > y + scale)) return;
    for (int i = 0; i < bagSize; i++)
    {
        if ((mx > x + i * scale) && (mx < x + (i + 1) * scale))
        {
            if (button == WM_LBUTTONDOWN)
            {
                int type = handItemType;
                handItemType = bag[i].type;
                bag[i].type = type;
            }
            else if (bag[i].type == tex_ico_mortar)
            {
                craft_menu.show = !craft_menu.show;
            }

            else if (bag[i].type == tex_grib)
            {
                health++;
                if (health > healthMax) health = healthMax;
                bag[i].type = -1; 
            }
            else if (bag[i].type == tex_ico_potion_life)
            {
                health+=15;
                if (health > healthMax) health = healthMax;
                bag[i].type = -1;
            }

            else if (bag[i].type == tex_ico_potion_speed)
            {
                buffs.speed.time = 3600;
                buffs.speed.timeMax = 3600;
                bag[i].type = -1;
            }
            else if (bag[i].type == tex_ico_potion_eye)
            {
                buffs.eye.time = 3600;
                buffs.eye.timeMax = 3600;
                bag[i].type = -1;
            }
            else
                bag[i].type = -1;
        }
    }
}

void Health_Show(int x, int y, int scale)
{
    glDisable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, hearth);
        for (int i = 0; i < healthMax; i++)
        {
            glPushMatrix();
                glTranslatef(x + i * scale, y, 0);
                glScalef(scale, scale, 1);
                if (i < health) glColor3f(1, 0, 0);
                else glColor3f(0, 0, 0);
                glDrawArrays(GL_TRIANGLE_FAN, 0,6);
            glPopMatrix();
        }
     glDisableClientState(GL_VERTEX_ARRAY);
}

void Cross_Show()
{
    static float cross[] = { 0,-1, 0,1, -1,0, 1,0 };
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, cross);
        glPushMatrix();
            glColor3f(1, 1, 1);
            glTranslatef(scrSize.x * 0.5, scrSize.y * 0.5, 0);
            glScalef(15, 15, 1);
            glLineWidth(3);
            glDrawArrays(GL_LINES, 0, 4);
        glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);
}

void CraftMenu_Click(int mx, int my, int button)
{
    if ((!craft_menu.show) || (button != WM_LBUTTONDOWN)) return;
    for (int i=0;i<3; i++)
        for (int j=0;j<3;j++)
            if (IsPointInSlot(craft_menu.items[i][j], mx, my))
            {
                int type = handItemType;
                handItemType = craft_menu.items[i][j].type;
                craft_menu.items[i][j].type = type;
                if (craft_menu.items[i][j].type <= 0)
                    craft_menu.items[i][j].type = 0;
            }
    if (IsPointInSlot(craft_menu.itemOut, mx, my)
        && (handItemType <= 0)
        && (craft_menu.itemOut.type > 0))
    {
        handItemType = craft_menu.itemOut.type;
        craft_menu.itemOut.type = 0;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                craft_menu.items[i][j].type = 0;
    }
    Recipe_Check();
}

void CraftMenu_Show()
{
    if ((!craft_menu.show) || (mouseBind)) return;
    Cell_Show(craft_menu.x, craft_menu.y, craft_menu.width, craft_menu.height, 0);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            Cell_Show(craft_menu.items[i][j].x,
                craft_menu.items[i][j].y,
                craft_menu.items[i][j].width,
                craft_menu.items[i][j].height,
                craft_menu.items[i][j].type);

    Cell_Show(craft_menu.itemOut.x,
        craft_menu.itemOut.y,
        craft_menu.itemOut.width,
        craft_menu.itemOut.height,
        craft_menu.itemOut.type);
}

void Menu_Show()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, scrSize.x, scrSize.y, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    Bag_Show(10, 10, 50);
    Health_Show(10, 70, 30);
    Cross_Show();
    CraftMenu_Show();

    Buff_Show(10, 110, 50, buffs.speed, tex_ico_speed);
    Buff_Show(60, 110, 50, buffs.eye, tex_ico_eye);
    HandItem_Show();
} 



typedef struct {
    char name[20];
    float vert[8];
    BOOL hover;
}TButton;

TButton btn[] = {
    {"start", {0,0,100,0,100,30,0,30}, FALSE},
    {"stop", {0,40,100,40,100,70,0,70}, FALSE},
    {"quit", {0,80,100,80,100,110,0,110}, FALSE}
};

int btnCnt = sizeof(btn) / sizeof(btn[0]);

void TButton_Show(TButton btn)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    if (btn.hover) glColor3f(1, 0, 0);
    else glColor3f(0, 1, 0);
    glVertexPointer(2, GL_FLOAT, 0, btn.vert);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

BOOL PointInButton(int x, int y, TButton btn)
{
    return (x > btn.vert[0]) && (x < btn.vert[4]) && (y > btn.vert[1]) && (y < btn.vert[5]);
}

void ShowMenu()
{
    glPushMatrix();
        glLoadIdentity();
        glOrtho(0, width1, height1, 0, -1, 1);
        for (int i = 0; i < btnCnt; i++)
            TButton_Show(btn[i]);
    glPopMatrix();
}

void DrawOs(float alfa)
{
    static float d = 0.05;
    glPushMatrix();
    glRotated(alfa, 0, 0, 1);
    glBegin(GL_LINES);
    glVertex2f(-1, 0);
    glVertex2f(1, 0);
    glVertex2f(1, 0);
    glVertex2f(1 - d, 0 + d);
    glVertex2f(1, 0);
    glVertex2f(1-d, 0-d);
    glEnd();
    glPopMatrix();
}

void Init(float start, float finish, int count)
{
    cnt = count;
    mas = (POINTFLOAT*)realloc(mas, sizeof(*mas) * cnt); 
    float dx = (finish - start) / (cnt - 1);
    for (int i = 0; i < cnt; i++)
    {
        mas[i].x = start;
        mas[i].y = sin(start);
        start += dx;
    }
}
 
void Show()
{
    float sx = 2.0 / (mas[cnt - 1].x - mas[0].x);
    float dx = (mas[cnt - 1].x + mas[0].x) * 0.5;
    glPushMatrix();
    glScalef(sx, scaleY, 1);
    glTranslatef(-dx, 0, 0);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < cnt; i++)
        glVertex2f(mas[i].x, mas[i].y);
    glEnd();
    glPopMatrix();
}

void Add(float x, float y)
{
    for (int i = 1; i < cnt; i++)
        mas[i - 1] = mas[i];
    mas[cnt - 1].x = x;
    mas[cnt - 1].y = y;

}

void ShowWorld()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &vert3);
    for (int i = -5; i < 5; i++)
        for (int j = -5; j < 5; j++)
        {
            glPushMatrix();
            if ((i + j) % 2 == 0) glColor3f(0, 0.5, 0);
            else glColor3f(1, 1, 1);
            glTranslatef(i * 2, j * 2, 0);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glPopMatrix();
        }

       
        glVertexPointer(3, GL_FLOAT, 0, &pyramid);
        //glPushMatrix();
        glColor3f(1, 1, 0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
        glTranslatef(2, 2, 0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
        glTranslatef(4, 6, 0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
        glTranslatef(-8, -6, 0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
    glDisableClientState(GL_VERTEX_ARRAY);


    
}

void MoveCamera()
{
    if (GetKeyState(VK_UP) < 0) xAlfa = ++xAlfa > 180 ? 180 : xAlfa;
    if (GetKeyState(VK_DOWN) < 0) xAlfa = --xAlfa < 0 ? 0 : xAlfa;
    if (GetKeyState(VK_LEFT) < 0) zAlfa++;
    if (GetKeyState(VK_RIGHT) <0) zAlfa--;
    
    float ugol = -zAlfa / 180 * M_PI;
    float speed=0;
    if (GetKeyState('W') < 0) speed = 0.1;
    if (GetKeyState('S') < 0) speed = -0.1;
    if (GetKeyState('A') < 0) { speed = 0.1; ugol -= M_PI * 0.5; }
    if (GetKeyState('D') < 0) { speed = 0.1; ugol += M_PI * 0.5; }

    if (speed !=0)
    {
        pos1.x += sin(ugol) * speed;
        pos1.y += cos(ugol) * speed;
    }

    glRotatef(-xAlfa, 1, 0, 0);
    glRotatef(-zAlfa, 0, 0, 1);
    glTranslatef(-pos1.x, -pos1.y, -3);
}

void Player_Move()
{
    Camera_MoveDirection(GetKeyState('W') < 0 ? 1 : (GetKeyState('S') < 0 ? -1 : 0),
        GetKeyState('D') < 0 ? 1 : (GetKeyState('A') < 0 ? -1 : 0),
        0.1+(buffs.speed.time>0?0.2:0));


    if (mouseBind)
        Camera_AutoMoveByMouse(400, 400, 0.2);

    camera_main.z = Map_GetHeight(camera_main.x, camera_main.y) + 1.7;
    //camera.z = Map_GetHeight(camera.x, camera.y) + 1.7;
}

float vertex[] = { -1,-1,1, 1,-1,1, 1,1,1, -1,1,1 };
float normal[] = { -1,-1,1, 1,-1,1, 1,1,1, -1,1,1 };

unsigned int texture,tex2;

int selectID = 0;

void Draw()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vertex);

        glNormalPointer(GL_FLOAT, 0, normal);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}


void Draw_Cube()
{
    glPushMatrix();
        glScalef(0.4, 0.4, 0.4);
        Draw();
        glRotatef(90, 1, 0, 0);
        Draw();
        glRotatef(90, 1, 0, 0);
        Draw();
        glRotatef(90, 1, 0, 0);
        Draw();
        glRotatef(90, 0, 1, 0);
        Draw();
        glRotatef(180, 0, 1, 0);
        Draw();
    glPopMatrix();
}

void Load_Texture(const char *fileName, GLuint *textureID)
{
    int width_t, height_t, cnt;
    unsigned char* data = stbi_load(fileName, &width_t, &height_t, &cnt, 0);

    glGenTextures(1, textureID);
    glBindTexture(GL_TEXTURE_2D, *textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_t, height_t,
                                    0, cnt == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
}

void Game_texture_Init()
{
    Load_Texture("wall.jpg", &texture);
    Load_Texture("pyramid.jpg", &tex2);
    glLoadIdentity();
    glFrustum(-0.1,0.1, -0.1,0.1,0.2,1000);
    glEnable(GL_DEPTH_TEST);

}

float vertex_text[] = { -1,-1,0, 1,-1,0, 1,1,0, -1,1,0,
                        -1,-1,2, 1,-1,2, 1,1,2, -1,1,2 };



float texCoord[] = { 0,1, 1,1, 0,1, 1,1, 
                     0,0, 1,0, 0,0, 1,0};

GLuint textuInd[] = { 0,1,5, 5,4,0, 1,2,6, 6,5,1, 2,3,7, 7,6,2, 3,0,4, 4,7,3 };
int textuIndCnt = sizeof(textuInd) / sizeof(GLuint);

float texCoord2[] = { 0,5, 5,5, 5,0, 0,0,
                      0,5, 5,5, 5,0, 0,0, };
GLuint texInd2[] = { 0,1,2, 2,3,0, 4,5,6, 6,7,4 };
int texIndCnt2 = sizeof(texInd2) / sizeof(GLuint);

float pyramid_home[] = { 0,0,1, -1,-1,0, 1,-1,0, 1,1,0, -1,1,0 };

float pyramidUV_home[] = { 2.5,0, 0,5, 5,5, 0,5, 5,5 };
GLuint pyramidInd_home[] = { 0,1,2, 0,2,3, 0,3,4, 0,4,1 };
int pyramidIndCnt_home = sizeof(pyramidInd_home) / sizeof(GLuint);

float figure[] = { 0,0,0, 1,0,0, 1,1,0, 0,1,0, 0,1,0, 1,1,0, 0.5,2,0 };
float fugureUV[] = {0,0.33, 0.2, 0.33, 0.2, 0, 0, 0,     0.2, 0.33, 0.4, 0.33, 0.3, 0};
GLuint figureInd[] = { 0,1,2, 2,3,0, 4,5,6 };
int figureIndCnt = sizeof(figureInd) / sizeof(GLuint);


void Game_text_Show()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    if (selectMode) glDisable(GL_TEXTURE_2D);
    else glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glColor3f(1, 1, 1);
    glPushMatrix();
    glRotatef(-70, 1, 0, 0);
    glRotatef(33, 0, 0, 1);
    glTranslatef(2, 3, -2);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, vertex_text);
        glTexCoordPointer(2, GL_FLOAT, 0, &texCoord);
            glPushMatrix();
            if (selectMode) glColor3ub(100, 0, 0);
            else if (selectID == 100) glColor3f(0, 1, 0);
            else glColor3f(1, 1, 1);
            glScalef(1, 1, 0.6);
            glDrawElements(GL_TRIANGLES, textuIndCnt, GL_UNSIGNED_INT, textuInd);
            glTexCoordPointer(2,GL_FLOAT,0,texCoord2);
            glBindTexture(GL_TEXTURE_2D, tex2);
            glDrawElements(GL_TRIANGLES, texIndCnt2, GL_UNSIGNED_INT, texInd2);
        glPopMatrix();

        glVertexPointer(3, GL_FLOAT, 0, pyramid_home);
        glTexCoordPointer(2, GL_FLOAT, 0, pyramidUV_home);
        glPushMatrix();
            if (selectMode) glColor3ub(200, 0, 0);
            else if (selectID == 200) glColor3f(0, 1, 0);
            else glColor3f(1, 1, 1);
            glTranslatef(0, 0, 1.2);
            glBindTexture(GL_TEXTURE_2D, tex2);
            glDrawElements(GL_TRIANGLES, pyramidIndCnt_home, GL_UNSIGNED_INT, pyramidInd_home);
        glPopMatrix();

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix();
}

void Player_Select(HWND hWnd, int x, int y)
{
    selectMode = TRUE;
    Game_text_Show();
    selectMode = FALSE;

    RECT rct; 
    GLubyte clr[3];
    GetClientRect(hWnd, &rct);
    glReadPixels(x, rct.bottom - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, clr);
    selectID = clr[0];
}

float vertex_18[] = { 0,0,0, 1,0,0, 1,1,0, 0,1,0 };
float greenCol[] = { 0,1,0,1, 0,1,0,1, 0,1,0,0, 0,1,0,0 };
float texCoord_18[] = { 0,1, 1,1, 1,0, 0,0 };
GLuint WindID;

void Game_18_Show()
{
    glPushMatrix();
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vertex_18);
        glTexCoordPointer(2, GL_FLOAT, 0, texCoord_18);

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, WindID);


            glColor3f(1, 1, 1);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        
            glTranslatef(-0.25, -0.25, -0.1);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        
            glTranslatef(-0.25, -0.25, -0.1);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix();
}

void Game_18_Init()
{
    Load_Texture("wind.png", &WindID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_ALPHA_TEST);
    //glAlphaFunc(GL_GREATER, 0.99);
    glOrtho(-1, 1, -1, 1, -1, 1);
    glEnable(GL_DEPTH_TEST);
}

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);
float getRand();
void ShowObj();
void ShowHome();
void HomeWork2_1();
void HomeWork2_2();
void ShowCircle();
void HomeWork3_1();

int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{

    WNDCLASSEX wcex;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
        L"GLSample",
        L"OpenGL Sample",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1100,
        700,
        NULL,
        NULL,
        hInstance,
        NULL);

    ShowWindow(hwnd, nCmdShow);
    SetCursor(wcex.hCursor);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);
    /*Game_New();

    srand(time(NULL));
    int mySeed = rand();

    curX = 10;
    Init(0, curX, 100);*/ //sapper
    
    /*glScalef(1/koef, 1, 1);
    Game_Init();*/ // volleyball
    /* program main loop */

    //glFrustum(-1, 1, -1, 1, 2, 80);
    //glEnable(GL_DEPTH_TEST);
    //glLoadIdentity();
    //glOrtho(-2, 2, -2, 2, -1, 1);
    //glFrustum(-1, 1, -1, 1, 2, 6);
    //glTranslatef(0, 0, -2);
    //glRotatef(2, 0, 1, 0);

    //Game_Shooter_Init();

    RECT rct; //for terrain
    GetClientRect(hwnd, &rct);
    WndResize(rct.right, rct.bottom);
    Map_terrain_Init();
    glEnable(GL_DEPTH_TEST);
    // Костыль, фиксит микролаг при первом разе нажатии на ЛКМ
    GLubyte clre[3];
    GetClientRect(hwnd, &rct);
    glReadPixels(rct.right / 2.0, rct.bottom / 2.0, 1, 1, GL_RGB,
        GL_UNSIGNED_BYTE, clre);




    /*glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-0.1, 0.1, -0.1, 0.1, 0.2, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);*/ //start learning lighting

    //Game_texture_Init();


   //Game_18_Init();
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */

            //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            /*HomeWork2_1();
            HomeWork2_2();*/
            /*if (mines == closedCell)
                Game_New();
            Game_Show();*/ //Игра сапер

            /*glLineWidth(2);
            glColor3f(1, 0, 0);
            DrawOs(0);
            glColor3f(0, 1, 0);
            DrawOs(90);

            curX += 0.1;
            Add(curX, sin(curX));

            glColor3f(0, 1, 1);
            Show();*/

            /*TBall_Move(&ball);
            Player_Move(player + 0, 'A', 'D', 'W', -koef, 0);
            Player_Move(player + 1, VK_LEFT, VK_RIGHT, VK_UP, 0,koef);


            Game_Show_volley();*/
            
            //glTranslatef(0, 0, -0.01);
            /*glRotatef(2, -0.5, 0.5, 0);
            glVertexPointer(3, GL_FLOAT, 0, &vert);
            glEnableClientState(GL_VERTEX_ARRAY);
                glColor3f(0, 1, 0);
                glDrawArrays(GL_TRIANGLES, 0, 3);
                
            glDisableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, &vert2);
            glEnableClientState(GL_VERTEX_ARRAY);
            glPushMatrix();
            glColor3f(1, 0, 0);
            glTranslatef(1, 0, -1);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glPopMatrix();
            glDisableClientState(GL_VERTEX_ARRAY);
            */
            /*glPushMatrix();
            MoveCamera();
            ShowWorld();
            glPopMatrix();*/ //3D

            //glPushMatrix();
            //    //glRotatef(theta, 0.0f, 0.0f, 1.0f);
            //    if (GetForegroundWindow() == hwnd)
            //        Player_Move();

            //    Camera_main_Apply();


            //    glBegin(GL_TRIANGLES);

            //        glColor3f(1.0f, 0.0f, 0.0f);   glVertex2f(0.0f, 1.0f);
            //        glColor3f(0.0f, 1.0f, 0.0f);   glVertex2f(0.87f, -0.5f);
            //        glColor3f(0.0f, 0.0f, 1.0f);   glVertex2f(-0.87f, -0.5f);

            //    glEnd();

            //glPopMatrix();

            /*ShowMenu();

            Game_Shooter_Show();
            Game_Shooter_Move();*/

            GetCursorPos(&mousePos);
            ScreenToClient(hwnd, &mousePos);

            if (GetForegroundWindow() == hwnd)
                Player_Move();

            Map_Terrain_Show(); //terrain
            Menu_Show();



            /*glPushMatrix();
                glRotatef(-60, 1, 0, 0);
                glRotatef(33, 0, 0, 1);
                glTranslatef(2, 3, -2);
                glBegin(GL_TRIANGLES);
                    glColor3f(1.0f, 0.0f, 0.0f); glVertex2f(0.0f, 1.0f);
                    glColor3f(0.0f, 1.0f, 0.0f); glVertex2f(0.87f, -0.5f);
                    glColor3f(0.0f, 0.0f, 1.0f); glVertex2f(-0.87f, -0.5f);
                glEnd();
            glPopMatrix();*/

/*            glPushMatrix();
                glRotatef(-60, 1, 0, 0);
                glRotatef(33, 0, 0, 1);
                glTranslatef(2, 3, -2);
                
                glPushMatrix();
                glRotatef(45, 0, 0, 1);
                glRotatef(theta, 0, 1, 0);
                    float position[] = { 0,0,1,0 };
                    glLightfv(GL_LIGHT0, GL_POSITION, position);

                    glTranslatef(0, 0, 1);
                    glScalef(0.2, 0.2, 0.2);
                    glColor3f(1, 1, 1);
                    Draw();
                glPopMatrix();
                
                glColor3f(0, 1, 0);
                Draw_Cube();

            glPopMatrix(); *///start learning lighting
            //Game_text_Show();

            //Game_18_Show();
            SwapBuffers(hDC);
                    
            theta += 1.0f;
            Sleep(1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_CREATE:
        Game_Create();
        break;

    case WM_SIZE:
        /*width1 = LOWORD(lParam);
        height1 = HIWORD(lParam);
        glViewport(0, 0, width1, height1);
        glLoadIdentity();
        k = LOWORD(lParam) / (float)HIWORD(lParam);
        glOrtho(-k, k, -1, 1, -1, 1);*/

        WndResize(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_MOUSEWHEEL:
        if ((int)wParam > 0) scaleY *= 1.5;
        else scaleY *= 0.7;
        if (scaleY < 0.02) scaleY = 0.02;
        break;


    case WM_MOUSEMOVE:
        for (int i = 0; i < btnCnt; i++)
            btn[i].hover = PointInButton(LOWORD(lParam), HIWORD(lParam), btn[i]);
        break;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        if (mouseBind)
            Player_Take(hwnd);
        else
        {
            Bag_Click(10, 10, 50, LOWORD(lParam), HIWORD(lParam), uMsg);
            CraftMenu_Click(LOWORD(lParam), HIWORD(lParam), uMsg);
        }
        //POINTFLOAT pf;
        //ScreeToOpenGL(hwnd, LOWORD(lParam), HIWORD(lParam), &pf.x, &pf.y);
        //int x = (int)pf.x;
        //int y = (int)pf.y;
        //if (IsCellInMap(x, y))
        //    map_sapper[x][y].flag=!map_sapper[x][y].flag;
    }break;

    case WM_SETCURSOR:
        ShowCursor(!mouseBind);
        break;


    case WM_DESTROY:
        return 0;

    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;

        case 'E':
            mouseBind = !mouseBind;
            SetCursorPos(400, 400);
            if (mouseBind)
                while (ShowCursor(FALSE) >= 0);
            else
                
                while (ShowCursor(TRUE) <= 0);
            break;
        }
    }
    break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}


float getRand()
{
    return (float)rand() / RAND_MAX * 2 - 1;
}

void ShowObj()
{
    glBegin(GL_TRIANGLES);
    glColor3f(0, 0, 0); glVertex2f(0, 0);
    glColor3f(1, 0, 0); glVertex2f(0.5, 0);
    glColor3f(0, 1, 0); glVertex2f(0, 0.5);
    glEnd();
}

void ShowHome()
{
    glPushMatrix();
    glBegin(GL_TRIANGLE_STRIP);
    glColor3f(0, light, 0);
    glVertex2f(-0.55, -0.55);
    glVertex2f(0.55, -0.55);
    glVertex2f(-0.55, 0.15);
    glVertex2f(0.55, 0.15);
    glEnd();
    glBegin(GL_TRIANGLES);
    glColor3f(light, 0, light);
    glVertex2f(-0.75, 0.15);
    glVertex2f(0.75, 0.15);
    glVertex2f(0, 0.65);
    glEnd();
    glBegin(GL_TRIANGLE_STRIP);
    glColor3f(light, light, light - 0.5);
    glVertex2f(-0.25, -0.25);
    glVertex2f(0.25, -0.25);
    glVertex2f(-0.25, 0.05);
    glVertex2f(0.25, 0.05);
    glEnd();
    glPopMatrix();
}

void HomeWork2_1()
{
    glPushMatrix();
    glLoadIdentity();
    glScalef(0.25, 0.25, 1);
    glTranslatef(3, -3, 0);
    ShowHome();
    glLoadIdentity();
    glScalef(0.5, 0.5, 1);
    glTranslatef(-1, -1, 0);
    ShowHome();
    glLoadIdentity();
    glScalef(0.45, 0.45, 1);
    glTranslatef(1, 0, 0);
    ShowHome();
    glPopMatrix();
}

void ShowCircle()
{
    float x, y;
    float cnt = 40;
    float l = 0.15;
    float a = M_PI * 2 / cnt;
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1, 1, 0);
    glVertex2f(0, 0);
    for (int i = -1; i < cnt; i++)
    {
        x = sin(a * i) * l;
        y = cos(a * i) * l;
        glVertex2f(x, y);
    }
    glEnd();
}
void HomeWork2_2()
{
    static float alfa = 0;
    alfa -= 2;
    glPushMatrix();
    glTranslatef(0, -1, 0);
    glRotatef(alfa, 0, 0, 1);
    glTranslatef(1.7, 0, 0);
    glColor3f(1, 1, 0);
    ShowCircle();
    glPopMatrix();
    light = sin(alfa / 180 * M_PI) * 0.45 + 0.5;
}

void HomeWork3_1()
{
    //glRotatef(2, 0.1, 0.1, 0.1);
    //glVertexPointer(2, GL_FLOAT, 0, &vertices);
    //glEnableClientState(GL_VERTEX_ARRAY);
    //glColorPointer(3, GL_FLOAT, 0, &Colors);
    //glEnableClientState(GL_COLOR_ARRAY);
    ////glColor3f(1, 1, 0);
    ////glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    //glDrawElements(GL_TRIANGLE_FAN, 6, GL_UNSIGNED_INT, &index);
    //glDisableClientState(GL_VERTEX_ARRAY);
    //glDisableClientState(GL_COLOR_ARRAY);


    float colors3_1[]={ 1,0,0, 0,1,0, 0,0,1, 1,1,0, 1,1,0, 1,1,0 };
    GLuint index3_1[] = {0,1,4,5};
    glVertexPointer(2, GL_FLOAT, 0, &pf3_1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glColorPointer(3, GL_FLOAT, 0, &colors3_1);
    glEnableClientState(GL_COLOR_ARRAY);
    //glColor3f(1, 1, 0);
    glDrawArrays(GL_LINE_STRIP, 0, 4);
    glDrawElements(GL_LINE_STRIP, 4, GL_UNSIGNED_INT, &index3_1);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    

}
