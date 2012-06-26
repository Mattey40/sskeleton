#include "Machine.h"

using namespace std;

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Machine::Machine(void) :
    description("unnamed machine"), currentAngle(M_PI / 4), heightEvent(NULL),
    currentDirection(-1)
{
    init(M_PI / 4);
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Machine::Machine(double initial) :
    description("unnamed machine"), currentAngle(M_PI / 4), heightEvent(NULL),
    currentDirection(-1)
{
    init(initial);
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
void Machine::init(double initial)
{
    addHeightEvent(GCNEW(DirectionHeightEvent(this, initial)));
    currentAngle = initial;
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
void Machine::addEdge(Edge *e, Skeleton *skel)
{
    if (heightEvent == NULL)
        findNextHeight(skel);

    /* when we're new, or sometimes when we swap a machine out and back in again
     * things get confused
     */
    e->setAngle(currentAngle);
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
void Machine::findNextHeight(Skeleton *skel)
{
    if (events.empty())
        throw std::runtime_error("I need height events!");

    currentDirection++;

    if (currentDirection == 0)
    {
        DirectionHeightEvent *dhe =
            dynamic_cast<DirectionHeightEvent *>(events.at(0));
        if (dhe != NULL)
        {
            /* first direction added to a new edge is taken to be the starting
             * angle
             */
            currentAngle = dhe->newAngle;
            heightEvent = dhe;
            currentDirection++;
            /* proceed to add the following direction... */
        }
        else
        {
            /* I ran into trouble---as we add edges, we want to be able to set
             * the initial angle and ignore the first height event. Otherwise
             * we immediately call replace edges in skeleton and introduce
             * additional edges.
             */
            throw std::runtime_error("You have to think really hard about how "
                    "the first event sets it's angle before you do this");
        }
    }

    if (currentDirection >= (int) getDirections().size())
        return;

    heightEvent = getDirections().at(currentDirection);

    skel->qu->add(heightEvent);
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
std::vector<HeightEvent *> Machine::getDirections(void)
{
    return events;
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
void Machine::sortHeightEvents(void)
{
    sort(events.begin(), events.end(), HeightEvent::PtrComparator());
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
void Machine::addHeightEvent(HeightEvent *dir)
{
    events.push_back(dir);
    sortHeightEvents();
}
