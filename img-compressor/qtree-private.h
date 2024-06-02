/**
 * @file qtree-private.h
 * @description student declaration of private QTree functions
 *              CPSC 221 PA3
 *
 *              SUBMIT THIS FILE.
 * 
 *				Simply declare your function prototypes here.
 *              No other scaffolding is necessary.
 */

// begin your declarations below 
void Render(unsigned int scale, PNG& img, Node* nd) const;

RGBAPixel nodeAverage(Node* node);

Node* copy(Node* curr, Node* other);

void clear(Node* curr);

Node* FlipHorizontal(Node *nd);


void Swap(Node* n1, Node* n2);

void SwapUneven(Node* n1, Node* n2);


Node* RotateCCW(Node* nd, pair<unsigned int, unsigned int> parent_ul);

pair<unsigned int, unsigned int> CalcNewBounds(Node* nd, pair<unsigned int, unsigned int> parent_ul);


bool GetChildren(Node * root, RGBAPixel avg, double tolerance);

void Prune(Node * node, double tolerance);

Node* Rotate(Node * node);