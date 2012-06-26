#include "sskeleton_dll.h"

SFace *returnFaces = NULL;

using namespace std;
/*******************************************************************************
 
 Purpose - DLL function to do the skeleton.

 Call ss_cleanup to dispose of alloc'd memory.

 *******************************************************************************/
extern "C" SFace * ss_skeleton(SEdge *outline, int numEdges, int *numFaces)
{
    LoopL<Edge> out;
    Loop<Edge> loop;
    out.v.push_back(&loop);

    vector<Corner> corners;
    vector<Edge> edges;
    vector<Machine> machines;

    /* Setup input. */
    SEdge *inEdge = outline;
    for (int i = 0; i < numEdges; i++, inEdge++)
    {
        corners.push_back(Corner(inEdge->start.x, inEdge->start.y,
                    inEdge->start.z));
        machines.push_back(Machine(inEdge->pitch));
        cout << "Created corner: " << corners.back() << endl;
    }

    for (size_t i = 0; i < corners.size() - 1; i++)
        edges.push_back(Edge(&corners[i], &corners[i + 1]));
    edges.push_back(Edge(&corners[corners.size() - 1], &corners[0]));

    for (size_t i = 0; i < edges.size(); i++)
    {
        edges[i].machine = &machines[i];
        loop.append(&edges[i]);
    }

    /* We need at least three points, otherwise skeleton algorithm goes bang.
     * Note that if they're not counter-clockwise it will do the same.
     */
    if (loop.count() < 3)
    {
        GC.collect();
        return NULL;
    }

    Skeleton skel(&out);
    skel.skeleton();

    *numFaces = skel.output->faces.size();
    returnFaces = (SFace *) malloc(*numFaces * sizeof(SFace));
    SFace *f = returnFaces;
    for (IndirectLinkedHashMap<Corner *, Output::Face *>::key_iterator faceIt =
            skel.output->faces.begin(); faceIt != skel.output->faces.end();
            ++faceIt)
    {
        Output::Face *face = faceIt->second;

        f->numPoints = face->pointCount();
        f->points = (SCoord *) malloc(f->numPoints * sizeof(SCoord));
        SCoord *c = f->points;

        for (LoopL<Point3D>::EIterator eit = face->points->eIterator();
                eit.hasNext(); )
        {
            Point3D *pt = eit.next();
            c->x = pt->x;
            c->y = pt->y;
            c->z = pt->z;

            c++;
        }

        f++;
    }

    GC.collect();
    return returnFaces;
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
extern "C" void ss_cleanup(void)
{
	GC.collect();

	free(returnFaces);
	returnFaces = NULL;
}
