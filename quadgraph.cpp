#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <math.h>

#define MAX_DEPTH 10
float BINS[11] = { 0,6,7,8,9,10,12,15,20,30,50};


struct Point{
	float x;
	float y;
	float value;

	Point(float x, float y, float value) {
		this->x = x;
		this->y = y;
		this->value = value;
	}
};

struct BBox {
	float x;
	float xx;
	float y;
	float yy;

	BBox(float x, float xx, float y, float yy) {
		this->x = x;
		this->xx = xx;
		this->y = y;
		this->yy = yy;
	};

	Point centroid(void) {
		return Point((float)0.5*(this->x + this->xx), (float)0.5*(this->y + this->yy), 100.0f);
	};

	bool contains(Point* p) {

		return p->x >= this->x &&
			p->x < this->xx &&
			p->y >= this->y &&
			p->y < this->yy;
	};
};

int get_bin_number(float v) {
	unsigned int ix = 0;

	while (ix < sizeof(BINS)/sizeof(*BINS)){
		if (v < BINS[ix]) {
			return ix;
		}
		ix++;
	}
	return ix;
}



bool is_neighbor(BBox* first, BBox* second) {
	if (first->yy == second->y && second->xx >= first->x && second->x <= first->xx) {
		return true;
	}
	else if (first->xx == second->x && second->yy >= first->y && second->y <= first->yy) {
		return true;
	}
	else if (first->y == second->yy && second->xx >= first->x && second->x <= first->xx) {
		return true;
	}
	else if (first->x == second->xx && second->yy >= first->y && second->y <= first->yy) {
		return true;
	}
	else {
		return false;
	}
};

std::vector<BBox*> get_quadrants(BBox* parent) {
	std::vector<BBox*> quadrants;
	float x_half = (float)0.5*(parent->x + parent->xx);
	float y_half = (float)0.5*(parent->y + parent->yy);

	quadrants.push_back(new BBox(parent->x, x_half, y_half, parent->yy));
	quadrants.push_back(new BBox(x_half, parent->xx, y_half, parent->yy));
	quadrants.push_back(new BBox(parent->x, x_half, parent->y, y_half));
	quadrants.push_back(new BBox(x_half, parent->xx, parent->y, y_half));

	return quadrants;
};

struct Tree {
	std::vector<Point*> points;
	std::vector<Tree*> children;
	BBox* bbox;
	std::vector<Tree*> neighbors;
	unsigned int bin;
	unsigned int depth;

	Tree(BBox* bbox) {
		this->bbox = bbox;
		this->bin = 0;
		this->depth = 0;
	};

	~Tree(void) {
		//Perform this recursively
		for (auto & child : this->children) {
			delete child;
		}

		//Delete bounding box
		delete this->bbox;
		
		//Remove all references (pointers) to this
		for (auto & ne : this->neighbors) {
			ne->neighbors.erase(
				std::remove(ne->neighbors.begin(), ne->neighbors.end(), this),
				ne->neighbors.end());
		}
	}

	bool insert_data_point(Point* p) {
		if (!this->bbox->contains(p)) {
			return false;
		}

		if (!this->children.empty()) {
			for (auto & child : this->children) {
				if (child->insert_data_point(p)) {
					return true;
				}
			}
			return false;
		}

		//Leaf node found!
		this->points.push_back(p);
		unsigned int bin = get_bin_number(p->value);
		if (this->points.size() < 2 || this->depth == MAX_DEPTH) {
			this->bin = fmin(bin, this->bin);
		}
		else if (bin != this->bin && this->depth < MAX_DEPTH) {
			this->subdivide();
		}
		return true;
	}

	void subdivide(void) {
		for (auto & quadrant : get_quadrants(this->bbox)) {
			Tree* t = new Tree(quadrant);
			t->depth = this->depth + 1;
			this->children.push_back(t);
		}

		for (auto & child : this->children) {
			for (auto & tmp : this->children) {
				if (child != tmp) {
					child->neighbors.push_back(tmp);
				}
			}

			for (auto & p : this->points) {
				child->insert_data_point(p);
			}

		}

		for (auto & ne : this->neighbors) {
			ne->neighbors.erase(
				std::remove(ne->neighbors.begin(), ne->neighbors.end(), this), 
				ne->neighbors.end());

			for (auto & child : this->children) {
				if (is_neighbor(ne->bbox, child->bbox)) {
					ne->neighbors.push_back(child);
					child->neighbors.push_back(ne);
				}
			}
		}

		this->points.clear();
		this->neighbors.clear();

	}

	void join_tree(Tree* other) {
		if (!is_neighbor(this->bbox, other->bbox)) {
			return;
		}

		if (!this->children.empty()) {
			for (auto & child : this->children) {
				child->join_tree(other);
			}
		}
		else if (other->children.empty()) {
			// Neighboring leaf nodes found!
			this->neighbors.push_back(other);
			other->neighbors.push_back(this);
		}
		else {
			// Leaf n�de of this tree found, find neighboring leaf nodes of other tree
			other->join_tree(this);
		}
	}

	void finish_inserts(void) {
		this->points.clear(); //No longer needed
		for (auto & child : this->children) {
			child->finish_inserts();
		}
	}

};

int main() {
	std::cout << "Starting inserts!" << std::endl;

	unsigned int t0 = (unsigned int)std::time(nullptr);

	std::srand((unsigned int)std::time(nullptr));

	unsigned int NO_OF_POINTS = 230000;

	std::vector<Point> points;

	for (size_t i = 0; i < NO_OF_POINTS; i++) {
		float x = std::rand() / (float)RAND_MAX * 20;
		float y = std::rand() / (float)RAND_MAX * 20 + 45;
		float depth = std::rand() / (float)RAND_MAX * 100;
		points.push_back(Point(x, y, depth));
	}

	std::vector<Tree*> roots;
	roots.push_back(new Tree(new BBox(0, 10, 45, 55)));
	roots.push_back(new Tree(new BBox(10, 20, 45, 55)));
	roots.push_back(new Tree(new BBox(0, 10, 55, 65)));
	roots.push_back(new Tree(new BBox(10, 20, 55, 65)));

	for (auto & p : points) {
		for (auto & r : roots) {
			r->insert_data_point(&p);
		}
	}

	std::cout << sizeof(Tree) << std::endl;
	std::cout << sizeof(Point) << std::endl;
	std::cout << sizeof(BBox) << std::endl;

	for (auto & r : roots) {
		r->finish_inserts();
		for (auto & r2 : roots) {
			if (r == r2) {
				continue;
			}
			r->join_tree(r2);
		}
	}
	points.clear();

	for (auto & r : roots) {
		delete r;
	}
	roots.clear();

	std::cout << (unsigned int)std::time(nullptr) - t0 << std::endl;
	std::cout << "Finished inserting points!" << std::endl;
}