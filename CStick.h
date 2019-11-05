#ifndef __CSTICK_H__
#define __CSTICK_H__

#include "d3dUtility.h"
#include "CSphere.h"

class CStick {
private:
	float                   v_x, v_y, v_z; //당구채가 가리키는 방향.
	float					m_x, m_y, m_z; //당구채가 가리키는 위치.
	float					distance;	   //(당구채 머리쪽 끝이 현재 있는 위치)와 m 사이의 거리
public:
	CStick(void) {
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		v_x = 0;
		v_y = 0;
		v_z = 0;
		m_x = 0;
		m_y = 0;
		m_z = 0;
		distance = 0;
		m_pBoundMesh = NULL;
	}
	~CStick(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color) {
		{
			if (NULL == pDevice)
				return false;

			m_mtrl.Ambient = color;
			m_mtrl.Diffuse = color;
			m_mtrl.Specular = color;
			m_mtrl.Emissive = d3d::BLACK;
			m_mtrl.Power = 5.0f;

			if (FAILED(D3DXCreateCylinder(pDevice, 0.04f, 0.16f, 4.0f, 20, 20, &m_pBoundMesh, NULL)))
				return false;
			return true;
		}
	}
	void destroy(void) {
		if (m_pBoundMesh != NULL) {
			m_pBoundMesh->Release();
			m_pBoundMesh = NULL;
		}
	}
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld) {
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mRotate);
		pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);
	}
	void setPosition(float x, float y, float z) {
		D3DXMATRIX m;
		this->m_x = x;
		this->m_y = y;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}
	void setDirection(float x, float y, float z) {
		v_x = x;
		v_y = y;
		v_z = z;
		D3DXMATRIX vX;
		D3DXMATRIX vY;
		D3DXMATRIX vZ;
		D3DXMatrixRotationX(&vX, x);
		D3DXMatrixRotationY(&vY, y);
		D3DXMatrixRotationZ(&vZ, z);
		m_mRotate = vX * vY * vZ;
	}

	float getDistance() const { return distance; }
	float getX() const { return m_x; }
	float getY() const { return m_y; }
	float getZ() const { return m_z; }
	D3DXVECTOR3 getCenter(void) const { return D3DXVECTOR3(m_x, m_y, m_z); }
	D3DXVECTOR3 getViewPoint(void) const { return D3DXVECTOR3(v_x, v_y, v_z); }
private:
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

	D3DXMATRIX              m_mLocal;
	D3DXMATRIX				m_mRotate;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pBoundMesh;
};

#endif