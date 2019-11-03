#ifndef __CBALL_H__
#define __CBALL_H__


#include "d3dUtility.h"
#define M_RADIUS 0.21   // ball radius (= 2.85cm, 1cm = 0.073684)
#define DECREASE_RATE 0.9982 //rate of speed decrease

class CSphere {
private:
	float					center_x, center_y, center_z;
	float                   m_radius;
	float					m_velocity_x, m_velocity_y, m_velocity_z;

public:
	CSphere(void);
	~CSphere(void);

public:
	bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color);
	void destroy(void);

	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld);

	bool hasIntersected(CSphere& ball);
	void hitBy(CSphere& ball);

	void ballUpdate(float timeDiff);

	double getVelocity_X() const { return this->m_velocity_x; }
	double getVelocity_Y() const { return this->m_velocity_y; }
	double getVelocity_Z() const { return this->m_velocity_z; }
	D3DXVECTOR3 getVelocity() const { return D3DXVECTOR3(m_velocity_x, m_velocity_y, m_velocity_z); }

	inline void setPower(double vx, double vz) { setPower(vx, getVelocity_Y(), vz); }
	inline void setPower(double vx, double vy, double vz);
	void setCenter(float x, float y, float z);

	float getRadius(void)  const { return this->m_radius; }
	const D3DXMATRIX& getLocalTransform(void) const;
	void setLocalTransform(const D3DXMATRIX& mLocal);
	D3DXVECTOR3 getCenter(void) const;

private:
	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pSphereMesh;

};

#endif