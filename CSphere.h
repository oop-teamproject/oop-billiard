#ifndef __CBALL_H__
#define __CBALL_H__


#include "d3dUtility.h"
#define M_RADIUS 0.21f   // ball radius (= 2.85cm, 1cm = 0.073684)
#define DECREASE_RATE 0.9982f //rate of speed decrease

class CSphere {
private:
	float					center_x, center_y, center_z;
	float                   m_radius;
	float					m_velocity_x, m_velocity_y, m_velocity_z;
	int					    ballStopped;		   //if the ball has been stopped after hit the ball
	int                     id;                    // id=1; ball's turn id=0; not his turn; if the ball intersected with id=1; ball it can
	int					    turncheck;             // Which ball is the turn; turncheck=1 is his turn turncheck=0; is not
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
	void setballStopped(int Sx);
	void setturncheck(int tck);
	void setid(int cid);

	float getVelocity_X() const { return this->m_velocity_x; }
	float getVelocity_Y() const { return this->m_velocity_y; }
	float getVelocity_Z() const { return this->m_velocity_z; }
	float getballStopped() const { return this->ballStopped; }
	float getid() const { return this->id; }
	float getturncheck() const { return this->turncheck;}
	D3DXVECTOR3 getVelocity() const { return D3DXVECTOR3(m_velocity_x, m_velocity_y, m_velocity_z); }

	inline void setPower(float vx, float vz) { setPower(vx, getVelocity_Y(), vz); }
	inline void setPower(float vx, float vy, float vz);
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