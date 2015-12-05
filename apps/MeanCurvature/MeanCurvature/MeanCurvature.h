// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MEANCURVATURE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MEANCURVATURE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MEANCURVATURE_EXPORTS
#define MEANCURVATURE_API __declspec(dllexport)
#else
#define MEANCURVATURE_API __declspec(dllimport)
#endif

#include <LBO.h>
#include <vector>

// This class is exported from the MeanCurvature.dll
class MEANCURVATURE_API MeanCurvature {
public:
	typedef Eigen::SparseMatrix<double> SparseMatrix;

	MeanCurvature();
	~MeanCurvature();

	void AssignMesh(CMesh *mesh_);
	void Execute();

	const std::vector<Vector3D>& GetMeanCurvature() { return meanCurvatureVecs; }

private:
	CMesh *mesh;
	LBO *lbo;
	std::vector<Vector3D> meanCurvatureVecs;
};
