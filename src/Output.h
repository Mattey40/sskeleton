#ifndef OUTPUT_H
#define OUTPUT_H

#include <boost/functional.hpp>
#include <boost/tuple/tuple.hpp>
#include <cmath>
#include <set>
#include <stdexcept>
#include "AngleAccumulator.h"
#include "Cache.h"
#include "ConsecutiveTriples.h"
#include "Edge.h"
#include "GraphMap.h"
#include "IdentityLookup.h"
#include "LinkedHashContainer.h"
#include "Loop.h"
#include "Point3D.h"
#include "Skeleton.h"
#include "Tuple3D.h"
#include "Vector3D.h"

class Skeleton;
class Output
{
    public:
        class LoopNormal
        {
            public:
                LoopL<Point3D> *loopl;
                Vector3D norm;

                LoopNormal(LoopL<Point3D> *loopl, Vector3D norm) :
                    loopl(loopl), norm(norm) { }
        };

        class Face;
        class SharedEdge
        {
            public:
                Point3D *start, *end;
                Face *left, *right;

                SharedEdge(void);
                SharedEdge(Point3D *start, Point3D *end);
                Point3D * getStart(Face *ref) const;
                Point3D * getEnd(Face *ref) const;
                bool operator==(const SharedEdge &other) const;
                Face * getOther(Face *ref);
                void setLeft(Point3D *start, Face *left);
                friend std::ostream & operator<<(std::ostream &strm,
                        const SharedEdge &t);
        };

        class Face
        {
            public:
                /* first is outside, others are holes */
                LoopL<Point3D> *points;

                /* bottom edges */
                IndirectLinkedHashSet<SharedEdge *> definingSE;

                /* defining edges of child (top) edges */
                IndirectLinkedHashSet<SharedEdge *> topSE;

                /* Face below us in the skeleton - can be traced back to an
                 * originator.
                 */
                Face *parent;

                GraphMap<Point3D> results;

                /* a typical edge that defines the plane normal */
                Edge *edge;

                /* subset of results who are horizontal edges and whose nextL
                 * are edge, or similar
                 */
                IndirectLinkedHashSet<Corner *> definingCorners;

                LoopL<SharedEdge> edges;

                Face(void);
                Face(Output *owner);
                LoopL<Point3D> * getLoopL(void) { return points; }
                int pointCount(void);
                bool isTop(SharedEdge *edge);
                bool isBottom(SharedEdge *edge);
                bool isSide(SharedEdge *edge);
                int getParentCount(void);
                void findSharedEdges(void);

            private:
                Output *owner;
        };

        class SegmentOriginatorCache:
            public IndirectCache<Corner *, std::vector<Corner *> >
        {
            public:
                SegmentOriginatorCache(Output &owner);
                virtual std::vector<Corner *> get(Corner *&aCorner);
                virtual std::vector<Corner *> create(Corner *&i);

            private:
                Output &owner;
        };

        IndirectLinkedHashMap<Corner *, Face *> faces;
        std::vector<LoopNormal> nonSkelFaces;

        IdentityLookup<SharedEdge> edges;

        Skeleton *skeleton;

        Output(Skeleton *skel);
        void newEdge(Edge *e, Corner *aParentLeadingCorner);
        void newDefiningSegment(Corner *leadingCorner);
        void addOutputSideTo(Point3D *a, Point3D *b, int edgeCount,
                Edge *edgearr[]);
        void addOutputSideTo(bool isTop, Point3D *a, Point3D *b, int edgeCount,
                Edge *edgearr[]);
        void setParent(Corner *neu, Corner *old);
        void calculate(Skeleton *skel);
        void merge(Corner *toKeep, Corner *toGo);
        SharedEdge * createEdge(Point3D *start, Point3D *end);
        IndirectCache<Corner *, std::vector<Corner *> > *
            getSegmentOriginator(void);
        void removeStraights(Loop<Point3D> *poly);
};

size_t hash_value(Output::SharedEdge const& e);

#endif
