// unsigned char a;

// void ray_parameter (octree *oct, ray x) {
// 	a = 0;
// 	if (r.dx < 0.0) {
// 		r.ox = oct->sizeX - r.ox;
// 		r.dx = -r.dx;
// 		a |= 4;//	activate the third byte
// 	}
// 	if (r.dy < 0.0) {
// 		r.oy = oct->sizeY - r.oy;
// 		r.dy = -r.dy;
// 		a |= 2;//	activate the second byte
// 	}
// 	if (r.dx < 0.0) {
// 		r.ox = oct->sizeX - r.ox;
// 		r.dx = -r.dx;
// 		a |= 1;//	activate the first byte
// 	}

// 	tx0 = (oct->xmin - r.ox) / r.dx;
// 	tx1 = (oct->xmax - r.ox) / r.dx;
// 	ty0 = (oct->ymin - r.oy) / r.dy;
// 	ty1 = (oct->ymax - r.oy) / r.dy;
// 	tz0 = (oct->zmin - r.oz) / r.dz;
// 	tz1 = (oct->zmax - r.oz) / r.dz;

// 	if (max(tx0, ty0, tz0) < min(tx1, ty1, tz1))
// 		proc_subtree(tx0, ty0, tz0, tx1, ty1, tz1, oct->root);
// }

// void proc_subtree(double tx0, double ty0, double tz0, double tx1, double ty1, double tz1, node *n) {
// 	double txm, tym, tzm;
// 	int current;

// 	if (tx1 < 0 || ty1 < 0 || tz1 < 0)
// 		return

// 	if (n->type == TERMINAL) {
// 		proc_terminal(n);
// 		return;
// 	}

// 	txm = .5 * (tx0 + tx1);
// 	tym = .5 * (ty0 + ty1);
// 	tzm = .5 * (tz0 + tz1);

// 	current = first_node(tx0, ty0, tz0, txm, tym, tzm);

// 	do {
// 		switch (current)
// 		{
// 			case 0:
// 				proc_subtree(tx0, ty0, tz0, txm, tym, tzm, n->son[a]);
// 				current = new_node(txm, 4, tym, 2, tzm, 1);
// 				break;
// 			case 1:
// 				proc_subtree(tx0, ty0, tzm, txm, tym, tz1, n->son[1^a]);
// 				current = new_node(txm, 5, tym, 3, tz1, 8);
// 				break;
// 			case 2:
// 				proc_subtree(tx0, tym, tz0, txm, ty1, tzm, n->son[2^a]);
// 				current = new_node(txm, 6, ty1, 8, tzm, 3);
// 				break;
// 			case 3:
// 				proc_subtree(tx0, tym, tzm, txm, ty1, tz1, n->son[3^a]);
// 				current = new_node(txm, 7, ty1, 8, tz1, 8);
// 				break;
// 			case 4:
// 				proc_subtree(txm, ty0, tz0, tx1, tym, tzm, n->son[4^a]);
// 				current = new_node(tx1, 8, tym, 6, tzm, 5);
// 				break;
// 			case 5:
// 				proc_subtree(txm, ty0, tzm, tx1, tym, tz1, n->son[5^a]);
// 				current = new_node(tx1, 8, tym, 7, tz1, 8);
// 				break;
// 			case 6:
// 				proc_subtree(txm, tym, tz0, tx1, ty1, tzm, n->son[6^a]);
// 				current = new_node(tx1, 8, ty1, 8, tzm, 7);
// 				break;
// 			case 7:
// 				proc_subtree(txm, tym, tzm, tx1, ty1, tz1, n->son[7^a]);
// 				current = 8;
// 				break;
// 		}
// 	} while (current < 8)
// }

// void proc_terminal (node *n) { ; }

// int first_node (double tx0, double ty0, double tz0, double txm, double tym, double tzm) {
// 	//	Table 2: plane
// 	char plane;
// 	//	0 -> YZ
// 	//	1 -> XZ
// 	//	2 -> XY
// 	if (tx0 > ty0)
// 		if (tx0 > tz0)
// 			plane = 0;
// 		else
// 			plane = 2;
// 	else
// 		if (ty0 > tz0)
// 			plane = 1;
// 		else
// 			plane = 2;

// 	//	Table 1: first node index
// 	switch (plane)
// 	{
// 		case 0:
// 			return (txm < tz0) | ((tym < tz0) << 1);
// 		case 1:
// 			return (txm < ty0) | ((tzm < ty0) << 2);
// 		case 2:
// 			return ((tym < tx0) << 1) | ((tzm < tx0) << 2);
// 	}
// }

// int new_node (double f1, int i1, double f2, int i2, double f3, int i3) {
// 	if (f1 < f2)
// 		if (f1 < f3)
// 			return i1;
// 	else
// 		if (f2 < f3)
// 			return i2;
// 	return i3;
// }