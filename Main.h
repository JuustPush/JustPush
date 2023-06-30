#pragma once

void Load_Texture(const char* fileName, GLuint* textureID);
float Map_GetHeight(float x, float y);

typedef struct {
    float x, y, z;
}TCell;
typedef struct {
    float r, g, b;
}TColor;

typedef struct {
    float u, v;
}TUV;

typedef struct {
    float x, y, z;
    float type;
    float scale;
} TObject;

typedef struct {
    TObject* items;
    int itemsCnt;
    int type;
}TObjGroup;

typedef struct {
    TObject* obj;
    float dx, dy, dz;
    int cnt;
}TAnim;

TAnim animation = {0,0,0,0,0};

#define mapWe 100
#define mapHe 100

TCell map[mapWe][mapHe];
TCell mapNormal[mapWe][mapHe];
TUV mapUV[mapWe][mapHe];

GLuint mapInd[mapWe - 1][mapHe - 1][6];
int mapIndCnt = sizeof(mapInd) / sizeof(GLuint);

float plant[] = { -0.5,0,0, 0.5,0,0, 0.5,0,1, -0.5,0,1,
                 0,-0.5,0, 0,0.5,0, 0,0.5,1, 0,-0.5,1 };
float plantUV[] = { 0,1, 1,1, 1,0, 0,0, 0,1, 1,1, 1,0, 0,0 };
GLuint plantInd[] = { 0,1,2, 2,3,0, 4,5,6, 6,7,4 };
int plantIndCnt = sizeof(plantInd) / sizeof(GLuint);

GLuint tex_pole, tex_trava, tex_flower, tex_flower2, tex_grib, tex_tree, tex_tree2;

TObject* plantMas = NULL;
int plantCnt;

GLuint tex_wood;
float cube[] = { 0,0,0, 1,0,0, 1,1,0, 0,1,0,
                 0,0,1, 1,0,1, 1,1,1, 0,1,1,
                 0,0,0, 1,0,0, 1,1,0, 0,1,0,
                 0,0,1, 1,0,1, 1,1,1, 0,1,1 };

float cubeUVlog[] = { 0.5,0.5, 1,0.5, 1,0, 0.5,0,
                     0.5,0.5, 1,0.5, 1,0, 0.5,0,
                     0,0.5, 0.5,0.5, 0,0.5, 0.5,0.5,
                     0,0,     0.5,0, 0,0,   0.5,0 };

float cubeUVleaf[] = { 0,1, 0.5,1, 0.5,0.5, 0,0.5,
                      0,1, 0.5,1, 0.5,0.5, 0,0.5,
                      0,0.5, 0.5,0.5, 0,0.5, 0.5,0.5,
                      0,1,   0.5,1,   0,1,   0.5,1};
GLuint cubeInd[] = { 0,1,2, 2,3,0, 4,5,6, 6,7,4, 8,9,13, 13,12,8,
                    9,10,14, 14,13,9, 10,11,15, 15,14,10, 11,8,12, 12,15,11 };
int cubeIndCnt = sizeof(cubeInd) / sizeof(GLuint);

TObjGroup* tree = NULL;
int treeCnt = 0;


void LoadTexture(const char* fileName, GLuint* target)
{
    int width, height, cnt;
    unsigned char* data = stbi_load(fileName, &width, &height, &cnt, 0);


    glGenTextures(1, target);
    glBindTexture(GL_TEXTURE_2D, *target);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
        0, cnt == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
}

float sun[] = { -1,-1,0, 1,-1,0, 1,1,0, -1,1,0 };

BOOL selectMode = FALSE;

#define ObjListCnt 255
typedef struct {
    int plantMas_Index;
    int colorIndex;
}TSelectObj;
TSelectObj selectMas[ObjListCnt];
int selectMasCnt = 0;

POINT scrSize;
float scrKoef;

typedef struct {
    int type;
    int x, y;
    int width, height;
}TSlot;
#define bagSize 16
TSlot bag[bagSize];

float bagRect[] = { 0,0, 1,0, 1,1, 0,1 };
float bagRectUV[] = { 0,0, 1,0, 1,1, 0,1 };

int health = 15;
int healthMax = 20;

float hearth[] = { 0.5,0.25, 0.25,0, 0,0.25, 0.5,1, 1,0.25, 0.75,0 };

BOOL mouseBind = TRUE;

static int hunger = 0;
static int tex_flowerEat = 0;
static int tex_flowerEat2 = 0;
static int bufTime_texFlower = 0;
static int bufTime_texFlower2 = 0;

typedef struct {
    int time;
    int timeMax;
} TBuff;

struct {
    TBuff speed;
    TBuff eye;
}buffs={0,0,0,0};

GLuint tex_ico_speed, tex_ico_eye;

int handItemType = 0;

POINT mousePos;

GLuint tex_ico_mortar, tex_ico_potion_eye,tex_ico_potion_speed, tex_ico_potion_life;

struct {
    int x, y;
    int width, height;
    TSlot items[3][3];
    TSlot itemOut;
    BOOL show;
}craft_menu;


typedef struct {
    int items[3][3];
    int itemOut;
}TRecipe;
TRecipe* recipe;
int recipeCnt = 0;




int Recipe_tex_ico_potion_life[3][3] = {
    0,              tex_grib,   0,
    tex_grib,       tex_grib,   tex_grib,
    0,              tex_grib,   0
};