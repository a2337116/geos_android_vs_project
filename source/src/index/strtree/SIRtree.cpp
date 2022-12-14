/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/index/strtree/SIRtree.h>
#include <geos/index/strtree/AbstractNode.h>
#include <geos/util.h>

#include <memory>
#include <vector>
#include <cassert>
#include <algorithm>



namespace geos {
namespace index { // geos.index
namespace strtree { // geos.index.strtree

static bool
compareSIRBoundables(const Boundable* a, const Boundable* b)
{
    return static_cast<const Interval*>(a->getBounds())->getCentre() <
           static_cast<const Interval*>(b->getBounds())->getCentre();
}

/*protected*/
std::unique_ptr<BoundableList>
SIRtree::createParentBoundables(BoundableList* childBoundables, int newLevel)
{
    assert(!childBoundables->empty());
    std::unique_ptr<BoundableList> parentBoundables(new BoundableList());
    parentBoundables->push_back(createNode(newLevel));

    std::unique_ptr<BoundableList> sortedChildBoundables(sortBoundables(childBoundables));

    //for(unsigned int i=0;i<sortedChildBoundables->size();i++)
    for(BoundableList::iterator i = sortedChildBoundables->begin(),
            e = sortedChildBoundables->end();
            i != e; ++i) {
        //Boundable *childBoundable=(AbstractNode*)(*sortedChildBoundables)[i];
        Boundable* childBoundable = *i;
        AbstractNode* lNode = lastNode(parentBoundables.get());
        if(lNode->getChildBoundables()->size() == nodeCapacity) {
            parentBoundables->push_back(createNode(newLevel));
        }
        lNode->addChildBoundable(childBoundable);
    }
    return parentBoundables;
}

bool
SIRtree::SIRIntersectsOp::intersects(const void* aBounds, const void* bBounds)
{
    return static_cast<const Interval*>(aBounds)->intersects(static_cast<const Interval*>(bBounds));
}

/*public*/
SIRtree::SIRtree():
    AbstractSTRtree(10),
    intersectsOp(new SIRIntersectsOp())
{
}

/*public*/
SIRtree::SIRtree(std::size_t p_nodeCapacity):
    AbstractSTRtree(p_nodeCapacity),
    intersectsOp(new SIRIntersectsOp())
{
}

SIRtree::~SIRtree()
{
    delete intersectsOp;
}


class SIRAbstractNode: public AbstractNode {
public:
    SIRAbstractNode(int p_level, std::size_t capacity)
        :
        AbstractNode(p_level, capacity)
    {}

    ~SIRAbstractNode() override
    {
        delete static_cast<Interval*>(bounds);
    }

protected:

    void*
    computeBounds() const override
    {
        Interval* p_bounds = nullptr;
        const BoundableList& b = *getChildBoundables();
        for(unsigned int i = 0; i < b.size(); ++i) {
            const Boundable* childBoundable = b[i];
            if(p_bounds == nullptr) {
                p_bounds = new Interval(*static_cast<const Interval*>(childBoundable->getBounds()));
            }
            else {
                p_bounds->expandToInclude(static_cast<const Interval*>(childBoundable->getBounds()));
            }
        }
        return p_bounds;
    }

};

AbstractNode*
SIRtree::createNode(int level)
{
    AbstractNode* an = new SIRAbstractNode(level, nodeCapacity);
    nodes->push_back(an);
    return an;
}

/**
 * Inserts an item having the given bounds into the tree.
 */
void
SIRtree::insert(double x1, double x2, void* item)
{
    std::unique_ptr<Interval> i{new Interval(std::min(x1, x2), std::max(x1, x2))};
    AbstractSTRtree::insert(i.get(), item);
    intervals.push_back(std::move(i));
}

std::unique_ptr<BoundableList>
SIRtree::sortBoundables(const BoundableList* input)
{
    std::unique_ptr<BoundableList> output(new BoundableList(*input));
    sort(output->begin(), output->end(), compareSIRBoundables);
    //output->sort(compareSIRBoundables);
    return output;
}

} // namespace geos.index.strtree
} // namespace geos.index
} // namespace geos

