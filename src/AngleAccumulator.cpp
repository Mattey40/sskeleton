#include "AngleAccumulator.h"

using namespace std;

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
AngleAccumulator::AngleAccumulator(bool positive, Vector3D normal) :
    targetPositive(positive), angle(0), firstPoint(NULL), lastPoint(NULL),
    firstVector(NULL), lastVector(NULL), normal(new Vector3D(normal))
{
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
AngleAccumulator::~AngleAccumulator(void)
{
    delete firstPoint;
    delete lastPoint;
    delete firstVector;
    delete lastVector;
    delete normal;
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void AngleAccumulator::add(Point3D pt)
{
    if (lastPoint == NULL)
    {
        delete firstPoint;
        firstPoint = new Point3D(pt);
        delete lastPoint;
        lastPoint = new Point3D(pt);
        return;
    }

    Vector3D v(pt);
    v.sub(*lastPoint);
    add(v);

    delete lastPoint;
    lastPoint = new Point3D(pt);
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void AngleAccumulator::add(Vector3D v)
{
    if (lastVector != NULL)
    {
        double dA = v.angle(*lastVector);

        Vector3D cross;
        cross.cross(*lastVector, v);

        if (abs(cross.length()) < 0.01) // parallel
        {
            if (dA < 0.001)
                cerr << "straight lines ok, but this is very spikey" << endl;
        }
        else
            if (cross.angle(*normal) > (M_PI * 2))
                dA = -dA;

        angle += dA;
    }
    else
    {
        delete firstVector;
        firstVector = new Vector3D(v);
    }

    /* rotate angle around origin so normal is up */
    delete lastVector;
    lastVector = new Vector3D(v);
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
bool AngleAccumulator::correctAngle(void)
{
    if (firstVector != NULL)
    {
        add(*firstPoint);
        add(*firstVector);
        delete firstVector;
        firstVector = NULL; // allow method to be called 1+ time (can't add more points though ;))
    }

    if (targetPositive)
        return abs(angle - M_PI * 2) < 0.1;
    else
        return abs(angle + M_PI * 2) < 0.1;
}
