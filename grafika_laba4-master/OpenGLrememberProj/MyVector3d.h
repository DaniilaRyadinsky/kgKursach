#ifndef  MYVECTOR3D_H
#define  MYVECTOR3D_H


#include <math.h>
#include "angle.h"

//Обычковенный 3хкомпонентный вектор
class Vec3
{
	double coords[3];

public:

	inline double x()
	{
		return coords[0];
	}
	inline double y()
	{
		return coords[1];
	}
	inline double z()
	{
		return coords[2];
	}

	inline void fromSpherical(angle &eta, angle &fi, double R)
	{
		coords[0] = R*sin(eta)*cos(fi);
		coords[1] = R*sin(eta)*sin(fi);
		coords[2] = R*cos(eta);
	}

	inline void setCoords(double x, double y, double z)
	{
		coords[0] = x;
		coords[1] = y;
		coords[2] = z;
	}

	inline Vec3(angle &eta, angle &fi, double R)
	{
		fromSpherical(eta, fi, R);
	}
	inline Vec3(double x, double y, double z)
	{
		coords[0] = x;
		coords[1] = y;
		coords[2] = z;
	}
	inline Vec3()
	{

	}

	inline Vec3 operator + (Vec3 &vec)
	{
		Vec3 newV;
		newV.coords[0] = coords[0] + vec.coords[0];
		newV.coords[1] = coords[1] + vec.coords[1];
		newV.coords[2] = coords[2] + vec.coords[2];
		return newV;
	}

	inline Vec3 operator - (Vec3 &vec)
	{
		Vec3 newV;
		newV.coords[0] = coords[0] - vec.coords[0];
		newV.coords[1] = coords[1] - vec.coords[1];
		newV.coords[2] = coords[2] - vec.coords[2];
		return newV;
	}

	inline Vec3 operator * (double k)
	{
		Vec3 newV;
		newV.coords[0] = coords[0] * k;
		newV.coords[1] = coords[1] * k;
		newV.coords[2] = coords[2] * k;
		return newV;
	}

	inline void operator = (Vec3 vec)
	{
		coords[0] = vec.coords[0];
		coords[1] = vec.coords[1];
		coords[2] = vec.coords[2];
	}
	
	inline double length()
	{
		return (sqrt(coords[0] * coords[0] + coords[1] * coords[1] + coords[2] * coords[2]));
	}



	inline Vec3 normolize()
	{
		Vec3 newV;
		double l = length();
		newV.setCoords(coords[0] / l, coords[1] / l, coords[2] / l);
		return newV;
	}
	inline Vec3 vectorMultiply(Vec3 &v)
	{
		Vec3 V;
		V.setCoords(coords[1] * v.coords[2] - coords[2] * v.coords[1],
			coords[2] * v.coords[0] - coords[0] * v.coords[2],
			coords[0] * v.coords[1] - coords[1] * v.coords[0]);
		return V;
	}

	inline const double *toArray(void)
	{
		return (coords);
	}
};


class mathVec {
	Vec3 start;
	Vec3 end;
public:

	mathVec(Vec3 start, Vec3 end) : start(start), end(end) {}

	inline Vec3 Start() {
		return start;
	}

	inline Vec3 End() {
		return end;
	}

	inline double modX() {
		return end.x() - start.x();
	}

	inline double modY() {
		return end.y() - start.y();
	}

	inline double modZ() {
		return end.z() - start.z();
	}

	inline double modMathVector() {
		return sqrt(pow(modX(), 2) + pow(modY(), 2) + pow(modZ(), 2));
	}

	inline double scalarMultiplyVector(mathVec& r) {
		return modX() * r.modX() + modY() * r.modY() + modZ() * r.modZ();
	}

	inline double angleBetwVectors(mathVec& r) {
		double modP = modMathVector();
		double modR = r.modMathVector();

		double multiply = scalarMultiplyVector(r);
		return acos(multiply / (modP * modR)) * 180 / PI;
	}


	inline void operator = (mathVec vec)
	{
		start = vec.start;
		end = vec.end;
	}
};

#endif