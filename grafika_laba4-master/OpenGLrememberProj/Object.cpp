#include "Object.h"

void Object::movie(GLdouble x, GLdouble y, GLdouble z)
{
	Vec3 v(x, y, z);
	movie(v);
}

void Object::movie(Vec3 &v)
{
	pos = pos + v;
}