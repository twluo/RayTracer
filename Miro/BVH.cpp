#include "BVH.h"
#include "Ray.h"
#include "Console.h"
#include "Triangle.h"
#include <algorithm>

struct bucket {
	int num;
	Vector3 min;
	Vector3 max;
};
float calcArea(Vector3 min, Vector3 max) {
	return 2 * ((max.x - min.x) * (max.y - min.y) + (max.x - min.x) * (max.z - min.z) + (max.y - min.y) * (max.z - min.z));
}
void BVH::draw() {
	if (leaf) {
		bbox.draw();
	}
	else {
		leftBox->draw();
		rightBox->draw();
	}
}
float calcCost(bucket *b, int partition, int split ) {
	int numleft = 0;
	int numright = 0;
	for (int i = 0; i < partition; i++) {
		if (i <= split)
			numleft += b[i].num;
		else
			numright += b[i].num;
	}
	return (calcArea(b[0].min, b[split].max) * numleft + calcArea(b[split + 1].min, b[partition - 1].max) * numright);
}


void BVH::count(int &leafCount, int &nodeCount) {
	if (leaf) {
		leafCount++;
		return;
	}
	else
		nodeCount++;
	leftBox->count(leafCount, nodeCount);
	rightBox->count(leafCount, nodeCount);
}

void BVH::countIntersections(int &RBI, int &RLI) {
	if (leaf) {
		RLI += RL;
		RL = 0;
		return;
	}
	else {
		RBI += RB;
		RB = 0;
	}
	leftBox->countIntersections(RBI, RLI);
	rightBox->countIntersections(RBI, RLI);
}

void
BVH::build(Objects * objs)
{
	RB = 0;
	RL = 0;
    // construct the bounding volume hierarchy
    m_objects = objs;
	leaf = false;
	Vector3 bmin;
	Vector3 bmax;
	Vector3 min;
	Vector3 max;
	min = m_objects->at(0)->getCenteroid();
	max = m_objects->at(0)->getCenteroid();
	bmin = m_objects->at(0)->getMin();
	bmax = m_objects->at(0)->getMax();

	//Calculating min and max
	for (int i = 1; i < m_objects->size(); i++) {
		Vector3 tempMin = m_objects->at(i)->getMin();
		Vector3 tempMax = m_objects->at(i)->getMax();
		Vector3 center = m_objects->at(i)->getCenteroid();
		if (center.x < min.x)
			min.x = center.x;
		else if (center.x > max.x) 
			max.x = center.x;
		if (center.y < min.y)
			min.y = center.y;
		else if (center.y > max.y)
			max.y = center.y;
		if (center.z < min.z)
			min.z = center.z;
		else if (center.z > max.z)
			max.z = center.z;

		if (tempMin.x < bmin.x) {
			bmin.x = tempMin.x;
		}
		if (tempMax.x > bmax.x) {
			bmax.x = tempMax.x;
		}
		if (tempMin.y < bmin.y) {
			bmin.y = tempMin.y;
		}
		if (tempMax.y > bmax.y) {
			bmax.y = tempMax.y;
		}
		if (tempMin.z < bmin.z) {
			bmin.z = tempMin.z;
		}
		if (tempMax.z > bmax.z) {
			bmax.z = tempMax.z;
		}
	}
	
	bbox = box(bmin, bmax);

	if (m_objects->size() < 10) {
		leaf = true;
		return;
	}
	//Setting up the bins
	float distx = max.x - min.x;
	float disty = max.y - min.y;
	float distz = max.z - min.z;
	const int partitions = 5;
	bucket xbuck[partitions];
	bucket ybuck[partitions];
	bucket zbuck[partitions];
	float xseg = distx / partitions;
	float yseg = disty / partitions;
	float zseg = distz / partitions; 
	for (int i = 0; i < partitions; i++) {
		xbuck[i].num = 0;
		xbuck[i].max = max;
		xbuck[i].max.x = min.x + (i + 1) * xseg;
		xbuck[i].min = min;
		xbuck[i].min.x = min.x + i * xseg; 

		ybuck[i].num = 0;
		ybuck[i].max = max;
		ybuck[i].max.y = min.y + (i + 1) * yseg;
		ybuck[i].min = min;
		ybuck[i].min.y = min.y + i * yseg;

		zbuck[i].num = 0;
		zbuck[i].max = max;
		zbuck[i].max.z = min.z + (i + 1) * zseg;
		zbuck[i].min = min;
		zbuck[i].min.z = min.z + i * zseg;
	}
	
	//Populating the bins
	bool popx = false;
	bool popy = false;
	bool popz = false;
	for (int i = 0; i < m_objects->size(); i++) {
		Vector3 center = m_objects->at(i)->getCenteroid();
		popx = false;
		popy = false;
		popz = false;
		for (int j = 0; j < partitions; j++) {
			if (!popx) {
				if (center.x <= xbuck[j].max.x) {
					xbuck[j].num++;
					popx = true;
				}
			}
			if (!popy) {
				if (center.y <= ybuck[j].max.y) {
					ybuck[j].num++;
					popy = true;
				}
			}
			if (!popz) {
				if (center.z <= zbuck[j].max.z) {
					zbuck[j].num++;
					popz = true;
				}
			}
		}
	}

	//Calculating the lowest cost
	float xcost = calcCost(xbuck, partitions, 0);
	float ycost = calcCost(ybuck, partitions, 0);
	float zcost = calcCost(zbuck, partitions, 0);
	int xsplit = 0;
	int ysplit = 0;
	int zsplit = 0;
	for (int i = 1; i < partitions; i++) {
		float nCost = calcCost(xbuck, partitions, i);
		if (nCost < xcost) {
			xcost = nCost;
			xsplit = i;
		}
		nCost = calcCost(ybuck, partitions, i);
		if (nCost < ycost) {
			ycost = nCost;
			ysplit = i;
		}
		nCost = calcCost(zbuck, partitions, i);
		if (nCost < zcost) {
			zcost = nCost;
			zsplit = i;
		}
	}
	int axis = 0;
	int split = xsplit;
	float cost = fmin(xcost, fmin(ycost, zcost));
	if (ycost == cost) {
		axis = 1;
		split = ysplit;
	}
	else if (zcost == cost) {
		axis = 2;
		split = zsplit;
	}

	//Splitting 
	Objects *left = new Objects();
	Objects *right = new Objects();
	for (int i = 0; i < m_objects->size(); i++) {
		Vector3 center = m_objects->at(i)->getCenteroid();
		switch (axis) {
		case 0:
			if (center.x <= xbuck[split].max.x) {
				left->push_back(m_objects->at(i));
			}
			else {
				right->push_back(m_objects->at(i));
			}
			break;
		case 1:
			if (center.y <= ybuck[split].max.y) {
				left->push_back(m_objects->at(i));
			}
			else {
				right->push_back(m_objects->at(i));
			}
			break;
		case 2:
			if (center.z <= zbuck[split].max.z) {
				left->push_back(m_objects->at(i));
			}
			else {
				right->push_back(m_objects->at(i));
			}
			break;
		}
	}
	//std::cout << "MIN IS = " << bmin << " AND MAX IS = " << bmax << std::endl;
	/*if (axis == 0)
	for (int i = 0; i < partitions; i++) {
		std::cout << "Bucket #" << i << " has " << xbuck[i].num << " elements and " << xbuck[i].min.x << " min and " << xbuck[i].max.x << " max" << std::endl;
	}
	if (axis == 1)
	for (int i = 0; i < partitions; i++) {
		std::cout << "Bucket #" << i << " has " << ybuck[i].num << " elements and " << ybuck[i].min.y << " min and " << ybuck[i].max.y << " max" << std::endl;
	}
	if (axis == 2)
	for (int i = 0; i < partitions; i++) {
		std::cout << "Bucket #" << i << " has " << zbuck[i].num << " elements and " << zbuck[i].min.z << " min and " << zbuck[i].max.z << " max" << std::endl;
	}*/
	//std::cout << "cost = " << cost << " split = " << split << " axis = " << axis << std::endl;
	
	//std::cout << "Size of left = " << left->size() << " Size of right = " << right->size() << std::endl;
	//std::cout << "min = " << min << " max = " << max << std::endl;
	//std::cout << "bmin = " << bmin << " bmax = " << bmax << std::endl;

	leftBox = new BVH();
	rightBox = new BVH();
	leftBox->build(left);
	rightBox->build(right);


}

bool
BVH::intersect(HitInfo& minHit, const Ray& ray, float tMin, float tMax)
{
    // Here you would need to traverse the BVH to perform ray-intersection
    // acceleration. For now we just intersect every object.
	if (leaf) {
		bool hit = false;
		HitInfo tempMinHit;
		minHit.t = MIRO_TMAX;
		for (size_t i = 0; i < m_objects->size(); ++i)
		{
			if ((*m_objects)[i]->intersect(tempMinHit, ray, tMin, tMax))
			{
				RL++;
				hit = true;
				if (tempMinHit.t < minHit.t)
					minHit = tempMinHit;
			}
		}

		return hit;
	}
	else {
		HitInfo rMinHit;
		HitInfo lMinHit;
		bool right = rightBox->getBox().intersect(rMinHit, ray, tMin, tMax);
		bool left = leftBox->getBox().intersect(lMinHit, ray, tMin, tMax);
		bool value = false;
		if (right && left) {
			RB += 2;
			//TODO: FIND A WAY TO DETERMINE WHICH ONE IS CLOSER;
			if (leftBox->intersect(lMinHit, ray, tMin, tMax)) {
				value = true;
				minHit = lMinHit;
			}
			if (rightBox->intersect(rMinHit, ray, tMin, tMax)) {
				if (value) {
					if (rMinHit.t < minHit.t)
						minHit = rMinHit;
				}
				else
					minHit = rMinHit;
				return true;
			}
			return value;
		}
		else if (right) {
			RB++;
			return rightBox->intersect(minHit, ray, tMin, tMax);
		}
		else if (left) {
			RB++;
			return leftBox->intersect(minHit, ray, tMin, tMax);
		}
		else
			return false; 
	}
	return false;
}
