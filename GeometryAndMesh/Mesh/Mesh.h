#ifndef MESH_H
#define MESH_H

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MESH_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MESH_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MESH_EXPORTS
#define MESH_API __declspec(dllexport)
#else
#define MESH_API __declspec(dllimport)
#endif

#include "Model.h"
#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

typedef unsigned int UINT;
typedef std::list<Vector3D>	_VECTORLIST;
typedef std::list<UINT>		_UINTLIST;

#define MAX_VERTEX_PER_FACE 20

class MESH_API CVertex
{
public:
	Vector3D	m_vPosition;		//
	UINT*		m_piEdge;			//Ӹõ㷢İ,ҪݵĶ̬
	_UINTLIST	m_lEdgeList;		//m_piEdgeʱ
	short		m_nValence;			//Ķ
	Vector3D	m_vNormal;			//㷨ɸ淨ƽõ
	Vector3D	m_vRawNormal;		//normals loaded from file
	bool		m_bIsBoundary;		//Ƿڱ߽
	int			m_nCutValence;
	UINT		m_color;			//ڱǿɫϢ;
	bool m_bValid;

public:
	//constructions
	CVertex() { m_piEdge = NULL; m_nValence = 0; m_nCutValence = 0; m_bIsBoundary = false; m_color = 0; m_bValid = true; }
	CVertex(double x, double y, double z) { m_vPosition = Vector3D(x, y, z); m_piEdge = NULL; m_nValence = 0; m_bIsBoundary = false; m_nCutValence = 0; m_bValid = true; }
	CVertex(Vector3D v) { m_vPosition = v; m_piEdge = NULL; m_nValence = 0; m_bIsBoundary = false; m_nCutValence = 0; m_bValid = true; }
	virtual ~CVertex();

	//operations
	CVertex& operator = (CVertex& v);

};

class MESH_API CTexture
{
public:
	Vector2D m_vPosition;
public:
	CTexture() { m_vPosition = Vector2D(0, 0); }
	CTexture(double x, double y) { m_vPosition = Vector2D(x, y); }
	CTexture(Vector2D v) { m_vPosition = v; }
};

class MESH_API CEdge
{
public:
	UINT	m_iVertex[2];		//ߵ˵㣬Vertex0>Vertex1

	UINT	m_iTwinEdge;		//ñ߷෴һߣΪ-1ñΪ߽
	UINT	m_iNextEdge;		//ʱ뷽һ
	UINT	m_iFace;			//ñ棬Ӧ
	UINT	m_color;			//ڱǿɫϢ;
	double  m_length;			//߳;
	bool m_bValid;

public:
	bool	m_bCut;
	int		m_nCutTag;

public:
	//constructions
	CEdge() {
		m_iVertex[0] = m_iVertex[1] = m_iTwinEdge = m_iNextEdge = m_iFace = -1;
		m_bCut = false; m_nCutTag = 0; m_color = 0; m_length = 0; m_bValid = true;
	}
	CEdge(UINT iV0, UINT iV1) { m_iVertex[0] = iV0; m_iVertex[1] = iV1; m_iTwinEdge = m_iNextEdge = m_iFace = -1; m_bCut = false; m_nCutTag = 0; m_bValid = true; }
	virtual ~CEdge();

	//operations
	CEdge& operator = (const CEdge& e);
};

class MESH_API CFace
{
public:
	short	m_nType;		//
	UINT*	m_piVertex;		//е
	UINT*	m_piEdge;		//б
	double* m_pdAngle;
	Vector3D m_vNormal;		//
	Vector3D m_vMassPoint;	//
	double	m_dArea;		//
	UINT	m_color;		//ڱǿɫϢ;
	bool m_bValid;

public:
	//constructions
	CFace() { m_nType = 0; m_piVertex = m_piEdge = NULL; m_pdAngle = NULL; m_vNormal = Vector3D(0.0, 0.0, 1.0); m_dArea = 0.0; m_color = 0; m_bValid = true; }
	CFace(short s);
	virtual ~CFace();

	//operations
	void Create(short s);
	CFace& operator = (const CFace& f);
};

class MESH_API CMesh :public CModel
{
public:
	UINT		m_nVertex;				//
	CVertex*	m_pVertex;				//

	std::vector<CTexture> m_pTexture;
	CTexture maxTex;

	UINT		m_nEdge;				//
	CEdge*		m_pEdge; 				//߱
	UINT		m_nFace;	 			//
	CFace*		m_pFace;				//
	UINT		m_nVertexCapacity;		//ǰб
	UINT		m_nEdgeCapacity;		//ǰ߱
	UINT		m_nFaceCapacity;		//ǰ

	Vector3D color;
	bool isVisible;

	unsigned m_nValidVertNum;
	unsigned m_nValidFaceNum;
	unsigned m_nValidEdgeNum;

	std::vector<Vector3D> isolatedPoints;

	double *m_pAngles;

	std::string		filename;				//ļ
	double scaleD;
	Vector3D origin;
	Vector3D bboxSize;

	//temp
	_UINTLIST m_lFocusEdge;
	_UINTLIST m_lFocusVertex;
	_UINTLIST m_lFocusFace;
	UINT	m_iPickedFace;
	UINT	m_iPickedEdge;
	UINT	m_iPickedVertex;

	bool	m_bClosed;

public:
	CMesh() {
		m_nVertex = m_nEdge = m_nFace = 0;
		m_pVertex = NULL; m_pEdge = NULL; m_pFace = NULL; m_pAngles = NULL;
		m_iPickedFace = m_iPickedEdge = m_iPickedVertex = -1;
		isVisible = true;
	}
	CMesh(CMesh* pMesh);
	virtual ~CMesh();

public:
	bool	Load(const char* sFileName);	// load from file
	bool	Save(const char* sFileName);	// save to file

	MESH_API friend std::istream& operator >> (std::istream &input, CMesh &mesh);

	bool	construct();// construct connectivity

	CMesh*	clone();

	//iEdgeڵ棨Ϊֻ߽һ棩iEdgeϸ֡
	UINT	split(UINT iEdge, double posPercent = -1.0);

	//flip edge whose two adjacent facws are in the same plane
	void flip(unsigned iEdge);

	void collapse(unsigned iEdge, Vector3D newPos);

	//бߵı߳CEdge->m_length
	void	calcAllEdgeLength();
private:
	void	clear();
	bool	reConstruct();// construct connectivity from current mesh
	bool	loadOBJ(const char* sFileName);
	bool	loadOFF(const char* sFileName);
	bool	loadFromSMF(std::list<Vector3D> &VertexList, 
		std::list<UINT> &FaceList, 
		std::vector<Vector3D> &normals);
	bool	saveToSMF(const char* sFileName);

	//iķ
	void	calFaceNormal(UINT i);
	//㶥iķķƽõ
	void	calVertexNormal(UINT i);
	//ебٳȲʱռӱ
	void	expandCapacity();
	//ÿΧĽǶ
	void calcVertexAngle();
};

#endif //MESH_H
