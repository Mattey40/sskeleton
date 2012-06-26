#ifndef SKELETON_DLL_H
#define SKELETON_DLL_H

#include "Corner.h"
#include "Loop.h"
#include "GC.h"
#include "Machine.h"

extern "C"
{
    struct SCoord
    {
        double x;
        double y;
        double z;
    };

    struct SFace
    {
        int numPoints;
        SCoord *points;
    };

    struct SEdge
    {
        SCoord start;
        double pitch;
    };

#ifdef _MSC_VER
    __declspec(dllexport)
#endif
    SFace * ss_skeleton(SEdge *outline, int numEdges, int *numFaces);

#ifdef _MSC_VER
	__declspec(dllexport)
#endif
	void ss_cleanup(void);
}

#endif
