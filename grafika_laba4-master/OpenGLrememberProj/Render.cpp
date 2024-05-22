#include "Render.h"

#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "MyShaders.h"

#include "ObjLoader.h"
#include "GUItextRectangle.h"

#include "Texture.h"

GuiTextRectangle rec;

bool textureMode = true;
bool lightMode = true;
bool movement = true;


//небольшой дефайн для упрощения кода
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile* model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;

Shader s[10];  //массивчик для десяти шейдеров
Shader frac;
Shader cassini;




//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double yaw, pitch;


	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		yaw = 1;
		pitch = 1;
	}


	//считает позицию камеры, исходя из углов поворота, вызывается движком
	virtual void SetUpCamera()
	{

		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist * cos(pitch) * cos(yaw),
			camDist * cos(pitch) * sin(yaw),
			camDist * sin(pitch));

		if (cos(pitch) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		gluLookAt(pos.x(), pos.y(), pos.z(), lookPoint.x(), lookPoint.y(), lookPoint.z(), normal.x(), normal.y(), normal.z());
	}



}  camera;   //создаем объект камеры


//класс недоделан!
class WASDcamera :public CustomCamera
{
public:

	float camSpeed;

	WASDcamera()
	{
		camSpeed = 0.4;
		pos.setCoords(5, 5, 5);
		lookPoint.setCoords(0, 0, 0);
		normal.setCoords(0, 0, 1);
	}

	virtual void SetUpCamera()
	{

		if (OpenGL::isKeyPressed('W'))
		{
			Vec3 forward = (lookPoint - pos).normolize() * camSpeed;
			pos = pos + forward;
			lookPoint = lookPoint + forward;

		}
		if (OpenGL::isKeyPressed('S'))
		{
			Vec3 forward = (lookPoint - pos).normolize() * (-camSpeed);
			pos = pos + forward;
			lookPoint = lookPoint + forward;

		}

		LookAt();
	}



} WASDcam;


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vec3(1, 1, 3);
	}


	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		Shader::DontUseShaders();
		bool f1 = glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		bool f2 = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		bool f3 = glIsEnabled(GL_DEPTH_TEST);

		glDisable(GL_DEPTH_TEST);
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale * 0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.x(), pos.y(), pos.z());
			glVertex3d(pos.x(), pos.y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.x(), pos.y(), 0);
			c.scale = c.scale * 1.5;
			c.Show();
		}
		/*
		if (f1)
			glEnable(GL_LIGHTING);
		if (f2)
			glEnable(GL_TEXTURE_2D);
		if (f3)
			glEnable(GL_DEPTH_TEST);
			*/
	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.x(), pos.y(), pos.z(), 1. };

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

float offsetX = 0, offsetY = 0;
float zoom = 1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;

//обработчик движения мыши
void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.yaw += 0.01 * dx;
		camera.pitch += -0.01 * dy;
	}


	if (OpenGL::isKeyPressed(VK_LBUTTON))
	{
		offsetX -= 1.0 * dx / ogl->getWidth() / zoom;
		offsetY += 1.0 * dy / ogl->getHeight() / zoom;
	}



	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y, 60, ogl->aspect);

		double z = light.pos.z();

		double k = 0, x = 0, y = 0;
		if (r.direction.z() == 0)
			k = 0;
		else
			k = (z - r.origin.z()) / r.direction.z();

		x = k * r.direction.x() + r.origin.x();
		y = k * r.direction.y() + r.origin.y();

		light.pos = Vec3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vec3(0, 0, 0.02 * dy);
	}


}

//обработчик вращения колеса  мыши
void mouseWheelEvent(OpenGL* ogl, int delta)
{


	float _tmpZ = delta * 0.003;
	if (ogl->isKeyPressed('Z'))
		_tmpZ *= 10;
	zoom += 0.2 * zoom * _tmpZ;


	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01 * delta;
}

//обработчик нажатия кнопок клавиатуры
void keyDownEvent(OpenGL* ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.yaw = 1;
		camera.pitch = 1;
		camera.camDist = 15;

		light.pos = Vec3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}

	if (key == 'S')
	{
		frac.LoadShaderFromFile();
		frac.Compile();

		s[0].LoadShaderFromFile();
		s[0].Compile();

		cassini.LoadShaderFromFile();
		cassini.Compile();
	}

	if (key == 'Q')
		Time = 0;
	OpenGL open;
	if (open.isKeyPressed('M'))
	{
		movement = !movement;
	}
}

void keyUpEvent(OpenGL* ogl, int key)
{

}


void DrawQuad()
{
	double A[] = { 20,20 };
	double B[] = { 20,-20 };
	double C[] = { -20,-20 };
	double D[] = { -20,20 };
	glBegin(GL_QUADS);
	glColor3d(.5, 0, 0);
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);
	glEnd();
}

ObjFile vaz2106, panel, road;

Texture panelTex, tex2106, roadTex;

void DrawPanel() {
	PUSH;
	s[1].UseShader();
	int l = glGetUniformLocationARB(s[1].program, "tex");
	glUniform1iARB(l, 2);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	panelTex.bindTexture();
	panel.DrawObj();
	glPopMatrix();
	POP;
}

void draw2106() {
	PUSH;
	s[1].UseShader();
	int l = glGetUniformLocationARB(s[1].program, "tex");
	glUniform1iARB(l, 1); //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	PUSH;
	glRotated(180, 0, 0, 1);
	glRotated(90, 0, 0, 1);
	glRotated(90, 1, 0, 0);
	tex2106.bindTexture();
	vaz2106.DrawObj();
	POP;

	POP;
}

void DrawRoad() {
	PUSH;
	s[1].UseShader();
	int l = glGetUniformLocationARB(s[1].program, "tex");
	glUniform1iARB(l, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	roadTex.bindTexture();
	road.DrawObj();
	glPopMatrix();
	POP;
}




//выполняется перед первым рендером
void initRender(OpenGL* ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
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

	/*
	//texture1.loadTextureFromFile("textures\\texture.bmp");   загрузка текстуры из файла
	*/


	frac.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	frac.FshaderFileName = "shaders\\frac.frag"; //имя файла фрагментного шейдера
	frac.LoadShaderFromFile(); //загружаем шейдеры из файла
	frac.Compile(); //компилируем

	cassini.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	cassini.FshaderFileName = "shaders\\cassini.frag"; //имя файла фрагментного шейдера
	cassini.LoadShaderFromFile(); //загружаем шейдеры из файла
	cassini.Compile(); //компилируем


	s[0].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[0].FshaderFileName = "shaders\\light.frag"; //имя файла фрагментного шейдера
	s[0].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[0].Compile(); //компилируем

	s[1].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //имя файла фрагментного шейдера
	s[1].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[1].Compile(); //компилируем



	//так как гит игнорит модели *.obj файлы, так как они совпадают по расширению с объектными файлами, 
	// создающимися во время компиляции, я переименовал модели в *.obj_m

	

	glActiveTexture(GL_TEXTURE2);
	loadModel("models\\untitled.obj_m", &panel);
	panelTex.loadTextureFromFile("textures//panel.bmp");
	panelTex.bindTexture();

	glActiveTexture(GL_TEXTURE1);
	loadModel("models\\2106.obj_m", &vaz2106);
	tex2106.loadTextureFromFile("textures//2106tex.bmp");
	tex2106.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("models\\road.obj_m", &road);
	roadTex.loadTextureFromFile("textures//road.bmp");
	roadTex.bindTexture();

	tick_n = GetTickCount();
	tick_o = tick_n;

	rec.setSize(300, 100);
	rec.setPosition(10, ogl->getHeight() - 100 - 10);
	rec.setText("T - вкл/выкл текстур\nL - вкл/выкл освещение\nF - Свет из камеры\nG - двигать свет по горизонтали\nG+ЛКМ двигать свет по вертекали", 0, 0, 0);


}


inline double f_bezz(double p1, double p2, double p3, double t)
{
	return p1 * (1 - t) * (1 - t) + 2 * p2 * t * (1 - t) + p3 * t * t; //посчитаная формула
}


void Bezz1(Vec3 P1, Vec3 P2, Vec3 P3) {

	glLineWidth(3); //ширина линии
	glBegin(GL_LINE_STRIP);
	for (double t = 0; t <= 1.0001; t += 0.01)
	{
		double P[3];
		P[0] = f_bezz(P1.x(), P2.x(), P3.x(), t);
		P[1] = f_bezz(P1.y(), P2.y(), P3.y(), t);
		P[2] = f_bezz(P1.z(), P2.z(), P3.z(), t);
		glVertex3dv(P); //Рисуем точку P
	}
	glEnd();
	glLineWidth(1); //возвращаем ширину линии = 1
}


double t_max = 0;
bool isTuda = true;


mathVec last = { Vec3(0,0,0), Vec3(0, 1, 0) };
double yaw = 0;

void vjjjjjj(Vec3 P1, Vec3 P2, Vec3 P3) {

	Bezz1(P1, P2, P3);


	Vec3 start = {
		f_bezz(P1.x(), P2.x(), P3.x(), t_max),
		f_bezz(P1.y(), P2.y(), P3.y(), t_max),
		f_bezz(P1.z(), P2.z(), P3.z(), t_max)
	};

	mathVec p(last.End(), start);

	glPushMatrix();
	yaw += p.angleBetwVectors(last);

	glTranslated(p.End().x(), p.End().y(), p.End().z());
	glRotated(-yaw + 12, 0, 0, 1);

	if (!isTuda) yaw = 0;
	draw2106();
	glPopMatrix();

	glLineWidth(1); //возвращаем ширину линии = 1
	last = p;
	t_max += 0.01;
}


int stage = 1;
void race(Vec3 P1, Vec3 P2, Vec3 P3, Vec3 P4, Vec3 P5, Vec3 P6, Vec3 P7, Vec3 P8, 
	Vec3 P9, Vec3 P10, Vec3 P11, Vec3 P12, Vec3 P13, Vec3 P14, Vec3 P15, Vec3 P16) {
	if (t_max > 1) {
		t_max = 0;
		stage++;
	}
	switch (stage) {
	case 1:
		vjjjjjj(P1, P2, P3);
		break;
	case 2:
		vjjjjjj(P3, P4, P5);
		break;
	case 3:
		vjjjjjj(P5, P6, P7);
		break;
	case 4:
		vjjjjjj(P7, P8, P9);
		break;
	case 5:
		vjjjjjj(P9, P10, P11);
		break;
	case 6:
		vjjjjjj(P11, P12, P13);
		break;
	case 7:
		vjjjjjj(P13, P14, P15);
		break;
	case 8:
		vjjjjjj(P15, P16, P1);
		break;
	case 9:
		stage = 1;
		break;
	}
}


void Render(OpenGL* ogl)
{

	tick_o = tick_n;
	tick_n = GetTickCount();
	Time += (tick_n - tick_o) / 1000.0;

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	*/

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//===================================


	s[0].UseShader();

	//передача параметров в шейдер.  Шаг один - ищем адрес uniform переменной по ее имени. 
	int location = glGetUniformLocationARB(s[0].program, "light_pos");
	//Шаг 2 - передаем ей значение
	glUniform3fARB(location, light.pos.x(), light.pos.y(), light.pos.z());

	location = glGetUniformLocationARB(s[0].program, "Ia");
	glUniform3fARB(location, 0.2, 0.2, 0.2);

	location = glGetUniformLocationARB(s[0].program, "Id");
	glUniform3fARB(location, 1.0, 1.0, 1.0);

	location = glGetUniformLocationARB(s[0].program, "Is");
	glUniform3fARB(location, .7, .7, .7);


	location = glGetUniformLocationARB(s[0].program, "ma");
	glUniform3fARB(location, 0.2, 0.2, 0.1);

	location = glGetUniformLocationARB(s[0].program, "md");
	glUniform3fARB(location, 0.4, 0.65, 0.5);

	location = glGetUniformLocationARB(s[0].program, "ms");
	glUniform4fARB(location, 0.9, 0.8, 0.3, 25.6);

	location = glGetUniformLocationARB(s[0].program, "camera");
	glUniform3fARB(location, camera.pos.x(), camera.pos.y(), camera.pos.z());

	
	Vec3 P1 = { -28,20,0 };
	Vec3 P2 = { 0,20,0 };
	Vec3 P3 = { 28,20,0 };
	Vec3 P4 = { 38,20,0 };
	Vec3 P5 = { 38,10,0 };
	Vec3 P6 = { 38,0,0 };
	Vec3 P7 = { 38,-10,0 };
	Vec3 P8 = { 38,-20,0 };
	Vec3 P9 = { 28,-20,0 };
	Vec3 P10 = { 0,-20,0 };
	Vec3 P11 = { -28,-20,0 };
	Vec3 P12 = { -38,-20,0 };
	Vec3 P13 = { -38,-10,0 };
	Vec3 P14 = { -38,0,0 };
	Vec3 P15 = { -38,10,0 };
	Vec3 P16 = { -38,20,0 };

	if (movement) {
		PUSH;
		race(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16);
		//vjjjjjj(P1, P2, P3);
		POP;
	}
	else {
		PUSH;
		glTranslated(last.End().x(), last.End().y(), last.End().z());
		glRotated(yaw - 12, 0, 0, 1);
		draw2106();
		POP;
	}
	

	//Shader::DontUseShaders();
	//
	////второй, без шейдеров
	//glPushMatrix();
	//	glTranslated(-5,15,0);
	//	//glScaled(-1.0,1.0,1.0);
	//	objModel.DrawObj();
	//glPopMatrix();

	//
	PUSH;
	DrawPanel();
	glTranslated(10, 0, 0);
	DrawPanel();
	glTranslated(0, 0, 10);
	DrawPanel();
	glTranslated(-10, 0, 0);
	DrawPanel();
	glTranslated(-20, 5, -10);
	DrawPanel();
	POP;
	PUSH;
	DrawRoad();
	POP;
	//////Рисование фрактала


	/*
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,1,0,1,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		frac.UseShader();

		int location = glGetUniformLocationARB(frac.program, "size");
		glUniform2fARB(location, (GLfloat)ogl->getWidth(), (GLfloat)ogl->getHeight());

		location = glGetUniformLocationARB(frac.program, "uOffset");
		glUniform2fARB(location, offsetX, offsetY);

		location = glGetUniformLocationARB(frac.program, "uZoom");
		glUniform1fARB(location, zoom);

		location = glGetUniformLocationARB(frac.program, "Time");
		glUniform1fARB(location, Time);

		DrawQuad();

	}
	*/


	//////Овал Кассини

	/*
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,1,0,1,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		cassini.UseShader();

		int location = glGetUniformLocationARB(cassini.program, "size");
		glUniform2fARB(location, (GLfloat)ogl->getWidth(), (GLfloat)ogl->getHeight());


		location = glGetUniformLocationARB(cassini.program, "Time");
		glUniform1fARB(location, Time);

		DrawQuad();
	}

	*/

	Shader::DontUseShaders();
}   //конец тела функции


bool gui_init = false;

//рисует интерфейс, вызывется после обычного рендера
void RenderGUI(OpenGL* ogl)
{

	Shader::DontUseShaders();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);


	glActiveTexture(GL_TEXTURE0);
	rec.Draw();


	Shader::DontUseShaders();
}

void resizeEvent(OpenGL* ogl, int newW, int newH)
{
	rec.setPosition(10, newH - 100 - 10);
}

