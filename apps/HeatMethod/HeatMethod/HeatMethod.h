// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the HEATMETHOD_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// HEATMETHOD_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef HEATMETHOD_EXPORTS
#define HEATMETHOD_API __declspec(dllexport)
#else
#define HEATMETHOD_API __declspec(dllimport)
#endif

#include <LBO.h>
#include <list>

// This class is exported from the HeatMethod.dll
class HEATMETHOD_API HeatMethod {
public:
	typedef Eigen::SparseMatrix<double> SparseMatrix;

	HeatMethod();
	~HeatMethod();

	void SetOutputInfo(bool outputInfo_);
	void AssignMesh(CMesh *mesh_);
	void AssignStep(double step_);
	void PreFactor();
	void AssignSources(const std::list<unsigned int> &sources_);
	void BuildDistanceField();
	const Eigen::VectorXd & GetDistances();

private:
	double CalcMeanEdgeLen();
	void ConstructDelta(Eigen::VectorXd &delta);

private:
	CMesh *mesh;
	double step;
	LBO *lbo;
	Eigen::VectorXd phi;

	Eigen::SimplicialCholesky<SparseMatrix> chol1, chol2;
	std::list<unsigned int> sources;

	std::vector<Vector3D> X;
	Eigen::VectorXd GX;
	Eigen::VectorXd delta;

	bool outputInfo;
};
