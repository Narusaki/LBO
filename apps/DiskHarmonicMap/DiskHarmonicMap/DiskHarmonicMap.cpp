// DiskHarmonicMap.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "DiskHarmonicMap.h"
#include <iostream>
#include <ctime>
using namespace std;

// This is the constructor of a class that has been exported.
// see DiskHarmonicMap.h for the class definition
DiskHarmonicMap::DiskHarmonicMap()
{
	lbo = NULL;
	return;
}


DiskHarmonicMap::~DiskHarmonicMap()
{
	if (lbo) delete lbo;
}

void DiskHarmonicMap::AssignMesh(CMesh *mesh_)
{
	mesh = mesh_;
	lbo = new LBO();
	lbo->AssignMesh(mesh);
	x1.resize(mesh->m_nVertex);
	x2.resize(mesh->m_nVertex);
}

void DiskHarmonicMap::Execute()
{
	// construct bondary conditions
	unsigned boundVertCnt = 0;
	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
		if (mesh->m_pVertex[i].m_bIsBoundary)
			++boundVertCnt;
	cout << "# of boundary vertices: " << boundVertCnt << endl;

	vector<unsigned> boundVerts;
	vector<double> boundVertsDouble;
	vector<double> thetas;

	if (!ConstructBoundaryConditions(boundVerts, thetas))
	{
		cout << "Fail to create boundary conditions!" << endl;
		return;
	}

	lbo->Construct();
	// fix Laplacian matrix
	SparseMatrix L = lbo->LMatrix();
	for (unsigned i = 0; i < boundVerts.size(); ++i)
	{
		for (unsigned j = 0; j < mesh->m_pVertex[boundVerts[i]].m_nValence; ++j)
		{
			unsigned opVert = mesh->m_pEdge[mesh->m_pVertex[boundVerts[i]].m_piEdge[j]].m_iVertex[1];
			L.coeffRef(boundVerts[i], opVert) = 0; 
		}
		// for the very first adjacent vertex
		unsigned opVert = mesh->m_pEdge[mesh->m_pEdge[mesh->m_pVertex[boundVerts[i]].m_piEdge[0]].m_iNextEdge].m_iVertex[1];
		L.coeffRef(boundVerts[i], opVert) = 0; 
		// make diagonal to be 1
		L.coeffRef(boundVerts[i], boundVerts[i]) = 1.0;
	}

	Eigen::VectorXd b1(mesh->m_nVertex);
	Eigen::VectorXd b2(mesh->m_nVertex);
	ConstructB(b1, b2, boundVerts, thetas);
	
	clock_t start = clock();
	Eigen::SparseLU<SparseMatrix> chol;
	chol.compute(L);
	x1 = chol.solve(b1); x2 = chol.solve(b2);
	clock_t end = clock();
	cout << "Time consumed: " << (double)(end - start) / (double)CLOCKS_PER_SEC << endl;

	double angleDistortion, areaDistortion;
	computeDistortion(x1, x2, angleDistortion, areaDistortion);
	
	cout << "Angle distortion: " << angleDistortion << endl;
	cout << "Area distortion: " << areaDistortion << endl;
}

bool DiskHarmonicMap::ConstructBoundaryConditions(std::vector<unsigned> &boundVerts, std::vector<double> &thetas)
{
	boundVerts.clear(); thetas.clear();
	unsigned startVert = -1;
	unsigned cntStart = 0;
	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
	{
		if (mesh->m_pVertex[i].m_bIsBoundary)
		{
			++cntStart;
			startVert = i;
			/*if (cntStart == 20) break;*/
			break;
		}
	}

	if (startVert == -1)
	{
		cout << "No boundary!" << endl;
		return false;
	}

	boundVerts.push_back(startVert);
	thetas.push_back(0.0);

	do
	{
		unsigned prevVert = boundVerts.back();
		for (unsigned i = 0; i < mesh->m_pVertex[prevVert].m_nValence; ++i)
		{
			unsigned curEdge = mesh->m_pVertex[prevVert].m_piEdge[i];
			unsigned v0 = mesh->m_pEdge[curEdge].m_iVertex[1];
			if (mesh->m_pEdge[curEdge].m_iTwinEdge != -1) continue;
			/*if (boundVerts.size() >= 2 && v0 == boundVerts[boundVerts.size()-2]) continue;*/

			boundVerts.push_back(v0);
			thetas.push_back(thetas.back() + mesh->m_pEdge[curEdge].m_length);
			break;
		}
	} while (boundVerts.back() != startVert);

	double maxLen = thetas.back();
	boundVerts.pop_back(); thetas.pop_back();

	for (unsigned i = 0; i < thetas.size(); ++i)
		thetas[i] *= (2 * PI / maxLen);
	return true;
}

void DiskHarmonicMap::ConstructB(Eigen::VectorXd &b1, Eigen::VectorXd &b2,
	std::vector<unsigned> &boundVerts, std::vector<double> &thetas)
{
	for (unsigned i = 0; i < mesh->m_nVertex; ++i)
	{
		b1[i] = 0; b2[i] = 0;
	}
	for (unsigned i = 0; i < boundVerts.size(); ++i)
	{
		b1[boundVerts[i]] = cos(thetas[i]);
		b2[boundVerts[i]] = sin(thetas[i]);
	}
}

void DiskHarmonicMap::computeDistortion(Eigen::VectorXd &vt1, Eigen::VectorXd &vt2, 
	double &angleDistortion, double &areaDistortion)
{
	// compute area scale
	double totalArea = 0.0;
	for (unsigned i = 0; i < mesh->m_nFace; ++i)
		totalArea += mesh->m_pFace[i].m_dArea;
	double scale = PI / totalArea;
	totalArea = PI;

	angleDistortion = 0.0; areaDistortion = 0.0;
	for (unsigned i = 0; i < mesh->m_nFace; ++i)
	{
		double curArea = mesh->m_pFace[i].m_dArea * scale;

		Vector2D verts[3];
		for (unsigned j = 0; j < 3; ++j)
			verts[j] = Vector2D(vt1[mesh->m_pFace[i].m_piVertex[j]], vt2[mesh->m_pFace[i].m_piVertex[j]]);
		double L[3];
		for (unsigned j = 0; j < 3; ++j)
			L[j] = (verts[j] - verts[(j + 1) % 3]).length();
		double p = (L[0] + L[1] + L[2]) / 2.0;
		double mappedArea = sqrt(p * (p - L[0]) * (p - L[1]) * (p - L[2]));

		if (mappedArea == 0.0 || curArea == 0.0) continue;
		/*areaDistortion += (curArea - mappedArea) * (curArea - mappedArea);*/
		areaDistortion += 0.5 * (curArea / mappedArea + mappedArea / curArea) * curArea;

		double curAngleDistortion = 0.0;
		bool flag = false;
		for (unsigned j = 0; j < 3; ++j)
		{
			double l0 = mesh->m_pEdge[mesh->m_pFace[i].m_piEdge[j]].m_length;
			double l1 = mesh->m_pEdge[mesh->m_pFace[i].m_piEdge[(j + 2) % 3]].m_length;
			double l2 = mesh->m_pEdge[mesh->m_pFace[i].m_piEdge[(j + 1) % 3]].m_length;

			double acosAngle = (l0*l0 + l1*l1 - l2*l2) / (2 * l0*l1);
			if (acosAngle > 1.0) acosAngle = 1.0;
			if (acosAngle < -1.0) acosAngle = -1.0;
			double curAngle = acos(acosAngle);

			if (tan(curAngle) == 0.0) { flag = true; break; }
			curAngleDistortion += 1.0 / tan(curAngle) * L[(j + 1) % 3] * L[(j + 1) % 3];
			/*angleDistortion += (curAngle - mappedAngle) * (curAngle - mappedAngle);*/
			/*angleDistortion += fabs(mappedAngle/curAngle - 1.0)*(mappedAngle/curAngle - 1.0);*/
		}
		if (flag) continue;
		angleDistortion += curAngleDistortion / (4.0 * mappedArea) * curArea;
	}

	angleDistortion /= totalArea;
	areaDistortion /= totalArea;
}