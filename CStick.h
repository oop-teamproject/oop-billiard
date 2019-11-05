#ifndef __CSTICK_H__
#define __CSTICK_H__

#include "d3dUtility.h"
#include "CSphere.h"
#include <cmath>
#define PI 3.14159265
class CStick {
private:
	float                   v_x, v_y, v_z; //당구채가 가리키는 방향(라디안). 각각 x, y, z축으로 회전한 정도
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

			if (FAILED(D3DXCreateCylinder(pDevice, 0.16f, 0.04f, 4.0f, 20, 20, &m_pBoundMesh, NULL)))
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
	void viewAt(float x, float y, float z) {
		//아직 완전히 구현되지 않음. phi는 얼추 맞는 것 같은데 theta가 잘 안맞는다.
		x -= m_x;
		y -= m_y;
		z -= m_z;
		if (y == 0 && z == 0) return;
		if (x == 0 && z == 0) return;
		float theta = std::atan2f(y, z); //yz평면에 내렸을 때 x축 방향으로 회전한 각도
		float phi = std::atan2f(x, z); //xz평면에 내렸을 때 y축 방향으로 회전한 각도
		D3DXMATRIX vX;
		D3DXMATRIX vY;
		if(z < 0)//0 < theta < 2PI
			D3DXMatrixRotationX(&vX, -theta + PI);
		else
			D3DXMatrixRotationX(&vX, -theta);
		D3DXMatrixRotationY(&vY, phi);

		m_mRotate = vY * vX;
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