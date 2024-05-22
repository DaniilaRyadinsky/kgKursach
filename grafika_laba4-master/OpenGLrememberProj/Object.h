#ifndef OBJECT_H
#define OBJECT_H

#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#include "MyVector3d.h"


//абстрактный объект
class Object
{
public:

	Vec3 pos;


	virtual void movie(GLdouble x, GLdouble y, GLdouble z);

	virtual void movie(Vec3 &v);
};


class RenderadbleObject: public Object
{
public:
	//Пока не работает
	Vec3 scale;
	Vec3 rotateAxe;
	double angle;

	

	RenderadbleObject()
	{
		scale = Vec3(1, 1, 1);
		rotateAxe = Vec3(0, 0, 1);
		angle = 0;
	}

	void Show()
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glRotated(angle, rotateAxe.x(), rotateAxe.y(), rotateAxe.z());
		glTranslated(pos.x(), pos.y(), pos.z());	
		glScaled(scale.x(), scale.y(), scale.z());
		RenderObject();
		glPopMatrix();
	}

	virtual void RenderObject() = 0;
};

#endif