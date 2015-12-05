// MeanCurvature.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MeanCurvature.h"

// This is the constructor of a class that has been exported.
// see MeanCurvature.h for the class definition
MeanCurvature::MeanCurvature()
{
	lbo = NULL;
}

MeanCurvature::~MeanCurvature()
{
	if (lbo) delete lbo;
}

void MeanCurvature::AssignMesh(CMesh *mesh_)
{
	mesh = mesh_;
	lbo = new LBO();
	lbo->AssignMesh(mesh);
	meanCurvatureVecs.resize(mesh->m_nVertex);
}

void MeanCurvature::Execute()
{
	lbo->Construct();
	Eigen::VectorXd x(mesh->m_nVertex);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < mesh->m_nVertex; ++j)
			x[j] = mesh->m_pVertex[j].m_vPosition[i];
		x = -(lbo->LMatrix() * x);
		for (int j = 0; j < mesh->m_nVertex; ++j)
			meanCurvatureVecs[j][i] = x[j];
	}
}
