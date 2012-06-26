#include <fstream>
#include <iostream>
#include <vector>

#include "Corner.h"
#include "Loop.h"
#include "GC.h"
#include "Machine.h"
#include "sskeleton_dll.h"

using namespace std;

int main(int argc, char **argv)
{
	SEdge edges[4];
	edges[0].start.x = 0; edges[0].start.y = 0; edges[0].start.z = 0;
	edges[1].start.x = 500; edges[1].start.y = 0; edges[1].start.z = 0;
	edges[2].start.x = 500; edges[2].start.y = 300; edges[2].start.z = 0;
	edges[3].start.x = 0; edges[3].start.y = 300; edges[3].start.z = 0;

    edges[0].pitch = M_PI / 4;
    edges[1].pitch = M_PI / 4;
    edges[2].pitch = M_PI / 4;
    edges[3].pitch = M_PI / 4;

	int numFaces;
	SFace * faces = ss_skeleton(edges, 4, &numFaces);

    ofstream fs;
    fs.open("/tmp/faces.dat");

    SFace *face = faces;
    for (int i = 0; i < numFaces; i++)
    {
        cout << endl;

        SCoord *pt = face->points;
        for (int i = 0; i < face->numPoints; i++)
        {
            cout << "RESULT: (" << pt->x << ", " << pt->y << ", " <<
                pt->z << ")" << endl;
            fs << pt->x << "," << pt->y << endl;

            pt++;
        }
        fs << endl;

        face++;
    }
    
    fs.close();

	ss_cleanup();

    //vector<Corner> corners;
    //vector<Edge> edges;

    //corners.push_back(Corner(0, 0));
    //corners.push_back(Corner(500, 0));
    //corners.push_back(Corner(500, 300));
    //corners.push_back(Corner(0, 300));


    //corners.push_back(Corner(0,0));
    //corners.push_back(Corner(1000,0));
    //corners.push_back(Corner(1000,500));
    //corners.push_back(Corner(700,500));
    //corners.push_back(Corner(700,700));
    //corners.push_back(Corner(300,700));
    //corners.push_back(Corner(300,500));
    //corners.push_back(Corner(0,500));

    //while (true)
    //{
    //    cout << "Enter a point (press Return when done): " << endl;

    //    string input;

    //    getline(cin, input);

    //    if (input.length() == 0)
    //    {
    //        cout << "...done" << endl;
    //        break;
    //    }

    //    string xS, yS;

    //    size_t commaIdx = input.find(',');
    //    xS = input.substr(0, commaIdx);
    //    yS = input.substr(commaIdx + 1);

    //    corners.push_back(Corner(atof(xS.c_str()), atof(yS.c_str())));
    //    cout << "    (stored corner: " << corners.back() << ")" << endl;
    //}

    //Machine directionMachine;

    //LoopL<Edge> out;
    //Loop<Edge> loop1;
    //out.v.push_back(&loop1);

    //for (size_t i = 0; i < corners.size() - 1; i++)
    //    edges.push_back(Edge(&corners[i], &corners[i + 1]));
    //edges.push_back(Edge(&corners[corners.size() - 1], &corners[0]));

    //for (size_t i = 0; i < edges.size(); i++)
    //    loop1.append(&edges[i]);

    //for (Loop<Edge>::LoopIterator eit = loop1.iterator(); eit.hasNext(); )
    //    eit.next()->machine = &directionMachine;

    //Skeleton skel(&out);
    //skel.skeleton();

    //ofstream fs;
    //fs.open("/tmp/faces.dat");

    //for (IndirectLinkedHashMap<Corner *, Output::Face *>::key_iterator faceIt =
    //        skel.output->faces.begin(); faceIt != skel.output->faces.end();
    //        ++faceIt)
    //{
    //    Output::Face *face = faceIt->second;

    //    cout << "Num points: " << face->pointCount() << endl;
    //    cout << "face:" << endl;
    //    for (vector<Loop<Point3D> *>::iterator lp3it = face->points->v.begin();
    //            lp3it != face->points->v.end(); ++lp3it)
    //    {
    //        Loop<Point3D> *lp3 = *lp3it;
    //        for (Loop<Point3D>::LoopIterator ptIt = lp3->iterator();
    //                ptIt.hasNext(); )
    //        {
    //            Point3D *pt = ptIt.next();
    //            cout << *pt << endl;
    //            fs << pt->x << "," << pt->y << endl;
    //        }
    //        fs << endl;
    //    }
    //}

    //fs.close();

    //GC.collect();

    return 0;
}
