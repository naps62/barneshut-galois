public BuildBVH(List<GameObject> gobjectlist, cBVH lparent, int start, int end, Axis axisid)
{
    GameObject newobj;
    int center;
    int loop;
    int count = end – start;
    List<GameObject> newgolist = newList<GameObject>();
 
    parent = lparent; // save off the parent BVHGObj Node
    // Early out check due to bad data
    // If the list is empty then we have no BVHGObj, or invalid parameters are passed in
    if (gobjectlist == null || end < start)
    {
        minX = 0;
        maxX = 0;
        minY = 0;
        maxY = 0;
        minZ = 0;
        maxZ = 0;
        prev = null;
        next = null;
        gobjects = null;

return;
    }
 
    // Check if we’re at our LEAF node, and if so, save the objects and stop recursing.  Also store the min/max for the leaf node and update the parent appropriately
    if (count <5)
    {
        // We need to find the aggregate min/max for all 5 remaining objects
        // Start by recording the min max of the first object to have a starting point, then we’ll loop through the remaining
        minX = gobjectlist[start].objectpos.X – gobjectlist[start].radius;
        maxX = gobjectlist[start].objectpos.X + gobjectlist[start].radius;
        minY = gobjectlist[start].objectpos.Y – gobjectlist[start].radius;
        maxY = gobjectlist[start].objectpos.Y + gobjectlist[start].radius;
        minZ = gobjectlist[start].objectpos.Z – gobjectlist[start].radius;
        maxZ = gobjectlist[start].objectpos.Z + gobjectlist[start].radius;
        // once we reach the leaf node, we must set prev/next to null to signify the end
        prev = null;
        next = null;
        // at the leaf node we store the remaining objects, so initialize a list
        gobjects = newList<GameObject>();
        // loop through all the objects to add them to our leaf node, and calculate the min/max values as we go 
        for (loop = start; loop <= end; loop++)
        {
            // test min X and max X against the current bounding volume
            if ((gobjectlist[loop].objectpos.X – gobjectlist[loop].radius) < minX)
                minX = (gobjectlist[loop].objectpos.X – gobjectlist[loop].radius);
            if ((gobjectlist[loop].objectpos.X + gobjectlist[loop].radius) > maxX)
                maxX = (gobjectlist[loop].objectpos.X + gobjectlist[loop].radius);
            // Update the leaf node’s parent if appropriate with the min/max
            if (parent != null && minX < parent.minX)
                parent.minX = minX;
            if (parent != null && maxX > parent.maxX)
                parent.maxX = maxX;
            // test min Y and max Y against the current bounding volume
            if ((gobjectlist[loop].objectpos.Y – gobjectlist[loop].radius) < minY)
                minY = (gobjectlist[loop].objectpos.Y – gobjectlist[loop].radius);
            if ((gobjectlist[loop].objectpos.Y + gobjectlist[loop].radius) > maxY)
                maxY = (gobjectlist[loop].objectpos.Y + gobjectlist[loop].radius);
            // Update the leaf node’s parent if appropriate with the min/max
            if (parent != null && minY < parent.minY)
                parent.minY = minY;
            if (parent != null && maxY > parent.maxY)
                parent.maxY = maxY;
 
            // test min Z and max Z against the current bounding volume
            if ( (gobjectlist[loop].objectpos.Z – gobjectlist[loop].radius) < minZ )
                minZ = (gobjectlist[loop].objectpos.Z – gobjectlist[loop].radius);
            if ( (gobjectlist[loop].objectpos.Z + gobjectlist[loop].radius) > maxZ )
                maxZ = (gobjectlist[loop].objectpos.Z + gobjectlist[loop].radius);
            // Update the leaf node’s parent if appropriate with the min/max
            if (parent != null && minZ < parent.minZ)
                parent.minZ = minZ;
            if (parent != null && maxZ > parent.maxZ)
                parent.maxZ = maxZ;
            // store our object into this nodes object list
            gobjects.Add(gobjectlist[loop]);
            // store this leaf node back in out object so we can quickly find what leaf node our object is stored in
            gobjectlist[loop].nodeBVHGObj = this;
        }
        // done with this branch, return recursively and on return update the parent min/max bounding volume
        return;
    }
 
    // if we have more than one object then sort the list and create the bvhGObj
    for (loop = start; loop <= end; loop++) // first create a new list using just the subject of objects from the old list
    {
        newgolist.Add(gobjectlist[loop]);
    }
    switch (axisid) // sort along the appropriate axis
    {
        caseAxis.X: // X
            newgolist.Sort(delegate(GameObject go1, GameObject go2) { return go1.objectpos.X.CompareTo(go2.objectpos.X); }); // Sort the game object by object position along the X axis
            break;
        caseAxis.Y: // Y
            newgolist.Sort(delegate(GameObject go1, GameObject go2) { return go1.objectpos.Y.CompareTo(go2.objectpos.Y); }); // Sort the game object by object position along the X axis
            break;
        caseAxis.Z: // Z
            newgolist.Sort(delegate(GameObject go1, GameObject go2) { return go1.objectpos.Z.CompareTo(go2.objectpos.Z); }); // Sort the game object by object position along the X axis
            break;
    }
    center = (int) (count * 0.5f); // Find the center object in our current sub-list
    // Initialize the branch to a starting value, then we’ll update it based on the leaf node recursion updating the parent
    minX = newgolist[0].objectpos.X – newgolist[0].radius;
    maxX = newgolist[0].objectpos.X + newgolist[0].radius;
    minY = newgolist[0].objectpos.Y – newgolist[0].radius;
    maxY = newgolist[0].objectpos.Y + newgolist[0].radius;
    minZ = newgolist[0].objectpos.Z – newgolist[0].radius;
    maxZ = newgolist[0].objectpos.Z + newgolist[0].radius;
    gobjects = null;
    // if we’re here then we’re still in a leaf node.  therefore we need to split prev/next and keep branching until we reach the leaf node
    prev = newBuildBVH(newgolist, this, 0, center, NextAxis(axisid)); // Split the Hierarchy to the left
    next = newBuildBVH(newgolist, this, center + 1, count, NextAxis(axisid)); // Split the Hierarchy to the right
    // Update the parent bounding box to ensure it includes the children. Note: the leaf node already updated it’s parent, but now that parent needs to keep updating it’s branch parent until we reach the root level
    if (parent != null && minX < parent.minX)
        parent.minX = minX;
    if (parent != null && maxX > parent.maxX)
        parent.maxX = maxX;
    if (parent != null && minY < parent.minY)
        parent.minY = minY;
    if (parent != null && maxY > parent.maxY)
        parent.maxY = maxY;
    if (parent != null && minZ < parent.minZ)
        parent.minZ = minZ;
    if (parent != null && maxZ > parent.maxZ)
        parent.maxZ = maxZ;
}