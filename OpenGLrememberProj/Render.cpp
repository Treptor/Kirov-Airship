#include "Render.h"

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"



bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

int bombTrigger = 0, rotationMultiplier=5, rotationMultiplierV = 5, bombStablePhase1 = 0, bombStablePhase2 = 0;
double bombDegree = 0, bombDegreeMult = 1, bombX=0, bombZ = 0, bombZmult = 0.001, exploScale = 0;

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'B')
	{
		if(bombTrigger==0)
		{
			bombTrigger = 1;
			bombDegree = 0, bombDegreeMult = 3, bombX=0, bombZ = 0, bombZmult = 0.001, bombStablePhase1 = 0, bombStablePhase2 = 0, exploScale = 0;
		}
	}

	if (key == 'W')
	{
		if(rotationMultiplier<20)
			rotationMultiplier++;
	}
	if (key == 'S')
	{
		if (rotationMultiplier>-20)
			rotationMultiplier--;
	}

	if (key == 'E')
	{
		if (rotationMultiplierV<20)
			rotationMultiplierV++;
	}
	if (key == 'Q')
	{
		if (rotationMultiplierV>-20)
			rotationMultiplierV--;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}




//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	GLuint texId;
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
}

//палитра цветов
double clrRed[] = { 0.7,0,0 }, clrGrey[] = { 0.53,0.53,0.53 }, clrKhaki[] = {0.46,0.4,0.3};

double normX, normY, normZ;

void FindNormal(double* a, double* b, double* c)
{
	double normDivider;
	normX = (b[1] - a[1])*(c[2] - a[2]) - (b[2] - a[2])*(c[1] - a[1]);
	normY = (b[2] - a[2])*(c[0] - a[0]) - (b[0] - a[0])*(c[2] - a[2]);
	normZ = (b[0] - a[0])*(c[1] - a[1]) - (b[1] - a[1])*(c[0] - a[0]);
	normDivider = sqrt(normX*normX + normY*normY + normZ*normZ);
}

void DrawPlaneQuad(double *A, double *B, double *C, double *D)
{
	glBegin(GL_QUADS);
	FindNormal(A, B, C);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(D);
	glEnd();
}
void DrawPlaneQuadN(double *A, double *B, double *C, double *D)
{
	glBegin(GL_QUADS);
	FindNormal(A, B, C);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(D);
	glEnd();
}

void DrawPlaneTriangle(double *A, double *B, double *C)
{
	glBegin(GL_TRIANGLES);
	FindNormal(A, B, C);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);
	glEnd();
}

void DrawPlaneTriangleN(double *A, double *B, double *C)
{
	glBegin(GL_TRIANGLES);
	FindNormal(A, B, C);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);
	glEnd();
}

void DrawRotor()
{
	//axis base
	double axisFBL[] = { -0.1,-0.2,-0.2 }, axisFBR[] = { -0.1,0.2,-0.2 }, axisFTL[] = { -0.1,-0.2,0.2 }, axisFTR[] = { -0.1,0.2,0.2 };
	double axisBBL[] = { 0.1,-0.2,-0.2 }, axisBBR[] = { 0.1,0.2,-0.2 }, axisBTL[] = { 0.1,-0.2,0.2 }, axisBTR[] = { 0.1,0.2,0.2 };
	//top
	double rotorTopFTL[] = { 0,-0.1,1.2 }, rotorTopFTR[] = { 0,0.1,1.2 };
	//bottom
	double rotorBotFBL[] = { 0,-0.1,-1.2 }, rotorBotFBR[] = { 0,0.1,-1.2 };
	//left
	double rotorLftFBL[] = { 0,-1.2,-0.1 }, rotorLftFTL[] = { 0,-1.2,0.1 };
	//right
	double rotorRgtFBR[] = { 0,1.2,-0.1 }, rotorRgtFTR[] = { 0,1.2,0.1 };

	glBegin(GL_QUADS);
	glColor3dv(clrGrey);
	//ось
	FindNormal(axisFBL, axisFBR, axisFTR);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(axisFBL);
	glVertex3dv(axisFBR);
	glVertex3dv(axisFTR);
	glVertex3dv(axisFTL);

	FindNormal(axisBBL, axisBBR, axisBTR);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(axisBBL);
	glVertex3dv(axisBBR);
	glVertex3dv(axisBTR);
	glVertex3dv(axisBTL);

	glColor3dv(clrRed);
	//верхняя лопасть
	FindNormal(rotorTopFTL, rotorTopFTR, axisFTR);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(rotorTopFTL);
	glVertex3dv(rotorTopFTR);
	glVertex3dv(axisFTR);
	glVertex3dv(axisFTL);

	FindNormal(rotorTopFTL, rotorTopFTR, axisBTR);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(rotorTopFTL);
	glVertex3dv(rotorTopFTR);
	glVertex3dv(axisBTR);
	glVertex3dv(axisBTL);

	//нижняя лопасть
	FindNormal(rotorBotFBL, rotorBotFBR, axisFBR);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(rotorBotFBL);
	glVertex3dv(rotorBotFBR);
	glVertex3dv(axisFBR);
	glVertex3dv(axisFBL);

	FindNormal(rotorBotFBL, rotorBotFBR, axisBBR);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(rotorBotFBL);
	glVertex3dv(rotorBotFBR);
	glVertex3dv(axisBBR);
	glVertex3dv(axisBBL);

	//левая лопасть
	FindNormal(rotorLftFTL, rotorLftFBL, axisFBL);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(rotorLftFTL);
	glVertex3dv(rotorLftFBL);
	glVertex3dv(axisFBL);
	glVertex3dv(axisFTL);

	FindNormal(rotorLftFTL, rotorLftFBL, axisBBL);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(rotorLftFTL);
	glVertex3dv(rotorLftFBL);
	glVertex3dv(axisBBL);
	glVertex3dv(axisBTL);

	//правая лопасть
	FindNormal(rotorRgtFTR, rotorRgtFBR, axisFBR);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(rotorRgtFTR);
	glVertex3dv(rotorRgtFBR);
	glVertex3dv(axisFBR);
	glVertex3dv(axisFTR);

	FindNormal(rotorRgtFTR, rotorRgtFBR, axisBBR);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(rotorRgtFTR);
	glVertex3dv(rotorRgtFBR);
	glVertex3dv(axisBBR);
	glVertex3dv(axisBTR);

	glEnd();

	glBegin(GL_TRIANGLES);
	glColor3dv(clrRed);
	//верхняя лопасть
	FindNormal(rotorTopFTL, axisFTL, axisBTL);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(rotorTopFTL);
	glVertex3dv(axisFTL);
	glVertex3dv(axisBTL);

	FindNormal(rotorTopFTR, axisFTR, axisBTR);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(rotorTopFTR);
	glVertex3dv(axisFTR);
	glVertex3dv(axisBTR);


	//нижняя лопасть
	FindNormal(rotorBotFBL, axisFBL, axisBBL);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(rotorBotFBL);
	glVertex3dv(axisFBL);
	glVertex3dv(axisBBL);

	FindNormal(rotorBotFBR, axisFBR, axisBBR);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(rotorBotFBR);
	glVertex3dv(axisFBR);
	glVertex3dv(axisBBR);

	//левая лопасть
	FindNormal(rotorLftFTL, axisFTL, axisBTL);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(rotorLftFTL);
	glVertex3dv(axisFTL);
	glVertex3dv(axisBTL);

	FindNormal(rotorLftFBL, axisBBL, axisFBL);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(rotorLftFBL);
	glVertex3dv(axisBBL);
	glVertex3dv(axisFBL);

	//правая лопасть
	FindNormal(rotorRgtFTR, axisFTR, axisBTR);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(rotorRgtFTR);
	glVertex3dv(axisFTR);
	glVertex3dv(axisBTR);

	FindNormal(rotorRgtFBR, axisFBR, axisBBR);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(rotorRgtFBR);
	glVertex3dv(axisFBR);
	glVertex3dv(axisBBR);

	/*
	*/
	glEnd();
}

void DrawRotorBase()
{
	//axis base
	double axisFBL[] = { -0.1,-0.1,-0.1 }, axisFBR[] = { -0.1,0.1,-0.1 }, axisFTL[] = { -0.1,-0.1,0.1 }, axisFTR[] = { -0.1,0.1,0.1 };
	double axisBBL[] = { -0.4,-0.1,-0.1 }, axisBBR[] = { -0.4,0.1,-0.1 }, axisBTL[] = { -0.4,-0.1,0.1 }, axisBTR[] = { -0.4,0.1,0.1 };
	//rotor
	double tubeT1[] = { -0.4,0,-0.5 }, tubeT2[] = { -0.4,-0.4,-0.2 }, tubeT3[] = { -0.4,-0.4,0.2 }, tubeT4[] = { -0.4,0,0.5 }, tubeT5[] = { -0.4,0.4,0.2 }, tubeT6[] = { -0.4,0.4,-0.2 };
	double tubeB1[] = { -1.4,0,-0.25 }, tubeB2[] = { -1.4,-0.2,-0.1 }, tubeB3[] = { -1.4,-0.2,0.1 }, tubeB4[] = { -1.4,0,0.25 }, tubeB5[] = { -1.4,0.2,0.1 }, tubeB6[] = { -1.4,0.2,-0.1 };
	double tubeEnd[] = { -1.5,0,0 };
	//leg
	double legFLT[] = { -0.5,-1.8,0.35}, legFLB[] = { -1.3,-1.8,0.35 }, legFRB[] = { -1.1,0,0.1 }, legFRT[] = { -0.7,0,0.2 };
	double legBLT[] = { -0.5,-1.8,-0.35 }, legBLB[] = { -1.3,-1.8,-0.35 }, legBRB[] = { -1.1,0,-0.1 }, legBRT[] = { -0.7,0,-0.2 };

	glBegin(GL_QUADS);
	glColor3dv(clrGrey);
	FindNormal(axisFBL, axisBBL, axisBTL);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(axisFBL);
	glVertex3dv(axisBBL);
	glVertex3dv(axisBTL);
	glVertex3dv(axisFTL);

	FindNormal(axisFBR, axisBBR, axisBTR);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(axisFBR);
	glVertex3dv(axisBBR);
	glVertex3dv(axisBTR);
	glVertex3dv(axisFTR);

	FindNormal(axisFTL, axisFTR, axisBTR);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(axisFTL);
	glVertex3dv(axisFTR);
	glVertex3dv(axisBTR);
	glVertex3dv(axisBTL);

	FindNormal(axisFBL, axisFBR, axisBBR);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(axisFBL);
	glVertex3dv(axisFBR);
	glVertex3dv(axisBBR);
	glVertex3dv(axisBBL);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3d(1, 0, 0);
	FindNormal(tubeT1, tubeT3, tubeT6);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(tubeT1);
	glVertex3dv(tubeT2);
	glVertex3dv(tubeT3);
	glVertex3dv(tubeT4);
	glVertex3dv(tubeT5);
	glVertex3dv(tubeT6);

	glEnd();

	DrawPlaneQuad(tubeB1, tubeT1, tubeT2, tubeB2);
	DrawPlaneQuadN(tubeT2, tubeB2, tubeB3, tubeT3);
	DrawPlaneQuad(tubeB3, tubeT3, tubeT4, tubeB4);
	DrawPlaneQuadN(tubeT4, tubeB4, tubeB5, tubeT5);
	DrawPlaneQuad(tubeB5, tubeT5, tubeT6, tubeB6);
	DrawPlaneQuadN(tubeT6, tubeB6, tubeB1, tubeT1);

	DrawPlaneTriangle(tubeEnd, tubeB1, tubeB2);
	DrawPlaneTriangle(tubeEnd, tubeB2, tubeB3);
	DrawPlaneTriangle(tubeEnd, tubeB3, tubeB4);
	DrawPlaneTriangle(tubeEnd, tubeB4, tubeB5);
	DrawPlaneTriangle(tubeEnd, tubeB5, tubeB6);
	DrawPlaneTriangle(tubeEnd, tubeB6, tubeB1);

	glBegin(GL_QUADS);
	glColor3dv(clrGrey);
	FindNormal(legFLT, legFLB, legFRB);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(legFLT);
	glVertex3dv(legFLB);
	glVertex3dv(legFRB);
	glVertex3dv(legFRT);

	FindNormal(legBLT, legBLB, legBRB);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(legBLT);
	glVertex3dv(legBLB);
	glVertex3dv(legBRB);
	glVertex3dv(legBRT);

	FindNormal(legBLT, legFLT, legFRT);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(legBLT);
	glVertex3dv(legFLT);
	glVertex3dv(legFRT);
	glVertex3dv(legBRT);

	FindNormal(legBLB, legFLB, legFRB);
	glNormal3d(normX,normY, normZ);
	glVertex3dv(legBLB);
	glVertex3dv(legFLB);
	glVertex3dv(legFRB);
	glVertex3dv(legBRB);
	glEnd();
}

void DrawTailFin()
{
	double legFBL[] = { 0.5,-0.2,0 }, legFBR[] = { 0.5,0.2,0 }, legFTR[] = { 0,0.1,1.7 }, legFTL[] = { 0,-0.1,1.7 };
	double legBBL[] = { -0.5,-0.2,0 }, legBBR[] = { -0.5,0.2,0 }, legBTR[] = { -0.9,0.1,1.7 }, legBTL[] = { -0.9,-0.1,1.7 };

	double finA1B[] = { 0,0.5,1.7 }, finA2B[] = { 0,-0.5,1.7 }, finA3B[] = { -0.9,-0.5,1.7 }, finA4B[] = { -0.9, 0.5, 1.7 };
	double finR1B[] = { 0,0.5,1.7 }, finR2B[] = { -0.9,0.5,1.7 }, finR3B[] = { -0.9,1,1.55 }, finR4B[] = { 0, 1, 1.55 };
	double finL1B[] = { 0,-0.5,1.7 }, finL2B[] = { -0.9,-0.5,1.7 }, finL3B[] = { -0.9,-1,1.55 }, finL4B[] = { 0, -1, 1.55 };
	double fin2R1B[] = { 0, 1, 1.55 }, fin2R2B[] = { -0.9,1,1.55 }, fin2R3B[] = { -0.9,1.5,1.25 }, fin2R4B[] = { 0, 1.5, 1.25 };
	double fin2L1B[] = { 0, -1, 1.55 }, fin2L2B[] = { -0.9,-1,1.55 }, fin2L3B[] = { -0.9,-1.5,1.25 }, fin2L4B[] = { 0, -1.5, 1.25 };

	double finA1T[] = { 0,0.5,1.9 }, finA2T[] = { 0,-0.5,1.9 }, finA3T[] = { -0.9,-0.5,1.9 }, finA4T[] = { -0.9, 0.5, 1.9 };
	double finR1T[] = { 0,0.5,1.9 }, finR2T[] = { -0.9,0.5,1.9 }, finR3T[] = { -0.9,1,1.75 }, finR4T[] = { 0, 1, 1.75 };
	double finL1T[] = { 0,-0.5,1.9 }, finL2T[] = { -0.9,-0.5,1.9 }, finL3T[] = { -0.9,-1,1.75 }, finL4T[] = { 0, -1, 1.75 };
	double fin2R1T[] = { 0, 1, 1.75 }, fin2R2T[] = { -0.9,1,1.75 }, fin2R3T[] = { -0.9,1.5,1.45 }, fin2R4T[] = { 0, 1.5, 1.45 };
	double fin2L1T[] = { 0, -1, 1.75 }, fin2L2T[] = { -0.9,-1,1.75 }, fin2L3T[] = { -0.9,-1.5,1.45 }, fin2L4T[] = { 0, -1.5, 1.45 };

	DrawPlaneQuad(legFBL, legFBR, legFTR, legFTL);
	DrawPlaneQuadN(legBBL, legBBR, legBTR, legBTL);
	DrawPlaneQuad(legBBL, legFBL, legFTL, legBTL);
	DrawPlaneQuadN(legBBR, legFBR, legFTR, legBTR);
	DrawPlaneQuad(finA1B, finA2B, finA3B, finA4B);
	DrawPlaneQuad(finR1B, finR2B, finR3B, finR4B);
	DrawPlaneQuadN(finL1B, finL2B, finL3B, finL4B);
	DrawPlaneQuad(fin2R1B, fin2R2B, fin2R3B, fin2R4B);
	DrawPlaneQuadN(fin2L1B, fin2L2B, fin2L3B, fin2L4B);

	DrawPlaneQuadN(finA1T, finA2T, finA3T, finA4T);
	DrawPlaneQuadN(finR1T, finR2T, finR3T, finR4T);
	DrawPlaneQuad(finL1T, finL2T, finL3T, finL4T);
	DrawPlaneQuadN(fin2R1T, fin2R2T, fin2R3T, fin2R4T);
	DrawPlaneQuad(fin2L1T, fin2L2T, fin2L3T, fin2L4T);

	DrawPlaneQuad(finA1T, finA2T, finA2B, finA1B);
	DrawPlaneQuad(finA3T, finA4T, finA4B, finA3B);

	DrawPlaneQuad(finA1T, finA1B, fin2R1B, fin2R1T);
	DrawPlaneQuadN(finA4T, finA4B, fin2R2B, fin2R2T);

	DrawPlaneQuadN(fin2R4T, fin2R4B, fin2R1B, fin2R1T);
	DrawPlaneQuadN(fin2R2T, fin2R2B, fin2R3B, fin2R3T);

	DrawPlaneQuadN(fin2R3T, fin2R3B, fin2R4B, fin2R4T);

	DrawPlaneQuad(fin2L4T, fin2L4B, fin2L1B, fin2L1T);
	DrawPlaneQuad(fin2L2T, fin2L2B, fin2L3B, fin2L3T);

	DrawPlaneQuad(fin2L3T, fin2L3B, fin2L4B, fin2L4T);
	
	DrawPlaneQuadN(finL1T, finL1B, fin2L1B, fin2L1T);
	DrawPlaneQuad(finL2T, finL2B, fin2L2B, fin2L2T);
}

void DrawBomb()
{
	double nose[] = {2,0,0};
	double tubeA1[] = { 1.7,0,-0.25 }, tubeA2[] = { 1.7,-0.22,-0.1 }, tubeA3[] = { 1.7,-0.22,0.1 }, tubeA4[] = { 1.7,0,0.25 }, tubeA5[] = { 1.7,0.22,0.1 }, tubeA6[] = { 1.7,0.22,-0.1 };
	double tubeB1[] = { 0.7,0,-0.25 }, tubeB2[] = { 0.7,-0.22,-0.1 }, tubeB3[] = { 0.7,-0.22,0.1 }, tubeB4[] = { 0.7,0,0.25 }, tubeB5[] = { 0.7,0.22,0.1 }, tubeB6[] = { 0.7,0.22,-0.1 };
	double tubeC1[] = { 0.5,0,-0.18 }, tubeC2[] = { 0.5,-0.15,-0.08 }, tubeC3[] = { 0.5,-0.15,0.08 }, tubeC4[] = { 0.5,0,0.18 }, tubeC5[] = { 0.5,0.15,0.08 }, tubeC6[] = { 0.5,0.15,-0.08 };
	double tubeD1[] = { 0.25,0,-0.18 }, tubeD2[] = { 0.25,-0.16,-0.08 }, tubeD3[] = { 0.25,-0.16,0.08 }, tubeD4[] = { 0.25,0,0.18 }, tubeD5[] = { 0.25,0.16,0.08 }, tubeD6[] = { 0.25,0.16,-0.08 };
	double tailA1[] = { 0.15,0,-0.25 }, tailA2[] = { 0.15,-0.22,-0.1 }, tailA3[] = { 0.15,-0.22,0.1 }, tailA4[] = { 0.15,0,0.25 }, tailA5[] = { 0.15,0.22,0.1 }, tailA6[] = { 0.15,0.22,-0.1 };
	double tailB1[] = { 0,0,-0.25 }, tailB2[] = { 0,-0.22,-0.1 }, tailB3[] = { 0,-0.22,0.1 }, tailB4[] = { 0,0,0.25 }, tailB5[] = { 0,0.22,0.1 }, tailB6[] = { 0,0.22,-0.1 };

	DrawPlaneTriangleN(nose, tubeA1, tubeA2);
	DrawPlaneTriangle(nose, tubeA3, tubeA2);
	DrawPlaneTriangleN(nose, tubeA3, tubeA4);
	DrawPlaneTriangle(nose, tubeA5, tubeA4);
	DrawPlaneTriangleN(nose, tubeA5, tubeA6);
	DrawPlaneTriangle(nose, tubeA1, tubeA6);
	
	glColor3dv(clrKhaki);

	DrawPlaneQuadN(tubeA1, tubeB1, tubeB2, tubeA2);
	DrawPlaneQuad(tubeA3, tubeB3, tubeB2, tubeA2);
	DrawPlaneQuadN(tubeA3, tubeB3, tubeB4, tubeA4);
	DrawPlaneQuad(tubeA5, tubeB5, tubeB4, tubeA4);
	DrawPlaneQuadN(tubeA5, tubeB5, tubeB6, tubeA6);
	DrawPlaneQuad(tubeA1, tubeB1, tubeB6, tubeA6);

	DrawPlaneQuad(tubeC1, tubeB1, tubeB2, tubeC2);
	DrawPlaneQuadN(tubeC3, tubeB3, tubeB2, tubeC2);
	DrawPlaneQuad(tubeC3, tubeB3, tubeB4, tubeC4);
	DrawPlaneQuadN(tubeC5, tubeB5, tubeB4, tubeC4);
	DrawPlaneQuad(tubeC5, tubeB5, tubeB6, tubeC6);
	DrawPlaneQuadN(tubeC1, tubeB1, tubeB6, tubeC6);

	DrawPlaneQuadN(tubeC1, tubeD1, tubeD2, tubeC2);
	DrawPlaneQuad(tubeC3, tubeD3, tubeD2, tubeC2);
	DrawPlaneQuadN(tubeC3, tubeD3, tubeD4, tubeC4);
	DrawPlaneQuad(tubeC5, tubeD5, tubeD4, tubeC4);
	DrawPlaneQuadN(tubeC5, tubeD5, tubeD6, tubeC6);
	DrawPlaneQuad(tubeC1, tubeD1, tubeD6, tubeC6);

	DrawPlaneQuadN(tailA1, tailB1, tailB2, tailA2);
	DrawPlaneQuad(tailA3, tailB3, tailB2, tailA2);
	DrawPlaneQuadN(tailA3, tailB3, tailB4, tailA4);
	DrawPlaneQuad(tailA5, tailB5, tailB4, tailA4);
	DrawPlaneQuadN(tailA5, tailB5, tailB6, tailA6);
	DrawPlaneQuad(tailA1, tailB1, tailB6, tailA6);

	DrawPlaneQuadN(tailA1, tailA2, tubeD2, tubeD1);
	DrawPlaneQuadN(tailA3, tailA4, tubeD4, tubeD3);
	DrawPlaneQuadN(tailA5, tailA6, tubeD6, tubeD5);
}

void DrawRibsBig()
{

	double ribE1[] = { 5, 3.2, 0 }, ribE2[] = { 5, 2.56, -1.92 }, ribE3[] = { 5,0.96, -3.2 }, ribE4[] = { 5,-0.96, -3.2 }, ribE5[] = { 5, -2.56, -1.92 }, ribE6[] = { 5, -3.2, 0 },
		ribE7[] = { 5, -2.56, 1.92 }, ribE8[] = { 5,-0.96, 3.2 }, ribE9[] = { 5,0, 3.2 }, ribE10[] = { 5,0.96, 3.2 }, ribE11[] = { 5,0.96, 3.2 }, ribE12[] = { 5,2.56, 1.92 };
	double ribF1[] = { 5.9, 3.2, 0 }, ribF2[] = { 5.9, 2.56, -1.92 }, ribF3[] = { 5.9,0.96, -3.2 }, ribF4[] = { 5.9,-0.96, -3.2 }, ribF5[] = { 5.9, -2.56, -1.92 }, ribF6[] = { 5.9, -3.2, 0 },
		ribF7[] = { 5.9, -2.56, 1.92 }, ribF8[] = { 5.9,-0.96, 3.2 }, ribF9[] = { 5.9,0, 3.2 }, ribF10[] = { 5.9,0.96, 3.2 }, ribF11[] = { 5.9,0.96, 3.2 }, ribF12[] = { 5.9,2.56, 1.92 };

	double ribEbot1[] = { 5, 2.8, 0 }, ribEbot2[] = { 5, 2.24, -1.62 }, ribEbot3[] = { 5,0.81, -2.8 }, ribEbot4[] = { 5,-0.81, -2.8 }, ribEbot5[] = { 5, -2.24, -1.62 }, ribEbot6[] = { 5, -2.8, 0 },
		ribEbot7[] = { 5, -2.24, 1.62 }, ribEbot8[] = { 5,-0.81, 2.8 }, ribEbot9[] = { 5,0, 2.8 }, ribEbot10[] = { 5,0.81, 2.8 }, ribEbot11[] = { 5,0.81, 2.8 }, ribEbot12[] = { 5,2.24, 1.62 };
	double ribFbot1[] = { 5.9, 2.8, 0 }, ribFbot2[] = { 5.9, 2.24, -1.62 }, ribFbot3[] = { 5.9,0.81, -2.8 }, ribFbot4[] = { 5.9,-0.81, -2.8 }, ribFbot5[] = { 5.9, -2.24, -1.62 }, ribFbot6[] = { 5.9, -2.8, 0 },
		ribFbot7[] = { 5.9, -2.24, 1.62 }, ribFbot8[] = { 5.9,-0.81, 2.8 }, ribFbot9[] = { 5.9,0, 2.8 }, ribFbot10[] = { 5.9,0.81, 2.8 }, ribFbot11[] = { 5.9,0.81, 2.8 }, ribFbot12[] = { 5.9,2.24, 1.62 };
	glBegin(GL_QUADS);

	FindNormal(ribF1, ribF2, ribE2);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(ribF1);
	glVertex3dv(ribF2);
	glVertex3dv(ribE2);
	glVertex3dv(ribE1);

	FindNormal(ribF2, ribF3, ribE3);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(ribF2);
	glVertex3dv(ribF3);
	glVertex3dv(ribE3);
	glVertex3dv(ribE2);

	FindNormal(ribF3, ribF4, ribE4);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(ribF3);
	glVertex3dv(ribF4);
	glVertex3dv(ribE4);
	glVertex3dv(ribE3);

	FindNormal(ribF4, ribF5, ribE5);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(ribF4);
	glVertex3dv(ribF5);
	glVertex3dv(ribE5);
	glVertex3dv(ribE4);

	FindNormal(ribF5, ribF6, ribE6);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(ribF5);
	glVertex3dv(ribF6);
	glVertex3dv(ribE6);
	glVertex3dv(ribE5);

	FindNormal(ribF6, ribF7, ribE7);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(ribF6);
	glVertex3dv(ribF7);
	glVertex3dv(ribE7);
	glVertex3dv(ribE6);

	FindNormal(ribF7, ribF8, ribE8);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(ribF7);
	glVertex3dv(ribF8);
	glVertex3dv(ribE8);
	glVertex3dv(ribE7);

	FindNormal(ribF8, ribF9, ribE9);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(ribF8);
	glVertex3dv(ribF9);
	glVertex3dv(ribE9);
	glVertex3dv(ribE8);

	FindNormal(ribF9, ribF10, ribE10);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(ribF9);
	glVertex3dv(ribF10);
	glVertex3dv(ribE10);
	glVertex3dv(ribE9);

	FindNormal(ribF10, ribF11, ribE11);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(ribF10);
	glVertex3dv(ribF11);
	glVertex3dv(ribE11);
	glVertex3dv(ribE10);

	FindNormal(ribF11, ribF12, ribE12);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(ribF11);
	glVertex3dv(ribF12);
	glVertex3dv(ribE12);
	glVertex3dv(ribE11);

	FindNormal(ribF12, ribF1, ribE1);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(ribF12);
	glVertex3dv(ribF1);
	glVertex3dv(ribE1);
	glVertex3dv(ribE12);

	glEnd();

	glBegin(GL_QUADS);

	FindNormal(ribFbot1, ribF1, ribF2);
	glNormal3d(-normX, -normY, -normZ);
	glVertex3dv(ribFbot1);
	glVertex3dv(ribF1);
	glVertex3dv(ribF2);
	glVertex3dv(ribFbot2);

	glVertex3dv(ribFbot3);
	glVertex3dv(ribF3);
	glVertex3dv(ribF2);
	glVertex3dv(ribFbot2);

	glVertex3dv(ribFbot3);
	glVertex3dv(ribF3);
	glVertex3dv(ribF4);
	glVertex3dv(ribFbot4);

	glVertex3dv(ribFbot5);
	glVertex3dv(ribF5);
	glVertex3dv(ribF4);
	glVertex3dv(ribFbot4);

	glVertex3dv(ribFbot5);
	glVertex3dv(ribF5);
	glVertex3dv(ribF6);
	glVertex3dv(ribFbot6);

	glVertex3dv(ribFbot7);
	glVertex3dv(ribF7);
	glVertex3dv(ribF6);
	glVertex3dv(ribFbot6);

	glVertex3dv(ribFbot7);
	glVertex3dv(ribF7);
	glVertex3dv(ribF8);
	glVertex3dv(ribFbot8);

	glVertex3dv(ribFbot9);
	glVertex3dv(ribF9);
	glVertex3dv(ribF8);
	glVertex3dv(ribFbot8);

	glVertex3dv(ribFbot9);
	glVertex3dv(ribF9);
	glVertex3dv(ribF10);
	glVertex3dv(ribFbot10);

	glVertex3dv(ribFbot11);
	glVertex3dv(ribF11);
	glVertex3dv(ribF10);
	glVertex3dv(ribFbot10);

	glVertex3dv(ribFbot11);
	glVertex3dv(ribF11);
	glVertex3dv(ribF12);
	glVertex3dv(ribFbot12);

	glVertex3dv(ribFbot1);
	glVertex3dv(ribF1);
	glVertex3dv(ribF12);
	glVertex3dv(ribFbot12);

	glEnd();

	glBegin(GL_QUADS);

	FindNormal(ribEbot1, ribE1, ribE2);
	glNormal3d(normX, normY, normZ);
	glVertex3dv(ribEbot1);
	glVertex3dv(ribE1);
	glVertex3dv(ribE2);
	glVertex3dv(ribEbot2);

	glVertex3dv(ribEbot3);
	glVertex3dv(ribE3);
	glVertex3dv(ribE2);
	glVertex3dv(ribEbot2);

	glVertex3dv(ribEbot3);
	glVertex3dv(ribE3);
	glVertex3dv(ribE4);
	glVertex3dv(ribEbot4);

	glVertex3dv(ribEbot5);
	glVertex3dv(ribE5);
	glVertex3dv(ribE4);
	glVertex3dv(ribEbot4);

	glVertex3dv(ribEbot5);
	glVertex3dv(ribE5);
	glVertex3dv(ribE6);
	glVertex3dv(ribEbot6);

	glVertex3dv(ribEbot7);
	glVertex3dv(ribE7);
	glVertex3dv(ribE6);
	glVertex3dv(ribEbot6);

	glVertex3dv(ribEbot7);
	glVertex3dv(ribE7);
	glVertex3dv(ribE8);
	glVertex3dv(ribEbot8);

	glVertex3dv(ribEbot9);
	glVertex3dv(ribE9);
	glVertex3dv(ribE8);
	glVertex3dv(ribEbot8);

	glVertex3dv(ribEbot9);
	glVertex3dv(ribE9);
	glVertex3dv(ribE10);
	glVertex3dv(ribEbot10);

	glVertex3dv(ribEbot11);
	glVertex3dv(ribE11);
	glVertex3dv(ribE10);
	glVertex3dv(ribEbot10);

	glVertex3dv(ribEbot11);
	glVertex3dv(ribE11);
	glVertex3dv(ribE12);
	glVertex3dv(ribEbot12);

	glVertex3dv(ribEbot1);
	glVertex3dv(ribE1);
	glVertex3dv(ribE12);
	glVertex3dv(ribEbot12);

	glEnd();
}

void DrawRibsSmall()
{
	double ribF1[] = { 0, 2.8, 0 }, ribF2[] = { 0, 2.24, -1.62 }, ribF3[] = { 0,0.81, -2.8 }, ribF4[] = { 0,-0.81, -2.8 }, ribF5[] = { 0, -2.24, -1.62 }, ribF6[] = { 0, -2.8, 0 },
		ribF7[] = { 0, -2.24, 1.62 }, ribF8[] = { 0,-0.81, 2.8 }, ribF9[] = { 0,0, 2.8 }, ribF10[] = { 0,0.81, 2.8 }, ribF11[] = { 0,0.81, 2.8 }, ribF12[] = { 0,2.24, 1.62 };
	double ribG1[] = { -1, 2.8, 0 }, ribG2[] = { -1, 2.24, -1.62 }, ribG3[] = { -1,0.81, -2.8 }, ribG4[] = { -1,-0.81, -2.8 }, ribG5[] = { -1, -2.24, -1.62 }, ribG6[] = { -1, -2.8, 0 },
		ribG7[] = { -1, -2.24, 1.62 }, ribG8[] = { -1,-0.81, 2.8 }, ribG9[] = { -1,0, 2.8 }, ribG10[] = { -1,0.81, 2.8 }, ribG11[] = { -1,0.81, 2.8 }, ribG12[] = { -1,2.24, 1.62 };

double ribFbot1[] = { 0, 2.2, 0 }, ribFbot2[] = { 0, 1.76, -1.32 }, ribFbot3[] = { 0,0.66, -2.2 }, ribFbot4[] = { 0,-0.66, -2.2 }, ribFbot5[] = { 0, -1.76, -1.32 }, ribFbot6[] = { 0, -2.2, 0 },
ribFbot7[] = { 0, -1.76, 1.32 }, ribFbot8[] = { 0,-0.66, 2.2 }, ribFbot9[] = { 0,0, 2.2 }, ribFbot10[] = { 0,0.66, 2.2 }, ribFbot11[] = { 0,0.66, 2.2 }, ribFbot12[] = { 0,1.76, 1.32 };

double ribGbot1[] = { -0.9, 2.2, 0 }, ribGbot2[] = { -0.9, 1.76, -1.32 }, ribGbot3[] = { -0.9,0.66, -2.2 }, ribGbot4[] = { -0.9,-0.66, -2.2 }, ribGbot5[] = { -0.9, -1.76, -1.32 }, ribGbot6[] = { -0.9, -2.2, 0 },
ribGbot7[] = { -0.9, -1.76, 1.32 }, ribGbot8[] = { -0.9,-0.66, 2.2 }, ribGbot9[] = { -0.9,0, 2.2 }, ribGbot10[] = { -0.9,0.66, 2.2 }, ribGbot11[] = { -0.9,0.66, 2.2 }, ribGbot12[] = { -0.9,1.76, 1.32 };

glBegin(GL_QUADS);

FindNormal(ribF1, ribF2, ribG2);
glNormal3d(normX, normY, normZ);
glVertex3dv(ribF1);
glVertex3dv(ribF2);
glVertex3dv(ribG2);
glVertex3dv(ribG1);

FindNormal(ribF2, ribF3, ribG3);
glNormal3d(normX, normY, normZ);
glVertex3dv(ribF2);
glVertex3dv(ribF3);
glVertex3dv(ribG3);
glVertex3dv(ribG2);

FindNormal(ribF3, ribF4, ribG4);
glNormal3d(normX, normY, normZ);
glVertex3dv(ribF3);
glVertex3dv(ribF4);
glVertex3dv(ribG4);
glVertex3dv(ribG3);

FindNormal(ribF4, ribF5, ribG5);
glNormal3d(normX, normY, normZ);
glVertex3dv(ribF4);
glVertex3dv(ribF5);
glVertex3dv(ribG5);
glVertex3dv(ribG4);

FindNormal(ribF5, ribF6, ribG6);
glNormal3d(normX, normY, normZ);
glVertex3dv(ribF5);
glVertex3dv(ribF6);
glVertex3dv(ribG6);
glVertex3dv(ribG5);

FindNormal(ribF6, ribF7, ribG7);
glNormal3d(normX, normY, normZ);
glVertex3dv(ribF6);
glVertex3dv(ribF7);
glVertex3dv(ribG7);
glVertex3dv(ribG6);

FindNormal(ribF7, ribF8, ribG8);
glNormal3d(normX, normY, normZ);
glVertex3dv(ribF7);
glVertex3dv(ribF8);
glVertex3dv(ribG8);
glVertex3dv(ribG7);

FindNormal(ribF8, ribF9, ribG9);
glNormal3d(normX, normY, normZ);
glVertex3dv(ribF8);
glVertex3dv(ribF9);
glVertex3dv(ribG9);
glVertex3dv(ribG8);

FindNormal(ribF9, ribF10, ribG10);
glNormal3d(normX, normY, normZ);
glVertex3dv(ribF9);
glVertex3dv(ribF10);
glVertex3dv(ribG10);
glVertex3dv(ribG9);

FindNormal(ribF10, ribF11, ribG11);
glNormal3d(normX, normY, normZ);
glVertex3dv(ribF10);
glVertex3dv(ribF11);
glVertex3dv(ribG11);
glVertex3dv(ribG10);

FindNormal(ribF11, ribF12, ribG12);
glNormal3d(normX, normY, normZ);
glVertex3dv(ribF11);
glVertex3dv(ribF12);
glVertex3dv(ribG12);
glVertex3dv(ribG11);

FindNormal(ribF12, ribF1, ribG1);
glNormal3d(normX, normY, normZ);
glVertex3dv(ribF12);
glVertex3dv(ribF1);
glVertex3dv(ribG1);
glVertex3dv(ribG12);

glEnd();

glBegin(GL_QUADS);

FindNormal(ribFbot1, ribF1, ribF2);
glNormal3d(-normX, -normY, -normZ);
glVertex3dv(ribFbot1);
glVertex3dv(ribF1);
glVertex3dv(ribF2);
glVertex3dv(ribFbot2);

glVertex3dv(ribFbot3);
glVertex3dv(ribF3);
glVertex3dv(ribF2);
glVertex3dv(ribFbot2);

glVertex3dv(ribFbot3);
glVertex3dv(ribF3);
glVertex3dv(ribF4);
glVertex3dv(ribFbot4);

glVertex3dv(ribFbot5);
glVertex3dv(ribF5);
glVertex3dv(ribF4);
glVertex3dv(ribFbot4);

glVertex3dv(ribFbot5);
glVertex3dv(ribF5);
glVertex3dv(ribF6);
glVertex3dv(ribFbot6);

glVertex3dv(ribFbot7);
glVertex3dv(ribF7);
glVertex3dv(ribF6);
glVertex3dv(ribFbot6);

glVertex3dv(ribFbot7);
glVertex3dv(ribF7);
glVertex3dv(ribF8);
glVertex3dv(ribFbot8);

glVertex3dv(ribFbot9);
glVertex3dv(ribF9);
glVertex3dv(ribF8);
glVertex3dv(ribFbot8);

glVertex3dv(ribFbot9);
glVertex3dv(ribF9);
glVertex3dv(ribF10);
glVertex3dv(ribFbot10);

glVertex3dv(ribFbot11);
glVertex3dv(ribF11);
glVertex3dv(ribF10);
glVertex3dv(ribFbot10);

glVertex3dv(ribFbot11);
glVertex3dv(ribF11);
glVertex3dv(ribF12);
glVertex3dv(ribFbot12);

glVertex3dv(ribFbot1);
glVertex3dv(ribF1);
glVertex3dv(ribF12);
glVertex3dv(ribFbot12);

glEnd();

glBegin(GL_QUADS);

FindNormal(ribGbot1, ribG1, ribG2);
glNormal3d(normX, normY, normZ);
glVertex3dv(ribGbot1);
glVertex3dv(ribG1);
glVertex3dv(ribG2);
glVertex3dv(ribGbot2);

glVertex3dv(ribGbot3);
glVertex3dv(ribG3);
glVertex3dv(ribG2);
glVertex3dv(ribGbot2);

glVertex3dv(ribGbot3);
glVertex3dv(ribG3);
glVertex3dv(ribG4);
glVertex3dv(ribGbot4);

glVertex3dv(ribGbot5);
glVertex3dv(ribG5);
glVertex3dv(ribG4);
glVertex3dv(ribGbot4);

glVertex3dv(ribGbot5);
glVertex3dv(ribG5);
glVertex3dv(ribG6);
glVertex3dv(ribGbot6);

glVertex3dv(ribGbot7);
glVertex3dv(ribG7);
glVertex3dv(ribG6);
glVertex3dv(ribGbot6);

glVertex3dv(ribGbot7);
glVertex3dv(ribG7);
glVertex3dv(ribG8);
glVertex3dv(ribGbot8);

glVertex3dv(ribGbot9);
glVertex3dv(ribG9);
glVertex3dv(ribG8);
glVertex3dv(ribGbot8);

glVertex3dv(ribGbot9);
glVertex3dv(ribG9);
glVertex3dv(ribG10);
glVertex3dv(ribGbot10);

glVertex3dv(ribGbot11);
glVertex3dv(ribG11);
glVertex3dv(ribG10);
glVertex3dv(ribGbot10);

glVertex3dv(ribGbot11);
glVertex3dv(ribG11);
glVertex3dv(ribG12);
glVertex3dv(ribGbot12);

glVertex3dv(ribGbot1);
glVertex3dv(ribG1);
glVertex3dv(ribG12);
glVertex3dv(ribGbot12);

glEnd();
}

void DrawBaloon()
{
	double balG1[] = { -5, 1.9, 0 }, balG2[] = { -5, 1.52, -1.14 }, balG3[] = { -5,0.57, -1.9 }, balG4[] = { -5,-0.57, -1.9 }, balG5[] = { -5, -1.52, -1.14 }, balG6[] = { -5, -1.9, 0 },
		balG7[] = { -5, -1.52, 1.14 }, balG8[] = { -5,-0.57, 1.9 }, balG9[] = { -5,0, 1.9 }, balG10[] = { -5,0.57, 1.9 }, balG11[] = { -5,0.57, 1.9 }, balG12[] = { -5,1.52, 1.14 };
	double balH1[] = { -6.5, 0.8, 0 }, balH2[] = { -6.5, 0.64, -0.48 }, balH3[] = { -6.5,0.24, -0.8 }, balH4[] = { -6.5,-0.24, -0.8 }, balH5[] = { -6.5, -0.64, -0.48 }, balH6[] = { -6.5, -0.8, 0 },
		balH7[] = { -6.5, -0.64, 0.48 }, balH8[] = { -6.5,-0.24, 0.8 }, balH9[] = { -6.5,0, 0.8 }, balH10[] = { -6.5,0.24, 0.8 }, balH11[] = { -6.5,0.24, 0.8 }, balH12[] = { -6.5,0.64, 0.48 }; 
	double balF1[] = { 0, 2.5, 0 }, balF2[] = { 0, 2, -1.5 }, balF3[] = { 0,0.75, -2.5 }, balF4[] = { 0,-0.75, -2.5 }, balF5[] = { 0, -2, -1.5 }, balF6[] = { 0, -2.5, 0 },
		balF7[] = { 0, -2, 1.5 }, balF8[] = { 0,-0.75, 2.5 }, balF9[] = { 0,0, 2.5 }, balF10[] = { 0,0.75, 2.5 }, balF11[] = { 0,0.75, 2.5 }, balF12[] = { 0,2, 1.5 };
	double balE1[] = { 5.9, 2.9, 0 }, balE2[] = { 5.9, 2.32, -1.74 }, balE3[] = { 5.9,0.87, -2.9 }, balE4[] = { 5.9,-0.87, -2.9 }, balE5[] = { 5.9, -2.32, -1.74 }, balE6[] = { 5.9, -2.9, 0 },
		balE7[] = { 5.9, -2.32, 1.74 }, balE8[] = { 5.9,-0.87, 2.9 }, balE9[] = { 5.9,0, 2.9 }, balE10[] = { 5.9,0.87, 2.9 }, balE11[] = { 5.9,0.87, 2.9 }, balE12[] = { 5.9,2.32, 1.74 };
	double balD1[] = { 12, 2.9, 0 }, balD2[] = { 12, 2.32, -1.74 }, balD3[] = { 12,0.87, -2.9 }, balD4[] = { 12,-0.87, -2.9 }, balD5[] = { 12, -2.32, -1.74 }, balD6[] = { 12, -2.9, 0 },
		balD7[] = { 12, -2.32, 1.74 }, balD8[] = { 12,-0.87, 2.9 }, balD9[] = { 12,0, 2.9 }, balD10[] = { 12,0.87, 2.9 }, balD11[] = { 12,0.87, 2.9 }, balD12[] = { 12,2.32, 1.74 };
	double balC1[] = { 15, 2.6, 0 }, balC2[] = { 15, 2.08, -1.56 }, balC3[] = { 15,0.78, -2.6 }, balC4[] = { 15,-0.78, -2.6 }, balC5[] = { 15, -2.08, -1.56 }, balC6[] = { 15, -2.6, 0 },
		balC7[] = { 15, -2.08, 1.56 }, balC8[] = { 15,-0.78, 2.6 }, balC9[] = { 15,0, 2.6 }, balC10[] = { 15,0.78, 2.6 }, balC11[] = { 15,0.78, 2.6 }, balC12[] = { 15,2.08, 1.56 };
	double balB1[] = { 17, 1.9, 0 }, balB2[] = { 17, 1.52, -1.14 }, balB3[] = { 17,0.57, -1.9 }, balB4[] = { 17,-0.57, -1.9 }, balB5[] = { 17, -1.52, -1.14 }, balB6[] = { 17, -1.9, 0 },
		balB7[] = { 17, -1.52, 1.14 }, balB8[] = { 17,-0.57, 1.9 }, balB9[] = { 17,0, 1.9 }, balB10[] = { 17,0.57, 1.9 }, balB11[] = { 17,0.57, 1.9 }, balB12[] = { 17,1.52, 1.14 };
	double balA1[] = { 18, 0.8, 0 }, balA2[] = { 18, 0.64, -0.48 }, balA3[] = { 18,0.24, -0.8 }, balA4[] = { 18,-0.24, -0.8 }, balA5[] = { 18, -0.64, -0.48 }, balA6[] = { 18, -0.8, 0 },
		balA7[] = { 18, -0.64, 0.48 }, balA8[] = { 18,-0.24, 0.8 }, balA9[] = { 18,0, 0.8 }, balA10[] = { 18,0.24, 0.8 }, balA11[] = { 18,0.24, 0.8 }, balA12[] = { 18,0.64, 0.48 };
	double nose[] = {18.35,0,0};
	double back[] = { -7,0,0 };

	DrawPlaneTriangleN(back, balH2, balH1);
	DrawPlaneTriangleN(back, balH3, balH2);
	DrawPlaneTriangleN(back, balH4, balH3);
	DrawPlaneTriangleN(back, balH5, balH4);
	DrawPlaneTriangleN(back, balH6, balH5);
	DrawPlaneTriangleN(back, balH7, balH6);
	DrawPlaneTriangleN(back, balH8, balH7);
	DrawPlaneTriangleN(back, balH9, balH8);
	DrawPlaneTriangleN(back, balH10, balH9);
	DrawPlaneTriangleN(back, balH11, balH10);
	DrawPlaneTriangleN(back, balH12, balH11);
	DrawPlaneTriangleN(back, balH1, balH12);

	DrawPlaneQuadN(balH1, balH2, balG2, balG1);
	DrawPlaneQuadN(balH2, balH3, balG3, balG2);
	DrawPlaneQuadN(balH3, balH4, balG4, balG3);
	DrawPlaneQuadN(balH4, balH5, balG5, balG4);
	DrawPlaneQuadN(balH5, balH6, balG6, balG5);
	DrawPlaneQuadN(balH6, balH7, balG7, balG6);
	DrawPlaneQuadN(balH7, balH8, balG8, balG7);
	DrawPlaneQuadN(balH8, balH9, balG9, balG8);
	DrawPlaneQuadN(balH9, balH10, balG10, balG9);
	DrawPlaneQuadN(balH10, balH11, balG11, balG10);
	DrawPlaneQuadN(balH11, balH12, balG12, balG11);
	DrawPlaneQuadN(balH12, balH1, balG1, balG12);

	DrawPlaneQuad(balF1, balF2, balG2, balG1);
	DrawPlaneQuad(balF2, balF3, balG3, balG2);
	DrawPlaneQuad(balF3, balF4, balG4, balG3);
	DrawPlaneQuad(balF4, balF5, balG5, balG4);
	DrawPlaneQuad(balF5, balF6, balG6, balG5);
	DrawPlaneQuad(balF6, balF7, balG7, balG6);
	DrawPlaneQuad(balF7, balF8, balG8, balG7);
	DrawPlaneQuad(balF8, balF9, balG9, balG8);
	DrawPlaneQuad(balF9, balF10, balG10, balG9);
	DrawPlaneQuad(balF10, balF11, balG11, balG10);
	DrawPlaneQuad(balF11, balF12, balG12, balG11);
	DrawPlaneQuad(balF12, balF1, balG1, balG12);

	DrawPlaneQuadN(balF1, balF2, balE2, balE1);
	DrawPlaneQuadN(balF2, balF3, balE3, balE2);
	DrawPlaneQuadN(balF3, balF4, balE4, balE3);
	DrawPlaneQuadN(balF4, balF5, balE5, balE4);
	DrawPlaneQuadN(balF5, balF6, balE6, balE5);
	DrawPlaneQuadN(balF6, balF7, balE7, balE6);
	DrawPlaneQuadN(balF7, balF8, balE8, balE7);
	DrawPlaneQuadN(balF8, balF9, balE9, balE8);
	DrawPlaneQuadN(balF9, balF10, balE10, balE9);
	DrawPlaneQuadN(balF10, balF11, balE11, balE10);
	DrawPlaneQuadN(balF11, balF12, balE12, balE11);
	DrawPlaneQuadN(balF12, balF1, balE1, balE12);

	DrawPlaneQuad(balD1, balD2, balE2, balE1);
	DrawPlaneQuad(balD2, balD3, balE3, balE2);
	DrawPlaneQuad(balD3, balD4, balE4, balE3);
	DrawPlaneQuad(balD4, balD5, balE5, balE4);
	DrawPlaneQuad(balD5, balD6, balE6, balE5);
	DrawPlaneQuad(balD6, balD7, balE7, balE6);
	DrawPlaneQuad(balD7, balD8, balE8, balE7);
	DrawPlaneQuad(balD8, balD9, balE9, balE8);
	DrawPlaneQuad(balD9, balD10, balE10, balE9);
	DrawPlaneQuad(balD10, balD11, balE11, balE10);
	DrawPlaneQuad(balD11, balD12, balE12, balE11);
	DrawPlaneQuad(balD12, balD1, balE1, balE12);
	
	DrawPlaneQuadN(balD1, balD2, balC2, balC1);
	DrawPlaneQuadN(balD2, balD3, balC3, balC2);
	DrawPlaneQuadN(balD3, balD4, balC4, balC3);
	DrawPlaneQuadN(balD4, balD5, balC5, balC4);
	DrawPlaneQuadN(balD5, balD6, balC6, balC5);
	DrawPlaneQuadN(balD6, balD7, balC7, balC6);
	DrawPlaneQuadN(balD7, balD8, balC8, balC7);
	DrawPlaneQuadN(balD8, balD9, balC9, balC8);
	DrawPlaneQuadN(balD9, balD10, balC10, balC9);
	DrawPlaneQuadN(balD10, balD11, balC11, balC10);
	DrawPlaneQuadN(balD11, balD12, balC12, balC11);
	DrawPlaneQuadN(balD12, balD1, balC1, balC12);

	DrawPlaneQuad(balB1, balB2, balC2, balC1);
	DrawPlaneQuad(balB2, balB3, balC3, balC2);
	DrawPlaneQuad(balB3, balB4, balC4, balC3);
	DrawPlaneQuad(balB4, balB5, balC5, balC4);
	DrawPlaneQuad(balB5, balB6, balC6, balC5);
	DrawPlaneQuad(balB6, balB7, balC7, balC6);
	DrawPlaneQuad(balB7, balB8, balC8, balC7);
	DrawPlaneQuad(balB8, balB9, balC9, balC8);
	DrawPlaneQuad(balB9, balB10, balC10, balC9);
	DrawPlaneQuad(balB10, balB11, balC11, balC10);
	DrawPlaneQuad(balB11, balB12, balC12, balC11);
	DrawPlaneQuad(balB12, balB1, balC1, balC12);

	DrawPlaneQuadN(balB1, balB2, balA2, balA1);
	DrawPlaneQuadN(balB2, balB3, balA3, balA2);
	DrawPlaneQuadN(balB3, balB4, balA4, balA3);
	DrawPlaneQuadN(balB4, balB5, balA5, balA4);
	DrawPlaneQuadN(balB5, balB6, balA6, balA5);
	DrawPlaneQuadN(balB6, balB7, balA7, balA6);
	DrawPlaneQuadN(balB7, balB8, balA8, balA7);
	DrawPlaneQuadN(balB8, balB9, balA9, balA8);
	DrawPlaneQuadN(balB9, balB10, balA10, balA9);
	DrawPlaneQuadN(balB10, balB11, balA11, balA10);
	DrawPlaneQuadN(balB11, balB12, balA12, balA11);
	DrawPlaneQuadN(balB12, balB1, balA1, balA12);

	DrawPlaneTriangle(nose, balA2, balA1);
	DrawPlaneTriangle(nose, balA3, balA2);
	DrawPlaneTriangle(nose, balA4, balA3);
	DrawPlaneTriangle(nose, balA5, balA4);
	DrawPlaneTriangle(nose, balA6, balA5);
	DrawPlaneTriangle(nose, balA7, balA6);
	DrawPlaneTriangle(nose, balA8, balA7);
	DrawPlaneTriangle(nose, balA9, balA8);
	DrawPlaneTriangle(nose, balA10, balA9);
	DrawPlaneTriangle(nose, balA11, balA10);
	DrawPlaneTriangle(nose, balA12, balA11);
	DrawPlaneTriangle(nose, balA1, balA12);
}

void DrawCockpit()
{
	double baseBTL[] = { -0.5,-0.75,0 }, baseBTR[] = { -0.5,0.75,0 }, baseBBR[] = {0,0.5,-0.5 }, baseBBL[] = { 0,-0.5,-0.5 };
	double baseFTL[] = { 5.5,-0.75,0 }, baseFTR[] = { 5.5,0.75,0 }, baseFBR[] = { 5,0.5,-0.5 }, baseFBL[] = { 5,-0.5,-0.5 };
	double baseStrBBR[] = { 5,0.5,-1 }, baseStrBBL[] = { 5,-0.5,-1 },baseStrFBR[] = { 0,0.5,-1 }, baseStrFBL[] = { 0,-0.5,-1 };
	double wdwFrontTL[] = { 5.25,-0.75,-1.5 }, wdwFrontTR[] = { 5.25,0.75,-1.5 }, wdwFrontBL[] = { 5,-0.5,-2.2 }, wdwFrontBR[] = { 5,0.5,-2.2 };
	double wdwBackTL[] = { 0,-0.75,-1.5 }, wdwBackTR[] = { 0,0.75,-1.5 }, wdwBackBL[] = { 0.5,-0.5,-2.5 }, wdwBackBR[] = { 0.5,0.5,-2.5 };
	double btmMidL[] = { 3.5,-0.5,-2.5 }, btmMidR[] = { 3.5,0.5,-2.5 };
	double SideMidL[] = { 3.5,-0.8,-1.5 }, SideMidR[] = { 3.5,0.8,-1.5 };
	double SideTopL[] = { 3.5,-0.5,-1 }, SideTopR[] = { 3.5,0.5,-1 };
	
	DrawPlaneQuadN(baseFTL, baseFTR, baseFBR, baseFBL);
	DrawPlaneQuad(baseBTL, baseBTR, baseBBR, baseBBL);
	DrawPlaneQuad(baseFTL, baseBTL, baseBBL, baseFBL);
	DrawPlaneQuadN(baseFTR, baseBTR, baseBBR, baseFBR);
	DrawPlaneQuad(baseStrBBL, baseStrBBR, baseFBR, baseFBL);
	DrawPlaneQuadN(baseStrFBL, baseStrFBR, baseBBR, baseBBL);
	DrawPlaneQuad(baseStrBBL, baseFBL, baseBBL, baseStrFBL);
	DrawPlaneQuadN(baseStrBBR, baseFBR, baseBBR, baseStrFBR);
	DrawPlaneQuadN(baseStrBBL, baseStrBBR, wdwFrontTR, wdwFrontTL);
	DrawPlaneQuadN(wdwFrontTL, wdwFrontTR, wdwFrontBR, wdwFrontBL);
	DrawPlaneQuad(baseStrFBL, baseStrFBR, wdwBackTR, wdwBackTL);
	DrawPlaneQuad(wdwBackTL, wdwBackTR, wdwBackBR, wdwBackBL);
	DrawPlaneQuadN(btmMidL, btmMidR, wdwBackBR, wdwBackBL);
	DrawPlaneQuad(btmMidL, btmMidR, wdwFrontBR, wdwFrontBL);
	DrawPlaneQuadN(SideMidR, btmMidR, wdwFrontBR, wdwFrontTR);
	DrawPlaneQuad(SideMidL, btmMidL, wdwFrontBL, wdwFrontTL);
	DrawPlaneQuad(SideMidR, btmMidR, wdwBackBR, wdwBackTR);
	DrawPlaneQuadN(SideMidL, btmMidL, wdwBackBL, wdwBackTL);

	DrawPlaneQuadN(SideMidL, SideTopL, baseStrBBL, wdwFrontTL);
	DrawPlaneQuad(SideMidR, SideTopR, baseStrBBR, wdwFrontTR);
	DrawPlaneQuad(SideMidL, SideTopL, baseStrFBL, wdwBackTL);
	DrawPlaneQuadN(SideMidR, SideTopR, baseStrFBR, wdwBackTR);

}

void DrawBombStack()
{
	double baloonBaseFLT[] = { 4,-0.75,2 }, baloonBaseFRT[] = { 4,0.75,2 }, baloonBaseFLB[] = { 4,-0.75,0 }, baloonBaseFRB[] = { 4,0.75,0 };
	double baloonBaseBLT[] = { 0.5,-0.75,2 }, baloonBaseBRT[] = { 0.5,0.75,2 }, baloonBaseBLB[] = { 0.5,-0.75,0 }, baloonBaseBRB[] = { 0.5,0.75,0 };
	double BaseFLB[] = { 3.65,-0.38,-0.5 }, BaseFRB[] = { 3.65,0.38,-0.5 };
	double BaseBLB[] = { 0.85,-0.38,-0.5 }, BaseBRB[] = { 0.85,0.38,-0.5 };
	double holderFRT[] = {2.8,-0.38,-0.5}, holderBRT[] = { 2.55,-0.38,-0.5 }, holderFRB[] = { 2.8,-0.38,-3 }, holderBRB[] = { 2.55,-0.38,-3 };
	double holderFLT[] = { 2.8,0.38,-0.5 }, holderBLT[] = { 2.55,0.38,-0.5 }, holderFLB[] = { 2.8,0.38,-3 }, holderBLB[] = { 2.55,0.38,-3 };
	double holderCurveFLB[] = { 2.8,0.18,-3.25 }, holderCurveBLB[] = { 2.55,0.18,-3.25 };
	double holderCurveFRB[] = { 2.8,-0.18,-3.25 }, holderCurveBRB[] = { 2.55,-0.18,-3.25 };
	double railFLT[] = { 0.95,-0.3,-0.5 }, railFLB[] = {0.95,-0.3,-3.25}, railBLB[] = { 0.5,-0.75,-3.75 };
	double railFRT[] = { 0.95,0.3,-0.5 }, railFRB[] = { 0.95,0.3,-3.25 }, railBRB[] = { 0.5,0.75,-3.75 };
	double tailBaseBLB[] = { -0.5,-0.3,-3.75}, tailBaseBRB[] = { -0.5,0.3,-3.75 }, tailBaseBLT[] = { -0.5,-0.3,2}, tailBaseBRT[] = { -0.5,0.3,2 };
	double tailCurveBLB[] = { -0.5,-0.3,-1.5 }, tailCurveBRB[] = { -0.5,0.3,-1.5 }, tailCurveBLT[] = { -1.55,-0.3,0.35 }, tailCurveBRT[] = { -1.55,0.3,0.35 };
	double tailBot[] = {-2.2,0,-3.25}, tailTop[] = { -2.2,0,-2 };

	DrawPlaneQuadN(baloonBaseFLT, baloonBaseFRT, baloonBaseFRB, baloonBaseFLB);
	DrawPlaneQuad(baloonBaseBLT, baloonBaseBRT, baloonBaseBRB, baloonBaseBLB);
	DrawPlaneQuadN(baloonBaseBLT, baloonBaseFLT, baloonBaseFLB, baloonBaseBLB);
	DrawPlaneQuad(baloonBaseBRT, baloonBaseFRT, baloonBaseFRB, baloonBaseBRB);

	DrawPlaneQuad(BaseFLB, BaseFRB, baloonBaseFRB, baloonBaseFLB);
	DrawPlaneQuadN(BaseBLB, BaseBRB, baloonBaseBRB, baloonBaseBLB);
	DrawPlaneQuad(BaseBLB, BaseFLB, baloonBaseFLB, baloonBaseBLB);
	DrawPlaneQuadN( BaseBRB, BaseFRB, baloonBaseFRB, baloonBaseBRB);
	DrawPlaneQuad(BaseBRB, BaseFRB, BaseFLB, BaseBLB);

	DrawPlaneQuad(holderFRT, holderBRT, holderBRB, holderFRB);
	DrawPlaneQuadN(holderFLT, holderBLT, holderBLB, holderFLB);
	DrawPlaneQuad(holderCurveFLB, holderCurveBLB, holderBLB, holderFLB);
	DrawPlaneQuadN(holderCurveFRB, holderCurveBRB, holderBRB, holderFRB);

	DrawPlaneQuadN(railFLT, railFLB, railBLB, baloonBaseBLB);
	DrawPlaneQuad(railFRT, railFRB, railBRB, baloonBaseBRB);
	DrawPlaneQuad(baloonBaseBLB, railBLB, railBRB, baloonBaseBRB);

	DrawPlaneQuad(tailBaseBLB, railBLB, baloonBaseBLT, tailBaseBLT);
	DrawPlaneQuadN(tailBaseBRB, railBRB, baloonBaseBRT, tailBaseBRT);
	DrawPlaneQuadN(railBLB, railBRB, tailBaseBRB, tailBaseBLB);

	DrawPlaneTriangleN(tailCurveBLB, tailCurveBLT, tailBaseBLT);
	DrawPlaneTriangle(tailCurveBRB, tailCurveBRT, tailBaseBRT);
	DrawPlaneQuadN(tailCurveBRB, tailCurveBRT, tailCurveBLT, tailCurveBLB);

	DrawPlaneQuad(tailTop , tailBot, tailBaseBLB, tailCurveBLB);
	DrawPlaneQuadN(tailTop, tailBot, tailBaseBRB, tailCurveBRB);
	DrawPlaneTriangleN(tailTop, tailCurveBRB, tailCurveBLB);
	DrawPlaneTriangle(tailBot, tailBaseBRB, tailBaseBLB);
}

void DrawExplosion()
{
	double Low1[] = { 5,0,0 }, Low2[] = { 3.5,3.5,0 }, Low3[] = { 0,5,0 }, Low4[] = { -3.5,3.5,0 }, Low5[] = { -5,0,0 }, Low6[] = { -3.5,-3.5,0 }, Low7[] = { 0,-5,0 }, Low8[] = { 3.5,-3.5,0 };
	double Mid1[] = { 3,0,2 }, Mid2[] = { 2.1,2.1,2 }, Mid3[] = { 0,3,2 }, Mid4[] = { -2.1,2.1,2 }, Mid5[] = { -3,0,2 }, Mid6[] = { -2.1,-2.1,2}, Mid7[] = { 0,-3,2 }, Mid8[] = { 2.1,-2.1,2 };
	double VLow1[] = { 7,0,-3 }, VLow2 [] = { 4.9,4.9,-3 }, VLow3[] = { 0,7,-3 }, VLow4[] = { -4.9,4.9,-3 }, VLow5[] = { -7,0,-3 }, VLow6[] = { -4.9,-4.9,-3 }, VLow7[] = { 0,-7,-3 }, VLow8[] = { 4.9,-4.9,-3 };
	double Top[] = { 0, 0, 3 };

	DrawPlaneQuad(Low1, Low2, Mid2, Mid1);
	DrawPlaneQuad(Low2, Low3, Mid3, Mid2);
	DrawPlaneQuad(Low3, Low4, Mid4, Mid3);
	DrawPlaneQuad(Low4, Low5, Mid5, Mid4);
	DrawPlaneQuad(Low5, Low6, Mid6, Mid5);
	DrawPlaneQuad(Low6, Low7, Mid7, Mid6);
	DrawPlaneQuad(Low7, Low8, Mid8, Mid7);
	DrawPlaneQuadN(Low1, Low8, Mid8, Mid1);

	DrawPlaneQuadN(Low1, Low2, VLow2, VLow1);
	DrawPlaneQuadN(Low2, Low3, VLow3, VLow2);
	DrawPlaneQuadN(Low3, Low4, VLow4, VLow3);
	DrawPlaneQuadN(Low4, Low5, VLow5, VLow4);
	DrawPlaneQuadN(Low5, Low6, VLow6, VLow5);
	DrawPlaneQuadN(Low6, Low7, VLow7, VLow6);
	DrawPlaneQuadN(Low7, Low8, VLow8, VLow7);
	DrawPlaneQuad(Low1, Low8, VLow8, VLow1);

	DrawPlaneTriangle(Mid1, Mid2, Top);
	DrawPlaneTriangleN(Mid3, Mid2, Top);
	DrawPlaneTriangle(Mid3, Mid4, Top);
	DrawPlaneTriangleN(Mid5, Mid4, Top);
	DrawPlaneTriangle(Mid5, Mid6, Top);
	DrawPlaneTriangleN(Mid7, Mid6, Top);
	DrawPlaneTriangle(Mid7, Mid8, Top);
	DrawPlaneTriangleN(Mid1, Mid8, Top);
}

double RotorRotation = 0, RotorRotationV = 0;

void BombDrop()
{
	if (bombZ < 50)
	{
		bombZ += bombZmult;
		bombZmult += 0.02;
		bombX += 0.05*bombZmult*rotationMultiplier;
		if (bombStablePhase1 == 0)
		{
			if (bombDegree < 60)
			{
				bombDegree += bombDegreeMult*1;
				bombDegreeMult -= 0.01;
			}
			else
			{
				bombStablePhase1 = 1;
				bombDegree = 60;
			}
		}
	}
	else
	{
		if (exploScale < 3)
			exploScale += 0.3;
		else
			bombTrigger = 0;
	}
}

void Render(OpenGL *ogl)
{       

	GLfloat amb[] = { 0.33, 0.33, 0.33, 1. };
	GLfloat dif[] = { 0.47, 0.47, 0.47, 1. };
	GLfloat spec[] = { 0.53, 0.53, 0.53, 1. };
	GLfloat sh = 0.9f * 256;
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
	{
		glEnable(GL_TEXTURE_2D);
	}

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	
	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);\
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

    //чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  

	//текст сообщения вверху слева, если надоест - закоментировать, или заменить =)
	/*
	glPushMatrix();
	char c[250];  //максимальная длина сообщения
	sprintf_s(c, "(T)Текстуры - %d\n(L)Свет - %d\n\nУправление светом:\n"
		, textureMode, lightMode);
	ogl->message = std::string(c);
	glPopMatrix();
	*/
	
	glPushMatrix();
	glTranslated(-4.02, 0, -3.1); //координаты реального размещения
	DrawBombStack();
	glPopMatrix();

	glPushMatrix();
	glTranslated(1.05, 0, -2.85); //координаты реального размещения
	DrawCockpit();
	glPopMatrix();

	glPushMatrix();
	glTranslated(-10, 0, 1.4);
	DrawTailFin();
	glPopMatrix();

	glPushMatrix();
	glTranslated(-10, -1.4, 0);
	glRotated(90, 1, 0, 0);
	DrawTailFin();
	glPopMatrix();

	glPushMatrix();
	glTranslated(-10, 1.4, 0);
	glRotated(-90, 1, 0, 0);
	DrawTailFin();
	glPopMatrix();

	glPushMatrix();
	glTranslated(-10, 0, -1.4);
	glRotated(180, 1, 0, 0);
	DrawTailFin();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.45, 5, 1.1);
	glScaled(1.2, 1.2, 1.2);
	glRotated(-90, 0, 1, 0);
	DrawRotorBase();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.45, -5, 1.1);
	glScaled(1.2, 1.2, 1.2);
	glRotated(180, 0, 0, 1);
	glRotated(-90, 0, 1, 0);
	DrawRotorBase();
	glPopMatrix();

	glPushMatrix();
	glTranslated(-3.94, 4.4, 0);
	glScaled(1.2, 1.2, 1.2);
	DrawRotorBase();
	glPopMatrix();

	glPushMatrix();
	glTranslated(-3.94, 0, 4.4);
	glScaled(1.2, 1.2, 1.2);
	glRotated(90, 1, 0, 0);
	DrawRotorBase();
	glPopMatrix();
	
	glPushMatrix();
	glTranslated(-3.94, -4.4, 0);
	glScaled(1.2, 1.2, 1.2);
	glRotated(180, 0, 1, 0);
	glRotated(180, 0, 0, 1);
	DrawRotorBase();
	glPopMatrix();

	if (textureMode)
	{
		//настройка материала
		GLfloat amb[] = { 0.45, 0.39, 0.28, 1. };
		GLfloat dif[] = { 0.36, 0.32, 0.24, 1. };
		GLfloat spec[] = { 0.59, 0.52, 0.46, 1. };
		GLfloat sh = 0.4f * 256;
		glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
		glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
		glMaterialf(GL_FRONT, GL_SHININESS, sh);
	}

	glPushMatrix();
	glTranslated(-5, 0, 0); //координаты реального размещения
	DrawBaloon();
	glPopMatrix();

	if (bombTrigger == 0)
	{
		//bombstack
		glPushMatrix();
		glTranslated(-3.62, 0, -3.2);
		glScaled(1.5, 1.5, 1.5);
		glRotated(90, 1, 0, 0);
		DrawBomb();
		glPopMatrix();

		glPushMatrix();
		glTranslated(-3.62, 0, -3.9);
		glScaled(1.5, 1.5, 1.5);
		glRotated(90, 1, 0, 0);
		DrawBomb();
		glPopMatrix();

		glPushMatrix();
		glTranslated(-3.62, 0, -4.6);
		glScaled(1.5, 1.5, 1.5);
		glRotated(90, 1, 0, 0);
		DrawBomb();
		glPopMatrix();

		glPushMatrix();
		glTranslated(-3.62, 0, -5.3);
		glScaled(1.5, 1.5, 1.5);
		glRotated(90, 1, 0, 0);
		DrawBomb();
		glPopMatrix();

		glPushMatrix();
		glTranslated(-3.62, 0, -6);
		glScaled(1.5, 1.5, 1.5);
		glRotated(90, 1, 0, 0);
		DrawBomb();
		glPopMatrix();
	}
	else
	{		//bombstack
		if(bombZ<=0.7)
		{ 
		glPushMatrix();
		glTranslated(-3.62, 0, -3.2 - bombZ);
		glScaled(1.5, 1.5, 1.5);
		glRotated(90, 1, 0, 0);
		DrawBomb();
		glPopMatrix();

		glPushMatrix();
		glTranslated(-3.62, 0, -3.9 - bombZ);
		glScaled(1.5, 1.5, 1.5);
		glRotated(90, 1, 0, 0);
		DrawBomb();
		glPopMatrix();

		glPushMatrix();
		glTranslated(-3.62, 0, -4.6 - bombZ);
		glScaled(1.5, 1.5, 1.5);
		glRotated(90, 1, 0, 0);
		DrawBomb();
		glPopMatrix();

		glPushMatrix();
		glTranslated(-3.62, 0, -5.3 - bombZ);
		glScaled(1.5, 1.5, 1.5);
		glRotated(90, 1, 0, 0);
		DrawBomb();
		glPopMatrix();
		}
		else
		{
			glPushMatrix();
			glTranslated(-3.62, 0, -3.9);
			glScaled(1.5, 1.5, 1.5);
			glRotated(90, 1, 0, 0);
			DrawBomb();
			glPopMatrix();

			glPushMatrix();
			glTranslated(-3.62, 0, -4.6);
			glScaled(1.5, 1.5, 1.5);
			glRotated(90, 1, 0, 0);
			DrawBomb();
			glPopMatrix();

			glPushMatrix();
			glTranslated(-3.62, 0, -5.3);
			glScaled(1.5, 1.5, 1.5);
			glRotated(90, 1, 0, 0);
			DrawBomb();
			glPopMatrix();

			glPushMatrix();
			glTranslated(-3.62, 0, -6);
			glScaled(1.5, 1.5, 1.5);
			glRotated(90, 1, 0, 0);
			DrawBomb();
			glPopMatrix();
		}

			BombDrop();
			glPushMatrix();
			glTranslated(-3.62-bombX, 0, -6- bombZ);
			glScaled(1.5, 1.5, 1.5);
			glRotated(bombDegree, 0, 1, 0);
			glRotated(90, 1, 0, 0);
			DrawBomb();
			glPopMatrix();

		if (textureMode)
			{
				//настройка материала
				GLfloat amb[] = { 1, 0.86, 0, 1. };
				GLfloat dif[] = { 1, 0.86, 0, 1. };
				GLfloat spec[] = { 1, 0.86, 0, 1. };
				GLfloat sh = 0.15f * 256;
				glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
				glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
				glMaterialf(GL_FRONT, GL_SHININESS, sh);
			}
			glPushMatrix();
			glTranslated(-3.62 - bombX, 0, 3-bombZ);
			glScaled(exploScale, exploScale, exploScale);
			DrawExplosion();
			glPopMatrix();
	}

	if (textureMode)
	{
		//настройка материала
		GLfloat amb[] = { 0.49, 0.04, 0.08, 1. };
		GLfloat dif[] = { 0.79, 0, 0.02, 1. };
		GLfloat spec[] = { 0.94, 0.25, 0.29, 1. };
		GLfloat sh = 0.4f * 256;
		glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
		glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
		glMaterialf(GL_FRONT, GL_SHININESS, sh);
	}

	glPushMatrix();
	glTranslated(-4.55, 0, 0);
	glScaled(1.05, 1, 1);
	DrawRibsSmall();
	glPopMatrix();

	glPushMatrix();
	glTranslated(-5, 0, 0); //координаты реального размещения
	DrawRibsBig();
	glPopMatrix();

	glPushMatrix();
	glTranslated(1.2, 0, 0);
	DrawRibsBig();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.45, 5, 1.1); //координаты реального размещения
	glRotated(RotorRotationV, 0, 0, 1);
	glScaled(1.2, 1.2, 1.2);
	glRotated(90, 0, 1, 0);
	DrawRotor();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.45, -5, 1.1); //координаты реального размещения
	glRotated(-RotorRotationV, 0, 0, 1);
	glScaled(1.2, 1.2, 1.2);
	glRotated(90, 0, 1, 0);
	DrawRotor();
	glPopMatrix();

	glPushMatrix();
	glTranslated(-3.94, 4.4, 0); //координаты реального размещения
	glRotated(RotorRotation, 1, 0, 0);
	glScaled(1.2, 1.2, 1.2);
	DrawRotor();
	glPopMatrix();

	glPushMatrix();
	glTranslated(-3.94, 0, 4.4); //координаты реального размещения
	glRotated(RotorRotation, 1, 0, 0);
	glScaled(1.2, 1.2, 1.2);
	glRotated(90, 1, 0, 0);
	DrawRotor();
	glPopMatrix();

	glPushMatrix();
	glTranslated(-3.94, -4.4, 0); //координаты реального размещения
	glRotated(RotorRotation, 1, 0, 0);
	glScaled(1.2, 1.2, 1.2);
	glRotated(180, 0, 1, 0);
	glRotated(180, 0, 0, 1);
	DrawRotor();
	glPopMatrix();

	if (textureMode)
	{
		//настройка материала
		GLfloat amb[] = { 0.24, 0.8, 0, 1. };
		GLfloat dif[] = { 0.24, 0.8, 0, 1. };
		GLfloat spec[] = { 0.24, 0.8, 0, 1. };
		GLfloat sh = 0.1f * 256;
		glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
		glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
		glMaterialf(GL_FRONT, GL_SHININESS, sh);
	}

	glPushMatrix();
	glBegin(GL_QUADS);
	glVertex3d(-50, 50, -56);
	glVertex3d(50, 50, -56);
	glVertex3d(50, -50, -56);
	glVertex3d(-50, -50, -56);
	glEnd();
	glPopMatrix();

	glClearColor(0.4, 0.61, 0.68, 1.0);

	if (RotorRotation >= (360 - 1.5*rotationMultiplier)|| RotorRotation <= (-360 - 1.5*rotationMultiplier))
		RotorRotation = 0;
	else
		RotorRotation += 1.5*rotationMultiplier;

	if (RotorRotationV >= (360 - 1.5*rotationMultiplierV) || RotorRotationV <= (-360 - 1.5*rotationMultiplierV))
		RotorRotationV = 0;
	else
		RotorRotationV += 1.5*rotationMultiplierV;
}   //конец тела функции

