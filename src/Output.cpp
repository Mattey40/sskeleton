#include "Output.h"

using namespace boost;
using namespace std;

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Output::SharedEdge::SharedEdge(void) :
    start(NULL), end(NULL), left(NULL), right(NULL)
{
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Output::SharedEdge::SharedEdge(Point3D *start, Point3D *end) :
    start(start), end(end), left(NULL), right(NULL)
{
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Point3D * Output::SharedEdge::getStart(Face *ref) const
{
    if (ref == left)
        return end;
    else if (ref == right)
        return start;
    throw runtime_error("face ref not found!");
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Point3D * Output::SharedEdge::getEnd(Face *ref) const
{
    if (ref == left)
        return start;
    else if (ref == right)
        return end;
    throw runtime_error("face ref not found!");
}

/*******************************************************************************
 
 Purpose - Symmetric wrt start, end!

 *******************************************************************************/
bool Output::SharedEdge::operator==(const Output::SharedEdge &other) const
{
    if (*other.start == *start)
        return *other.end == *end;
    else if (*other.end == *start)
        return *other.start == *end;

    return false;
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
std::ostream & operator<<(std::ostream &strm, const Output::SharedEdge &t)
{
    return strm << "{" << *t.start << " to " << *t.end << "}";
}

/*******************************************************************************
 
 Purpose - Symmetric wrt start, end!

 *******************************************************************************/
size_t hash_value(Output::SharedEdge const& e)
{
    boost::hash<Point3D> hasher;
    size_t hash = 7;
    hash += 71 * (e.start != NULL ? hasher(*e.start) : 0);
    hash += 71 * (e.end != NULL ? hasher(*e.end) : 0);

    return hash;
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Output::Face * Output::SharedEdge::getOther(Face *ref)
{
    if (ref == left)
        return right;
    else if (ref == right)
        return left;
    throw runtime_error("face ref not found!");
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
void Output::SharedEdge::setLeft(Point3D *start, Output::Face *left)
{
    if (*this->start == *start)
        this->left = left;
    else if (*this->end == *start)
        this->right = left;
    else
        throw runtime_error("start point not found!");
}

/*******************************************************************************
 
 Purpose - DO NOT WANT

 *******************************************************************************/
Output::Face::Face(void) :
    points(NULL), parent(NULL), edge(NULL), owner(NULL)
{
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Output::Face::Face(Output *owner) :
    points(NULL), parent(NULL), edge(NULL), owner(owner)
{
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
int Output::Face::pointCount(void)
{
    if (points == NULL)
        throw runtime_error("pointCount: points are null!");

    return points->count();
}

/*******************************************************************************
 
 Purpose - Is a defining edge of the above (child) edges

 *******************************************************************************/
bool Output::Face::isTop(SharedEdge *edge)
{
    return topSE.contains(edge);
}

/*******************************************************************************
 
 Purpose - Is a defining edge

 *******************************************************************************/
bool Output::Face::isBottom(SharedEdge *edge)
{
    return definingSE.contains(edge);
}

/*******************************************************************************
 
 Purpose - Isn't a top or bottom edges

 *******************************************************************************/
bool Output::Face::isSide(SharedEdge *edge)
{
    return !(isTop(edge) || isBottom(edge));
}

/*******************************************************************************
 
 Purpose - When calculating an offset, we can assume that all edges add a face
 at every interval
 this returns the number of faces below this face.

 *******************************************************************************/
int Output::Face::getParentCount(void)
{
    int count = -1;
    Face *f = this;
    while (f != NULL)
    {
        count++;
        f = f->parent;
    }
    return count;
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
void Output::Face::findSharedEdges(void)
{
    edges.v.clear();
    if (points == NULL)
        throw runtime_error("findSharedEdges: points are null!");

    for (size_t i = 0; i < points->v.size(); i++)
    {
        Loop<SharedEdge> *loop = GCNEW(Loop<SharedEdge>());

        Loop<Point3D>::LoopableIterator lit = points->v[i]->loopableIterator();
        while (lit.hasNext())
        {
            Loopable<Point3D> *loopable = lit.next();

            SharedEdge *e = owner->createEdge(loopable->get(),
                    loopable->getNext()->get());
            e->setLeft(loopable->get(), this);

            loop->append(e);
        }

        edges.v.push_back(loop);
    }
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Output::SegmentOriginatorCache::SegmentOriginatorCache(Output &owner) :
    owner(owner)
{
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
std::vector<Corner *> Output::SegmentOriginatorCache::get(Corner *&aCorner)
{
    vector<Corner *> out;

    Face *f = owner.faces[aCorner->nextL->start];
    while (f->parent != NULL)
        f = f->parent;

    out.assign(f->definingCorners.begin(), f->definingCorners.end());

    return out;
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
std::vector<Corner *> Output::SegmentOriginatorCache::create(Corner *&i)
{
    throw runtime_error("Have overriden get(), shouldn't end up here!");
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Output::Output(Skeleton *skel) :
    skeleton(skel)
{
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Output::SharedEdge * Output::createEdge(Point3D *start, Point3D *end)
{
    SharedEdge newEdge(start, end);
    return &edges.get(newEdge);
}

/*******************************************************************************
 
 Purpose - One edge may start in two locations at the saem time. To accomodate
 this, you call newEdge once per edge, then new Defining Segment for each corner
 that references that edge.

 *******************************************************************************/
void Output::newEdge(Edge *e, Corner *aParentLeadingCorner)
{
    Face *face = GCNEW(Face(this));

    face->edge = e;

    if (aParentLeadingCorner != NULL) // an originator - an edge in the plan
    {
        if (!faces.containsKey(aParentLeadingCorner))
            throw runtime_error("parent leading corner not found!");
        Face *parentFace = faces[aParentLeadingCorner];
        face->parent = parentFace;
        //skeleton.parent(face, parentFace);
    }

    /* we assume that start locations for edges are unique! */
    if (faces.containsKey(e->start))
        throw runtime_error("start location for edge not unique!");

    e->start->nextL = e; // these are always true?! - we rely on them for indexing, below
    e->end->prevL = e;
    faces[e->start] = face;
}

/*******************************************************************************
 
 Purpose - see newEdge

 *******************************************************************************/
void Output::newDefiningSegment(Corner *leadingCorner)
{
    Face *face = faces[leadingCorner->nextL->start];

    SharedEdge *se = createEdge(leadingCorner, leadingCorner->nextC);
    face->definingSE.push_back(se);
    se->setLeft(leadingCorner, face);
    face->results.add(*se->start, *se->end);

    face->definingCorners.push_back(leadingCorner);
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
void Output::addOutputSideTo(Point3D *a, Point3D *b, int edgeCount,
        Edge *edgearr[])
{
    addOutputSideTo(false, a, b, edgeCount, edgearr);
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void Output::addOutputSideTo(bool isTop, Point3D *a, Point3D *b, int edgeCount,
        Edge *edgearr[])
{
    for (int i = 0; i < edgeCount; i++)
    {
        Edge *edge = edgearr[i];

        /* assumption: start of edge will always be a leading corner on the
         * face!
         */
        Corner *c = edge->start;

        if (!faces.containsKey(c))
            throw runtime_error("corner not found!");
        Face *f = faces[c];
        if (isTop)
            f->topSE.push_back(createEdge(a, b));

        /* just check those tuples aren't corners... */
        f->results.add(*a, *b);
    }
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void Output::setParent(Corner *neu, Corner *old)
{
    Face *nf = faces[neu];
    Face *of = faces[old];
    // skeleton.parent(nF, oF);
    nf->parent = of;
}

/*******************************************************************************

  Purpose - (calculate) Constructs the faces from using the results graph and
  the points involved with each edge.

 *******************************************************************************/
void initLinkedHashSet(LinkedHashSet<Point3D> &lhs,
        GraphMap<Point3D>::MapType &map)
{
    GraphMap<Point3D>::MapType::key_iterator it;
    for (it = map.begin(); it != map.end(); ++it)
        lhs.push_back(it->first);
}
void Output::calculate(Skeleton *skel)
{
    IndirectLinkedHashMap<Corner *, Face *>::key_iterator it;

    //edge:
    for (it = faces.begin(); it != faces.end(); ++it)
    {
        Face *face = it->second;
        cout << "Calculate face results: " << face->results << endl;

        LinkedHashSet<Point3D> notVisited;
        initLinkedHashSet(notVisited, face->results.map);

        LoopL<Point3D> *faceWithHoles = GCNEW(LoopL<Point3D>());
        face->points = faceWithHoles;

        try
        {
            Point3D edgeStart = *face->definingSE.front()->getStart(face);
            
            while (!notVisited.empty())
            {
                /* associated face input polygon */
                Loop<Point3D> *poly = GCNEW(Loop<Point3D>());

                bool isOuter = notVisited.contains(edgeStart);

                Point3D start = isOuter ? edgeStart : notVisited.front();
                Point3D pos = start;

                //cout << "Face results: " << face.results << endl;
                //cout << "Start: " << start << endl;
                //cout << "Face results for start: " << face.results.get(start) << endl;
                //cout << boolalpha << "Does map contain key? " << face.results.map.containsKey(start) << endl;
                Point3D last = face->results.get(start)->at(0);

                Point3D *first = NULL;
                Point3D *lastAdded = NULL;

                AngleAccumulator ac(isOuter, face->edge->getPlaneNormal());

                int count = 0;

                //pointsInLoop:
                do
                {
                    vector<Point3D> *choice = face->results.get(pos);
                    if (choice == NULL)
                        throw runtime_error("choice == NULL");

                    for (size_t i = 0; i < choice->size(); i++)
                    {
                        Point3D c = (*choice)[i];

                        if (count++ > 1000) // handbrake turn!
                            goto continueedge;

                        if (last != c && pos != c)
                        {
                            if (first == NULL)
                                first = GCNEW(Point3D(c));

                            notVisited.remove(c);

                            /* remove short edges between the previous corners,
                             * and between the current corner and the startstart
                             * (bad hack)
                             */
                            if ((lastAdded == NULL ||
                                    lastAdded->distance(c) > 0.01) &&
                                (*first == c || first->distance(c) > 0.01))
                            {
                                poly->append(GCNEW(Point3D(c)));
                                cout << "Adding to polygon: " << c << endl;
                                ac.add(c);

                                delete lastAdded;
                                lastAdded = new Point3D(c);
                            }

                            last = pos;
                            pos = c;
                            goto continuePointsInLoop;
                        }
                    }

                    cerr << "didn't find faces on " << face->definingSE << endl;
                    cerr << face->results << endl;

                continuePointsInLoop:
                    ;
                }
                while (pos != start);

                delete lastAdded;

                /* inner loops go counter clockwise */
                if (!ac.correctAngle())
                    poly->reverse();

                removeStraights(poly);

                /* as we remove degenerately small polygons* */
                if (poly->count() >= 3)
                {
                    faceWithHoles->v.push_back(poly);
                    cout << "***Added poly to points!***" << endl;
                }
                else
                    cout << "***Didn't add poly to points, as size is only " << poly->count() << endl;
            }
        }
        catch (std::exception &e)
        {
            cerr << "Exception in calculate: " << e.what() << endl;
        }

    continueedge:
        ;
    }

    /* *so we remove faces without polygons */
    vector<Face *> nullFaces;
    for (it = faces.begin(); it != faces.end(); ++it)
    {
        Face *f = it->second;
        if (f->points->v.size() <= 0)
            nullFaces.push_back(f);
    }

    if (nullFaces.size() != 0)
        throw runtime_error("nullfaces size != 0"); // do something sensible

    for (it = faces.begin(); it != faces.end(); ++it)
        it->second->findSharedEdges();
}

/*******************************************************************************

  Purpose - Two parallel faces have become consecutive, remove info about toGo,
  add to toKeep

 *******************************************************************************/
void Output::merge(Corner *toKeep, Corner *toGo)
{
    if (!faces.containsKey(toGo->nextL->start))
    {
        cerr << "three consecutive parallel edges in input?" << endl;
        return;
    }

    if (!faces.containsKey(toKeep->nextL->start))
        throw runtime_error("cannot find toKeep!");

    Face *toGoFace = faces[toGo->nextL->start];
    Face *toKeepFace = faces[toKeep->nextL->start];

    toKeepFace->definingSE.insert(toKeepFace->definingSE.end(),
            toGoFace->definingSE.begin(), toGoFace->definingSE.end());

    cout << "Merge (before): " << toKeepFace->results << endl;
    toKeepFace->results.addEntriesFrom(toGoFace->results);
    cout << "Merge (after): " << toKeepFace->results << endl;

    toKeepFace->definingCorners.insert(toKeepFace->definingCorners.end(),
            toGoFace->definingCorners.begin(), toGoFace->definingCorners.end());

    /* forward any further face requests to the new one */
    faces[toGo->nextL->start] = toKeepFace;
    faces[toGo] = toKeepFace;
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
IndirectCache<Corner *, std::vector<Corner *> > *
    Output::getSegmentOriginator(void)
{
    return GCNEW(SegmentOriginatorCache(*this));
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void Output::removeStraights(Loop<Point3D> *poly)
{
    /* This will be filtered out later. Some of this assumes >= 3 edges. */
    if (poly->count() < 3)
        return;

    vector<Loopable<Point3D> > togo;
    
    Loop<Point3D>::LoopableIterator litr = poly->loopableIterator();
    ConsecutiveTriples<Loopable<Point3D> > citr(&litr, true);
    while (citr.hasNext())
    {
        tuple<Loopable<Point3D>, Loopable<Point3D>, Loopable<Point3D> > *trip =
            citr.next();

        Loopable<Point3D> a = trip->get<0>(),
            b = trip->get<1>(),
            c = trip->get<2>();

        Vector3D ab(*b.get()),
                 bc(*c.get());
        ab.sub(*a.get());
        bc.sub(*b.get());

        double angle = ab.angle(bc);
        double small = 0.001;
        if (angle < small || angle > M_PI - small)
            togo.push_back(b);
    }

    vector<Loopable<Point3D> >::iterator it;
    for (it = togo.begin(); it != togo.end(); ++it)
    {
        Point3D *tmp = it->get();
        poly->remove(tmp);
    }
}
