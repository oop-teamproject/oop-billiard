#ifndef __CLIGHT_H__
#define __CLIGHT_H__
#include "d3dUtility.h"

class CLight {
public:
	CLight(void);
	~CLight(void);
public:
	bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f);
	void destroy(void);
	bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld);
	void draw(IDirect3DDevice9* pDevice/*, const D3DXMATRIX& mWorld*/);
	D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }
private:
	DWORD               m_index;
	D3DXMATRIX          m_mLocal;
	D3DLIGHT9           m_lit;
	ID3DXMesh* m_pMesh;
	d3d::BoundingSphere m_bound;
};

#endif