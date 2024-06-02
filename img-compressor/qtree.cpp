/**
 * @file qtree.cpp
 * @description student implementation of QTree class used for storing image data
 *              CPSC 221 PA3
 *
 *              SUBMIT THIS FILE
 */

#include "qtree.h"
#include <iostream>
#include <cmath>

/**
 * Constructor that builds a QTree out of the given PNG.
 * Every leaf in the tree corresponds to a pixel in the PNG.
 * Every non-leaf node corresponds to a rectangle of pixels
 * in the original PNG, represented by an (x,y) pair for the
 * upper left corner of the rectangle and an (x,y) pair for
 * lower right corner of the rectangle. In addition, the Node
 * stores a pixel representing the average color over the
 * rectangle.
 *
 * The average color for each node in your implementation MUST
 * be determined in constant time. HINT: this will lead to nodes
 * at shallower levels of the tree to accumulate some error in their
 * average color value, but we will accept this consequence in
 * exchange for faster tree construction.
 * Note that we will be looking for specific color values in our
 * autograder, so if you instead perform a slow but accurate
 * average color computation, you will likely fail the test cases!
 *
 * Every node's children correspond to a partition of the
 * node's rectangle into (up to) four smaller rectangles. The node's
 * rectangle is split evenly (or as close to evenly as possible)
 * along both horizontal and vertical axes. If an even split along
 * the vertical axis is not possible, the extra line will be included
 * in the left side; If an even split along the horizontal axis is not
 * possible, the extra line will be included in the upper side.
 * If a single-pixel-wide rectangle needs to be split, the NE and SE children
 * will be null; likewise if a single-pixel-tall rectangle needs to be split,
 * the SW and SE children will be null.
 *
 * In this way, each of the children's rectangles together will have coordinates
 * that when combined, completely cover the original rectangle's image
 * region and do not overlap.
 */
QTree::QTree(const PNG& imIn) {
	height = imIn.height();
	width = imIn.width();
	pair<unsigned int, unsigned int> ul = {0,0};
	pair<unsigned int, unsigned int> lr = {width-1, height-1};
	root = BuildNode(imIn, ul,lr);
}

/**
 * Overloaded assignment operator for QTrees.
 * Part of the Big Three that we must define because the class
 * allocates dynamic memory. This depends on your implementation
 * of the copy and clear funtions.
 *
 * @param rhs The right hand side of the assignment statement.
 */
QTree& QTree::operator=(const QTree& rhs) {
	// ADD YOUR IMPLEMENTATION BELOW
	if (this != &rhs) {
		// this and rhs are not the same object in memory
		Clear();
		Copy(rhs);
	}
	return *this;
}

/**
 * Render returns a PNG image consisting of the pixels
 * stored in the tree. may be used on pruned trees. Draws
 * every leaf node's rectangle onto a PNG canvas using the
 * average color stored in the node.
 *
 * For up-scaled images, no color interpolation will be done;
 * each rectangle is fully rendered into a larger rectangular region.
 *
 * @param scale multiplier for each horizontal/vertical dimension
 * @pre scale > 0
 */
PNG QTree::Render(unsigned int scale) const {
	PNG rendered = PNG(width*scale, height*scale);
	Render(scale,rendered,root);
	return rendered;
}


void QTree::Render(unsigned int scale, PNG& img, Node* nd) const {
	if (nd == NULL) {
		return ;
	}

	if (nd->SE == NULL && nd->NE == NULL && nd->SW == NULL && nd->NW == NULL) {
		for (unsigned int x = nd->upLeft.first*scale; x <= (nd->lowRight.first + 1) * scale - 1; x++) {
			for (unsigned int y = nd->upLeft.second*scale; y <= (nd->lowRight.second + 1) * scale - 1; y++) {
				RGBAPixel *pixel = img.getPixel(x,y);
				*pixel = nd->avg;
			}
		}
	}

	Render(scale, img, nd->SE);
	Render(scale, img, nd->NE);
	Render(scale, img, nd->SW);
	Render(scale, img, nd->NW);
	
	return ;
}

/**
 *  Prune function trims subtrees as high as possible in the tree.
 *  A subtree is pruned (cleared) if all of the subtree's leaves are within
 *  tolerance of the average color stored in the root of the subtree.
 *  NOTE - you may use the distanceTo function found in RGBAPixel.h
 *  Pruning criteria should be evaluated on the original tree, not
 *  on any pruned subtree. (we only expect that trees would be pruned once.)
 *
 * You may want a recursive helper function for this one.
 *
 * @param tolerance maximum RGBA distance to qualify for pruning
 * @pre this tree has not previously been pruned, nor is copied from a previously pruned tree.
 */
void QTree::Prune(double tolerance) {
	// ADD YOUR IMPLEMENTATION BELOW
	Prune(root, tolerance);
	
}

void QTree::Prune(Node * node, double tolerance) {
	if (node == NULL) {
		return;
	}
	RGBAPixel avg = node->avg;
	bool withinTolerance = GetChildren(node, avg, tolerance);

	if (withinTolerance) {
		clear(node->SE);
		clear(node->NE);
		clear(node->SW);
		clear(node->NW);
		node->SE = nullptr;
		node->SW = nullptr;
		node->NW = nullptr;
		node->NE = nullptr;
		return;
	} 

	Prune(node->NW, tolerance);
	Prune(node->NE, tolerance);
	Prune(node->SW, tolerance);
	Prune(node->SE, tolerance);
	return;
}



bool QTree::GetChildren(Node * root, RGBAPixel avg, double tolerance) {
	if (root == NULL) {
		return true;
	}

	if (root->SE == NULL && root->NE == NULL && root->SW == NULL && root->NW == NULL) {
		if (avg.distanceTo(root->avg) > tolerance) {
	 		return false;
	 	}
	}

	bool within = true;

	within = GetChildren(root->SE, avg, tolerance);
	if (!within) {
		return within;
	}

	within = GetChildren(root->SW, avg, tolerance);

	if (!within) {
		return within;
	}
	within = GetChildren(root->NE, avg, tolerance);

	if (!within) {
		return within;
	}
	within = GetChildren(root->NW, avg, tolerance);

	if (!within) {
		return within;
	}

	return within;
}


/**
 *  FlipHorizontal rearranges the contents of the tree, so that
 *  its rendered image will appear mirrored across a vertical axis.
 *  This may be called on a previously pruned/flipped/rotated tree.
 *
 *  After flipping, the NW/NE/SW/SE pointers must map to what will be
 *  physically rendered in the respective NW/NE/SW/SE corners, but it
 *  is no longer necessary to ensure that 1-pixel wide rectangles have
 *  null eastern children
 *  (i.e. after flipping, a node's NW and SW pointers may be null, but
 *  have non-null NE and SE)
 * 
 *  You may want a recursive helper function for this one.
 */
void QTree::FlipHorizontal() {
	root = FlipHorizontal(root);
	
}


Node* QTree::FlipHorizontal(Node *nd) {
	if (nd == nullptr) {
		return nullptr;
	}
	Node* NW = nd->NW;
	Node* NE = nd->NE;
	Node* SW = nd->SW;
	Node* SE = nd->SE;
	pair<unsigned int, unsigned int> NW_ul, NW_lr;
	pair<unsigned int, unsigned int> NE_lr;
	pair<unsigned int, unsigned int> SW_ul;
	pair<unsigned int, unsigned int> SE_ul, SE_lr;
	


	if (nd->NE != NULL) {
		NE_lr = {nd->upLeft.first + NE->lowRight.first - NE->upLeft.first, NE->lowRight.second};
		nd->NE->upLeft = nd->upLeft;
		nd->NE->lowRight = NE_lr;
	}
	if (nd->NW != NULL) {
		unsigned int NE_w = 0;
		if (NE != NULL) {
			NE_w = NE->lowRight.first - NE->upLeft.first + 1;
		}
		NW_ul = {nd->upLeft.first + NE_w, nd->upLeft.second};
		NW_lr = {nd->lowRight.first, NW->lowRight.second};
		nd->NW->upLeft = NW_ul;
		nd->NW->lowRight = NW_lr;

	}
	if (nd->SE != NULL) {
		SE_ul = {nd->upLeft.first, SE->upLeft.second};
		SE_lr = {nd->upLeft.first + SE->lowRight.first - SE->upLeft.first, SE->lowRight.second};
		nd->SE->upLeft = SE_ul;
		nd->SE->lowRight = SE_lr;
	}
	if (nd->SW != NULL) {
		unsigned int SE_w = 0;
		if (SE != NULL) {
			SE_w = SE->lowRight.first - SE->upLeft.first + 1;
		}
		SW_ul = {nd->upLeft.first + SE_w, SW->upLeft.second};
		nd->SW->upLeft = SW_ul;
		nd->SW->lowRight = nd->lowRight;
	}
	swap(nd->NW,nd->NE);
	swap(nd->SW,nd->SE);
	Node* old_NW = nd->NW;
	Node* old_NE = nd->NE;
	Node* old_SW = nd->SW;
	Node* old_SE = nd->SE;
	nd->NE = FlipHorizontal(old_NE);
	nd->NW = FlipHorizontal(old_NW);
	nd->SW = FlipHorizontal(old_SW);
	nd->SE = FlipHorizontal(old_SE);
	return nd;
}


/**
 *  RotateCCW rearranges the contents of the tree, so that its
 *  rendered image will appear rotated by 90 degrees counter-clockwise.
 *  This may be called on a previously pruned/flipped/rotated tree.
 *
 *  Note that this may alter the dimensions of the rendered image, relative
 *  to its original dimensions.
 *
 *  After rotation, the NW/NE/SW/SE pointers must map to what will be
 *  physically rendered in the respective NW/NE/SW/SE corners, but it
 *  is no longer necessary to ensure that 1-pixel tall or wide rectangles
 *  have null eastern or southern children
 *  (i.e. after rotation, a node's NW and NE pointers may be null, but have
 *  non-null SW and SE, or it may have null NW/SW but non-null NE/SE)
 *
 *  You may want a recursive helper function for this one.
 */
// void QTree::RotateCCW() {
//     // recalculate width / height
//     // unsigned int temp = width;
//     // width = height; 
//     // height = temp;
//     root = RotateCCW(root, root->upLeft);
// 	// width = root->lowRight.first + 1;
// 	// height = root->lowRight.second + 1;
// 	width = 200;
// 	height = 200;
// }

// Node* QTree::RotateCCW(Node* nd, pair<unsigned int, unsigned int> parent_ul) {
//     if (nd == NULL) {
//      return nd;
//     }
// 	nd->upLeft = parent_ul;

//     Node* NW = nd->NW;
//     Node* NE = nd->NE;
//     Node* SW = nd->SW;
//     Node* SE = nd->SE;

//     pair<unsigned int, unsigned int> NW_ul, NW_lr;
//     pair<unsigned int, unsigned int> NE_ul, NE_lr;
//     pair<unsigned int, unsigned int> SW_ul, SW_lr;
//     pair<unsigned int, unsigned int> SE_ul, SE_lr;
// 	// unsigned int nw_x = nd->upLeft.first, ne_x = nd->upLeft.first, sw_x = nd->upLeft.first, se_x = nd->upLeft.first;
// 	// unsigned int nw_y = nd->upLeft.second, ne_y = nd->upLeft.second, sw_y = nd->upLeft.second, se_y = nd->upLeft.second;
// 	pair<unsigned int, unsigned int> nw, ne, sw, se = nd->upLeft;

//     if (NE != NULL) {
//      NE_ul = nd->upLeft;
//      NE_lr = {nd->upLeft.first + NE->lowRight.second - NE->upLeft.second, nd->upLeft.second + NE->lowRight.first - NE->upLeft.first};
//      nd->NE->upLeft = NE_ul;
//      nd->NE->lowRight = NE_lr;
//     }
//     if (SE != NULL) {
//      unsigned int NE_w = 0;
//      if (NE != NULL) {
//          NE_w = NE->lowRight.second - NE->upLeft.second + 1;
//      }
//      SE_ul = {nd->upLeft.first + NE_w, nd->upLeft.second};
//      SE_lr = {nd->upLeft.first + SE->lowRight.second - SE->upLeft.second + NE_w, nd->upLeft.second + SE->lowRight.first - SE->upLeft.first};
//      nd->SE->upLeft = SE_ul;
//      nd->SE->lowRight = SE_lr;
//     }
//     if (NW != NULL) {
//      unsigned int NE_h = 0;
//      if (NE != NULL || SE != NULL) {
// 		if (NE != NULL) {
// 			NE_h = NE->lowRight.first - NE->upLeft.first + 1;
// 		} else {
// 			NE_h = SE->lowRight.first - SE->upLeft.first + 1;
// 		}
//      } 
//      NW_ul = {nd->upLeft.first, nd->upLeft.second + NE_h};
//      NW_lr = {nd->upLeft.first + NW->lowRight.second - NW->upLeft.second, nd->upLeft.second + NW->lowRight.first - NW->upLeft.first + NE_h};

//      nd->NW->upLeft = NW_ul;
//      nd->NW->lowRight = NW_lr;
//     }
//     if (SW != NULL) {
//      unsigned int NW_w = 0;
//      unsigned int SE_h = 0;
//      if (NE != NULL || SE != NULL) {
// 		if (NE != NULL) {
// 			SE_h = NE->lowRight.first - NE->upLeft.first + 1;
// 		} else {
// 			SE_h = SE->lowRight.first - SE->upLeft.first + 1;
// 		}
//      }
//      if (NW != NULL) {
//          NW_w = NW->lowRight.second - NW->upLeft.second + 1;
//      }
//      SW_ul = {nd->upLeft.first + NW_w, nd->upLeft.second + SE_h};
//      SW_lr = {nd->upLeft.first + SW->lowRight.second - SW->upLeft.second + NW_w, nd->upLeft.second + SW->lowRight.first - SW->upLeft.first + SE_h};

//      nd->SW->upLeft = SW_ul;
//      nd->SW->lowRight = SW_lr;
//     }
// 	swap(nd->NW, nd->NE);
// 	swap(nd->SE, nd->SW);
// 	swap(nd->SW, nd->NE);

//     Node* old_NW = nd->NW;
//     Node* old_NE = nd->NE;
//     Node* old_SW = nd->SW;
//     Node* old_SE = nd->SE;

// 	// if (NW != NULL) {
// 	// 	ne_x += NW->lowRight.first - NW->upLeft.first + 1;
// 	// }
// 	// if (NW != NULL || NE != NULL) {
// 	// 	sw_y = (NW != NULL) ? NW->lowRight.second - NW->upLeft.second + 1 : NE->lowRight.second - NE->upLeft.second + 1;
// 	// 	se_y += (NW != NULL) ? NW->lowRight.second - NW->upLeft.second + 1 : NE->lowRight.second - NE->upLeft.second + 1;
// 	// }
// 	// if (SW != NULL) {
// 	// 	se_x += SW->lowRight.first - SW->upLeft.first + 1;
// 	// }

// 	if (NE != NULL) {
// 		ne = NE->upLeft;
// 	}
// 	if (NW != NULL) {
// 		nw = NW->upLeft;
// 	}
// 	if (SW != NULL) {
// 		sw = SW->upLeft;
// 	}
// 	if (SE != NULL) {
// 		se = SE->upLeft;
// 	}
// 	nd->NE = RotateCCW(old_NE, ne);
//     nd->NW = RotateCCW(old_NW, nw);
//     nd->SW = RotateCCW(old_SW, sw);
//     nd->SE = RotateCCW(old_SE, se);
//     return nd;
// }




/**
 *  RotateCCW rearranges the contents of the tree, so that its
 *  rendered image will appear rotated by 90 degrees counter-clockwise.
 *  This may be called on a previously pruned/flipped/rotated tree.
 *
 *  Note that this may alter the dimensions of the rendered image, relative
 *  to its original dimensions.
 *
 *  After rotation, the NW/NE/SW/SE pointers must map to what will be
 *  physically rendered in the respective NW/NE/SW/SE corners, but it
 *  is no longer necessary to ensure that 1-pixel tall or wide rectangles
 *  have null eastern or southern children
 *  (i.e. after rotation, a node's NW and NE pointers may be null, but have
 *  non-null SW and SE, or it may have null NW/SW but non-null NE/SE)
 *
 *  You may want a recursive helper function for this one.
 */
void QTree::RotateCCW() {
	// ADD YOUR IMPLEMENTATION BELOW
	if (root != NULL) {
		pair<unsigned int, unsigned int> newLR = {root->lowRight.second, root->lowRight.first};
		root->lowRight = newLR;
	}
	root = Rotate(root);
	unsigned int temp = height;
	height = width;
	width = temp;
	
}

Node* QTree::Rotate(Node * node) {
	if (node == NULL) {
		return nullptr;
	}

	vector<Node*> temp = {node->NW, node->NE, node->SW, node->SE};

	node->NW = temp[1];
	node->NE = temp[3];
	node->SW = temp[0];
	node->SE = temp[2];

	int NW_W = 0;
	int NW_H = 0;
	pair<unsigned int, unsigned int> NW_UL;
	pair<unsigned int, unsigned int> NW_LR;

	pair<unsigned int, unsigned int> NE_UL;
	int NE_W;
	int NE_H;
	pair<unsigned int, unsigned int> NE_LR;

	pair<unsigned int, unsigned int> SW_UL;
	int SW_W = 0;
	int SW_H = 0;
	pair<unsigned int, unsigned int> SW_LR;

	pair<unsigned int, unsigned int> SE_UL;
	int SE_W;
	int SE_H;
	pair<unsigned int, unsigned int> SE_LR;


	if (node->NW != NULL) {
		NW_UL = node->upLeft;
		NW_W = temp[1]->lowRight.first - temp[1]->upLeft.first;
		NW_H = temp[1]->lowRight.second - temp[1]->upLeft.second;
		NW_LR = {NW_UL.first + NW_H, NW_UL.second + NW_W};
	}

	if (node->NE != NULL) {
		NE_UL = node->upLeft;
		if (node->NW != NULL) {
			NE_UL = {NW_UL.first + NW_H + 1, NW_UL.second};
		}
		NE_W = temp[3]->lowRight.first - temp[3]->upLeft.first;
		NE_H = temp[3]->lowRight.second - temp[3]->upLeft.second;
		NE_LR = {NE_UL.first + NE_H, NE_UL.second + NE_W};
	}

	if (node->SW != NULL) {
		SW_UL = node->upLeft;
		if (node->NW != NULL) {
			SW_UL = {NW_UL.first, NW_UL.second + NW_W + 1};
		}
		SW_W = temp[0]->lowRight.first - temp[0]->upLeft.first;
		SW_H = temp[0]->lowRight.second - temp[0]->upLeft.second;
		SW_LR = {SW_UL.first + SW_H, SW_UL.second + SW_W};
	}
	
	if (node->SE != NULL) {
		SE_UL = node->upLeft;
		if (node->SW != NULL) {
			SE_UL = {SW_UL.first + SW_H + 1, SW_UL.second};
		} else if (node->NE != NULL) {
			SE_UL = {NE_UL.first, NE_UL.second + NE_W + 1};
		}
		SE_W = temp[2]->lowRight.first - temp[2]->upLeft.first;
		SE_H = temp[2]->lowRight.second - temp[2]->upLeft.second;
		SE_LR = {SE_UL.first + SE_H, SE_UL.second + SE_W};
	}

	if (node->NW != NULL) {
		node->NW->upLeft = NW_UL;
		node->NW->lowRight = NW_LR;
	}

	if (node->NE != NULL) {
		node->NE->upLeft = NE_UL;
		node->NE->lowRight = NE_LR;
	}

	if (node->SW != NULL) {
		node->SW->upLeft = SW_UL;
		node->SW->lowRight = SW_LR;
	}

	if (node->SE != NULL) {
		node->SE->upLeft = SE_UL;
		node->SE->lowRight = SE_LR;
	}


	node->NW = Rotate(node->NW);
	node->NE = Rotate(node->NE);
	node->SE = Rotate(node->SE);
	node->SW = Rotate(node->SW);

	return node;
}

// pair<unsigned int, unsigned int> QTree::CalcNewBounds(Node *nd, pair<unsigned int, unsigned int> parent_ul) {
// 	int nd_width = nd->lowRight.first - nd->upLeft.first + 1;
// 	int nd_height = nd->lowRight.second - nd->upLeft.second + 1;
// 	int diff = nd_width - nd_height;
// 	unsigned int nd_x = nd->upLeft.first;
// 	unsigned int nd_y = nd->upLeft.second;

// 	if (nd_width == nd_height) {
// 		return nd->upLeft;
// 	}
// 	if (nd->upLeft.first != parent_ul.first) {
// 		if (nd_height > nd_width) {
// 			nd_x = nd->upLeft.first + abs(diff);
// 		} else {
// 			nd_x = nd->upLeft.first - abs(diff);
// 		}
// 	}
// 	if (nd->upLeft.second != parent_ul.second) {
// 		if (nd_height > nd_width) {
// 			nd_y = nd->upLeft.second - abs(diff);
// 		} else {
// 			nd_y = nd->upLeft.second + abs(diff);
// 		}
// 	}
// 	pair<unsigned int, unsigned int> res = {nd_x, nd_y};
// 	return res;
// }

/**
 * Destroys all dynamically allocated memory associated with the
 * current QTree object. Complete for PA3.
 * You may want a recursive helper function for this one.
 */
void QTree::Clear() {
	// ADD YOUR IMPLEMENTATION BELOW
	clear(root);
	root = nullptr;
}

void QTree::clear(Node* curr) {
	if (curr == nullptr) {
		return;
	}

	clear(curr->NW);
	clear(curr->NE);
	clear(curr->SW);
	clear(curr->SE);

	Node* temp = curr;
	curr = nullptr;
	delete temp;
	return;
}

/**
 * Copies the parameter other QTree into the current QTree.
 * Does not free any memory. Called by copy constructor and operator=.
 * You may want a recursive helper function for this one.
 * @param other The QTree to be copied.
 */
void QTree::Copy(const QTree& other) {
	// ADD YOUR IMPLEMENTATION BELOW
	root = copy(root, other.root);
}

Node* QTree::copy(Node* curr, Node* other) {
	if (other == nullptr) {
		return curr;
	}
	if (curr == nullptr) {
		curr = new Node({0,0}, {0,0}, RGBAPixel());
	}
	
	curr->avg = other->avg;
	curr->upLeft = {other->upLeft.first, other->upLeft.second};
	curr->lowRight = {other->lowRight.first, other->lowRight.second};

	curr->NW = copy(curr->NW, other->NW);
	curr->NE = copy(curr->NE, other->NE);
	curr->SW = copy(curr->SW, other->SW);
	curr->SE = copy(curr->SE, other->SE);

	return curr;
}

/**
 * Private helper function for the constructor. Recursively builds
 * the tree according to the specification of the constructor.
 * @param img reference to the original input image.
 * @param ul upper left point of current node's rectangle.
 * @param lr lower right point of current node's rectangle.
 */
Node* QTree::BuildNode(const PNG& img, pair<unsigned int, unsigned int> ul, pair<unsigned int, unsigned int> lr) {
	// Replace the line below with your implementation
	int x = lr.first + 1 - ul.first;
	int y = lr.second + 1 - ul.second;

	if (x == 0 || y == 0) {
		return NULL;
	}
	
	Node* nd = new Node(ul, lr, RGBAPixel());

	if (x == 1 && y == 1) {
		RGBAPixel* pixel = img.getPixel(ul.first, ul.second);
		nd->avg = *pixel;
		return nd;
	}
	
	int larger_x = x - (x/2);
	int larger_y = y - (y/2);

	pair<unsigned int, unsigned int> NW_lr = {ul.first + larger_x - 1, ul.second + larger_y - 1};

	pair<unsigned int, unsigned int> NE_ul = {ul.first + larger_x, ul.second};
	pair<unsigned int, unsigned int> NE_lr = {lr.first, ul.second + larger_y - 1};

	pair<unsigned int, unsigned int> SW_ul = {ul.first, ul.second + larger_y};
	pair<unsigned int, unsigned int> SW_lr = {ul.first + larger_x - 1, lr.second};

	pair<unsigned int, unsigned int> SE_ul = {ul.first + larger_x, ul.second + larger_y};

	if (x != 1 && y != 1) {
		nd->NW = BuildNode(img, ul, NW_lr);
		nd->NE = BuildNode(img, NE_ul, NE_lr);
		nd->SW = BuildNode(img, SW_ul, SW_lr);
		nd->SE = BuildNode(img, SE_ul, lr);
	} else if (y == 1) {
		nd->NE = BuildNode(img, NE_ul, NE_lr);
		nd->NW = BuildNode(img, ul, NW_lr);
	} else {
		nd->SW = BuildNode(img, SW_ul, SW_lr);
		nd->NW = BuildNode(img, ul, NW_lr);
	}

	nd->avg = nodeAverage(nd);
	
	return nd;
}

// /*********************************************************/
// /*** IMPLEMENT YOUR OWN PRIVATE MEMBER FUNCTIONS BELOW ***/
// /*********************************************************/

RGBAPixel QTree::nodeAverage(Node* node) {

	int width = 0;
	int height = 0;
	int area = 0;

	Node* NW = node->NW;
	Node* NE = node->NE;
	Node* SW = node->SW;
	Node* SE = node->SE;

	int red = 0;
	int green = 0;
	int blue = 0;
	int totalArea = 0;

	if (NW != nullptr) {
		width = NW->lowRight.first - NW->upLeft.first + 1;
		height = NW->lowRight.second - NW->upLeft.second + 1;
		area = width * height;
		
		red += (NW->avg.r * area);
		green += (NW->avg.g * area);
		blue += (NW->avg.b * area);
		totalArea += area;
	}

	if (NE != nullptr) {
		width = NE->lowRight.first - NE->upLeft.first + 1;
		height = NE->lowRight.second - NE->upLeft.second + 1;
		area = width * height;
		
		red += (NE->avg.r * area);
		green += (NE->avg.g * area);
		blue += (NE->avg.b * area);
		totalArea += area;
	}

	if (SW != nullptr) {
		width = SW->lowRight.first - SW->upLeft.first + 1;
		height = SW->lowRight.second - SW->upLeft.second + 1;
		area = width * height;
		
		red += (SW->avg.r * area);
		green += (SW->avg.g * area);
		blue += (SW->avg.b * area);
		totalArea += area;
	}

	if (SE != nullptr) {
		width = SE->lowRight.first - SE->upLeft.first + 1;
		height = SE->lowRight.second - SE->upLeft.second + 1;
		area = width * height;
		
		red += (SE->avg.r * area);
		green += (SE->avg.g * area);
		blue += (SE->avg.b * area);
		totalArea += area;
	}

	red = red / totalArea;
	green = green / totalArea;
	blue = blue / totalArea;

	RGBAPixel a = RGBAPixel(red, green, blue);
	return a;
}