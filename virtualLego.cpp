////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: 박창현 Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <d3dx9.h>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>

IDirect3DDevice9* Device = NULL;

// window size
const int Width  = 1024;
const int Height = 768;

// There are four balls
// initialize the position (coordinate) of each ball (ball0 ~ ball3)
const float spherePos[4][2] = { {-2.7f,0} , {+2.4f,0} , {3.3f,0} , {-2.7f,-0.9f}}; 
// initialize the color of each ball (ball0 ~ ball3)
const D3DXCOLOR sphereColor[4] = {d3d::RED, d3d::RED, d3d::YELLOW, d3d::WHITE};

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.21   // ball radius (= 2.85cm, 1cm = 0.073684)
#define GRAVITY_CONST 72.128 //gravity constant(=980cm/s = 72.128/s)
// 10.097,  20.189
#define PI 3.14159265
#define M_HEIGHT 0.01 //height of wall
#define DECREASE_RATE 0.9982 //rate of speed decrease

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class CSphere {
private :
	float					center_x, center_y, center_z;
    float                   m_radius;
	float					m_velocity_x;
	float					m_velocity_y;
	float					m_velocity_z;

public:
    CSphere(void)
    {
        D3DXMatrixIdentity(&m_mLocal); //m_mLocal은 클래스 맨 밑바닥에 있다. 로컬 좌표 변환(?)
        ZeroMemory(&m_mtrl, sizeof(m_mtrl)); //m_mtrl은 클래스 맨 밑바닥에 있다. 구체의 표면?
        m_radius = M_RADIUS;
		m_velocity_x = 0;
		m_velocity_y = 0;
		m_velocity_z = 0;
        m_pSphereMesh = NULL;
    }
    ~CSphere(void) {}

public:
    bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power    = 5.0f;
		
        if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
            return false;
        return true;
    }
	
    void destroy(void)
    {
        if (m_pSphereMesh != NULL) {
            m_pSphereMesh->Release();
            m_pSphereMesh = NULL;
        }
    }

    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
		m_pSphereMesh->DrawSubset(0);
    }
	
    bool hasIntersected(CSphere& ball) 
	{
		D3DXVECTOR3 other = ball.getCenter();
		float dx = center_x - other.x;
		float dy = center_y - other.y;
		float dz = center_z - other.z;
		float sumRad = getRadius() + ball.getRadius();
		return (sumRad * sumRad) >= (dx * dx) + (dy * dy) + (dz * dz);
	}
	
	void hitBy(CSphere& ball) 
	{ 
		if (!hasIntersected(ball))
			return;
		else
		{
			D3DXVECTOR3 other = ball.getCenter();
			D3DXVECTOR3 vNorm = getCenter() - ball.getCenter();
			D3DXVec3Normalize(&vNorm, &vNorm);
			float dx = center_x - other.x;
			float dy = center_y - other.y;
			float dz = center_z - other.z;
			float dsum = sqrt((dx * dx) + (dy * dy) + (dz * dz));
			float diffx, diffy, diffz;
			D3DXVECTOR3 diff;
			if (dsum != 0)
			{
				diff = getRadius() * vNorm;
				diffx = getRadius() * dx / dsum - dx / 2;
				diffy = getRadius() * dy / dsum - dy / 2;
				diffz = getRadius() * dz / dsum - dz / 2;
			};
			if (dsum == 0)
			{
				diffx = getRadius() / 2;//// 원래의 xyz 좌표의 차이
				diffy = getRadius() / 2;
				diffz = getRadius() / 2;

			};
			center_x = center_x + diffx;
			center_y = center_y + diffy;/// 원래의 좌표만큼 서로밀어낸다.
			center_z = center_z + diffz;
			other.x = other.x - diffx;
			other.y = other.y - diffy;
			other.z = other.z - diffz;

			 setCenter(center_x, center_y, center_z);
			 ball.setCenter(other.x, other.y, other.z);
			 //center 값 재조정 완료
			 D3DXVECTOR3 vThisNorm = D3DXVec3Dot(&getVelocity(), &vNorm) * vNorm;
			 D3DXVECTOR3 vThisOrtho = getVelocity() - vThisNorm;
			 D3DXVECTOR3 vBallNorm = D3DXVec3Dot(&ball.getVelocity(), &vNorm) * vNorm;
			 D3DXVECTOR3 vBallOrtho = ball.getVelocity() - vBallNorm;
			 D3DXVECTOR3 vThisNewVec = vBallNorm + vThisOrtho;
			 D3DXVECTOR3 vBallNewVec = vThisNorm + vBallOrtho;
			 setPower(vThisNewVec.x, vThisNewVec.y, vThisNewVec.z);
			 ball.setPower(vBallNewVec.x, vBallNewVec.y, vBallNewVec.z);
		}
	}

	void ballUpdate(float timeDiff) /*timeDiff-- 초 단위*/
	{
		const float TIME_SCALE = 3.3;
		D3DXVECTOR3 cord = this->getCenter();
		double vx = abs(this->getVelocity_X());
		double vy = abs(this->getVelocity_Y());
		double vz = abs(this->getVelocity_Z());

		if(vx > 0.01 || vy > 0.01 || vz > 0.01)
		{
			float tX = cord.x + TIME_SCALE*timeDiff*m_velocity_x;
			float tY = cord.y + TIME_SCALE*timeDiff*m_velocity_y;
			float tZ = cord.z + TIME_SCALE*timeDiff*m_velocity_z;
			this->setCenter(tX, tY, tZ);
		}
		else { this->setPower(0,0);}
		//this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);
		double rate = 1 -  (1 - DECREASE_RATE)*timeDiff * 400;
		if(rate < 0 )
			rate = 0;
		this->setPower(getVelocity_X() * rate/*, getVelocity_Y() - 0.3 * GRAVITY_CONST * timeDiff*/, getVelocity_Z() * rate);
	}

	double getVelocity_X() const { return this->m_velocity_x; }
	double getVelocity_Y() const { return this->m_velocity_y; }
	double getVelocity_Z() const { return this->m_velocity_z; }
	D3DXVECTOR3 getVelocity() const { return D3DXVECTOR3(m_velocity_x, m_velocity_y, m_velocity_z); }

	void setPower(double vx, double vz)
	{
		setPower(vx, getVelocity_Y(), vz);
	}
	void setPower(double vx, double vy, double vz)
	{
		this->m_velocity_x = vx;
		this->m_velocity_y = vy;
		this->m_velocity_z = vz;
	}
	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x=x;	center_y=y;	center_z=z;
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}
	
	float getRadius(void)  const { return this->m_radius;  }
    const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
    D3DXVECTOR3 getCenter(void) const
    {
        D3DXVECTOR3 org(center_x, center_y, center_z);
        return org;
    }
	
private:
    D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh*              m_pSphereMesh;
	
};



// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall {

private:
	
    float					m_x;
	float					m_y;
	float					m_z;
	float                   m_width;
    float                   m_depth;
	float					m_height;
	
public:
    CWall(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_width = 0;
        m_depth = 0;
		m_height = 0;
        m_pBoundMesh = NULL;
    }
    ~CWall(void) {}
public:
    bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power    = 5.0f;
		
        m_width = iwidth;
        m_depth = idepth;
		m_height = iheight;
		
        if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
            return false;
        return true;
    }
    void destroy(void)
    {
        if (m_pBoundMesh != NULL) {
            m_pBoundMesh->Release();
            m_pBoundMesh = NULL;
        }
    }
    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);
    }
private:
	float square(float X) { return X * X; }
public:
	bool hasIntersected(CSphere& ball) const
	{
		D3DXVECTOR3 ballCenter = ball.getCenter();
		D3DXVECTOR3 wallCenter = this->getCenter();
		float ballRadius = ball.getRadius();
		float width = getWidth() / 2;
		float height = getHeight() /2;
		float depth = getDepth() /2;
		//충돌하지 않는 경우가 훨씬 많으므로, 충분히 넓은 바운딩박스 안에 들어오지 않는 경우를 먼저 배제한다.
		if (ballCenter.x - ballRadius > wallCenter.x + width || ballCenter.x + ballRadius < wallCenter.x - width)
			return false;
		if (ballCenter.y - ballRadius > wallCenter.y + height || ballCenter.y + ballRadius < wallCenter.y - height)
			return false;
		if (ballCenter.z - ballRadius > wallCenter.z + depth || ballCenter.z + ballRadius < wallCenter.z - depth)
			return false;
		D3DXVECTOR3 closest = closestPoint(ball);
		D3DXVECTOR3 diff = closest - ballCenter;
		//구의 중심과, 구에 가장 가까운 정육면체 위의 점 사이의 거리를 재서 판단한다.
 		return D3DXVec3Length(&diff) <= ballRadius;
	}
	
	//파라미터로 주어진 구에 가장 가까운 직육면체 위의 점을 구한다.
	D3DXVECTOR3 closestPoint(CSphere& ball) const {
		D3DXVECTOR3 result;
		D3DXVECTOR3 ballCenter = ball.getCenter();
		D3DXVECTOR3 wallCenter = this->getCenter();
		float width = getWidth() / 2;
		float height = getHeight() / 2;
		float depth = getDepth() / 2;

		if (ballCenter.x < wallCenter.x - width)
			result.x = wallCenter.x - width;
		else if (ballCenter.x > wallCenter.x + width )
			result.x = wallCenter.x + width;
		else result.x = ballCenter.x;

		if (ballCenter.y < wallCenter.y - height)
			result.y = wallCenter.y - height;
		else if (ballCenter.y > wallCenter.y + height)
			result.y = wallCenter.y + height;
		else result.y = ballCenter.y;

		if (ballCenter.z < wallCenter.z - depth)
			result.z = wallCenter.z - depth;
		else if (ballCenter.z > wallCenter.z + depth)
			result.z = wallCenter.z + depth;
		else result.z = ballCenter.z;

		return result;
	}
	void hitBy(CSphere& ball)
	{
		/*test code*/
		if (!hasIntersected(ball))
		{
			return;
		}
		else
		{
			//공과 벽이 겹칠 경우 뒤로 조금 돌린다.
			//만약 공이 전혀 움직이지 않고 있던 경우 모순이 생기므로 속도를 임의로 설정한다.
			if (ball.getVelocity() == D3DXVECTOR3(0, 0, 0))
				ball.setPower(1, 0, 0);

			D3DXVECTOR3 prevPos = ball.getCenter() - ball.getVelocity();
			D3DXVECTOR3 currentPos = ball.getCenter();
			D3DXVECTOR3 closest = closestPoint(ball);
			float prevDist;
			float currDist;
			prevDist = D3DXVec3Length(&(closest - prevPos));
			currDist = D3DXVec3Length(&(closest - currentPos));
			float multiplier = (ball.getRadius() - prevDist) / (currDist - prevDist);
			ball.setCenter(prevPos.x + multiplier * ball.getVelocity_X(), prevPos.y + multiplier * ball.getVelocity_Y(), prevPos.z + multiplier * ball.getVelocity_Z());

			//새로 속도를 계산한다.
			D3DXVECTOR3 ballToWall = closest - ball.getCenter();
			if (ballToWall == D3DXVECTOR3(0,0,0)) //closest와 구의 중심이 완전히 겹쳐 Normalize할 수 없는 경우는 임의의 벡터를 쓴다.
				ballToWall.x = 1.0f;
			D3DXVec3Normalize(&ballToWall, &ballToWall);
			D3DXVECTOR3 vTowardWall;
			vTowardWall = ballToWall * D3DXVec3Dot(&ballToWall, &ball.getVelocity());
			D3DXVECTOR3 vOrthogonal = ball.getVelocity() - vTowardWall;
			vTowardWall = -vTowardWall;
			D3DXVECTOR3 newVelocity = vTowardWall + vOrthogonal;
			if (newVelocity.y <= 0.1)
				ball.setPower(newVelocity.x, 0, newVelocity.z);
			else
				ball.setPower(newVelocity.x, newVelocity.y, newVelocity.z);

			//뒤로 돌렸던 만큼 다시 앞으로 이동시킨다.
			currentPos = ball.getCenter();
			currentPos += (1 - multiplier) * ball.getVelocity();
			ball.setCenter(currentPos.x, currentPos.y, currentPos.z);
		}
	}    
	
	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_y = y;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}
	
    float getHeight(void) const { return this->m_height; }
	float getWidth(void)  const { return this->m_width;  }
	float getDepth(void)  const { return this->m_depth;  }
	D3DXVECTOR3 getCenter(void) const
	{
		D3DXVECTOR3 org(m_x, m_y, m_z);
		return org;
	}
	
	
private :
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	
	D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh*              m_pBoundMesh;
};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
    CLight(void)
    {
        static DWORD i = 0;
        m_index = i++;
        D3DXMatrixIdentity(&m_mLocal);
        ::ZeroMemory(&m_lit, sizeof(m_lit));
        m_pMesh = NULL;
        m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_bound._radius = 0.0f;
    }
    ~CLight(void) {}
public:
    bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
    {
        if (NULL == pDevice)
            return false;
        if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
            return false;
		
        m_bound._center = lit.Position;
        m_bound._radius = radius;
		
        m_lit.Type          = lit.Type;
        m_lit.Diffuse       = lit.Diffuse;
        m_lit.Specular      = lit.Specular;
        m_lit.Ambient       = lit.Ambient;
        m_lit.Position      = lit.Position;
        m_lit.Direction     = lit.Direction;
        m_lit.Range         = lit.Range;
        m_lit.Falloff       = lit.Falloff;
        m_lit.Attenuation0  = lit.Attenuation0;
        m_lit.Attenuation1  = lit.Attenuation1;
        m_lit.Attenuation2  = lit.Attenuation2;
        m_lit.Theta         = lit.Theta;
        m_lit.Phi           = lit.Phi;
        return true;
    }
    void destroy(void)
    {
        if (m_pMesh != NULL) {
            m_pMesh->Release();
            m_pMesh = NULL;
        }
    }
    bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return false;
		
        D3DXVECTOR3 pos(m_bound._center);
        D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
        D3DXVec3TransformCoord(&pos, &pos, &mWorld);
        m_lit.Position = pos;
		
        pDevice->SetLight(m_index, &m_lit);
        pDevice->LightEnable(m_index, TRUE);
        return true;
    }

    void draw(IDirect3DDevice9* pDevice/*, const D3DXMATRIX& mWorld*/)
    {
        if (NULL == pDevice)
            return;
        D3DXMATRIX m;
        D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
		pDevice->SetTransform(D3DTS_WORLD, &m/*&mWorld*/);
		//pDevice->MultiplyTransform(D3DTS_WORLD, &m);
        pDevice->SetMaterial(&d3d::WHITE_MTRL);
        m_pMesh->DrawSubset(0);
    }

    D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
    DWORD               m_index;
    D3DXMATRIX          m_mLocal;
    D3DLIGHT9           m_lit;
    ID3DXMesh*          m_pMesh;
    d3d::BoundingSphere m_bound;
};


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall	g_legoPlane;
CWall	g_legowall[4];
CSphere	g_sphere[4];
CSphere	g_target_blueball;
CLight	g_light;

double g_camera_pos[3] = {0.0, 5.0, -8.0};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------


void destroyAllLegoBlock(void)
{
}

// initialization
bool Setup()
{
	int i;
	
    D3DXMatrixIdentity(&g_mWorld);
    D3DXMatrixIdentity(&g_mView);
    D3DXMatrixIdentity(&g_mProj);
		
	// create plane and set the position
    if (false == g_legoPlane.create(Device, -1, -1, 9, 0.03f, 6, d3d::GREEN)) return false;
    g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);
	
	// create walls and set the position. note that there are four walls
	if (false == g_legowall[0].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[0].setPosition(0.0f, 0.12f, 3.06f);
	if (false == g_legowall[1].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[1].setPosition(0.0f, 0.12f, -3.06f);
	if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(4.56f, 0.12f, 0.0f);
	if (false == g_legowall[3].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[3].setPosition(-4.56f, 0.12f, 0.0f);

	// create four balls and set the position
	for (i=0;i<4;i++) {
		if (false == g_sphere[i].create(Device, sphereColor[i])) return false;
		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS , spherePos[i][1]);
		g_sphere[i].setPower(0,0);
	}
	
	// create blue ball for set direction
    if (false == g_target_blueball.create(Device, d3d::BLUE)) return false;
	g_target_blueball.setCenter(.0f, (float)M_RADIUS , .0f);
	
	// light setting 
    D3DLIGHT9 lit;
    ::ZeroMemory(&lit, sizeof(lit));
    lit.Type         = D3DLIGHT_POINT;
    lit.Diffuse      = d3d::WHITE; 
	lit.Specular     = d3d::WHITE * 0.9f;
    lit.Ambient      = d3d::WHITE * 0.9f;
    lit.Position     = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
    lit.Range        = 100.0f;
    lit.Attenuation0 = 0.0f;
    lit.Attenuation1 = 0.9f;
    lit.Attenuation2 = 0.0f;
    if (false == g_light.create(Device, lit))
        return false;
	
	// Position and aim the camera.
	D3DXVECTOR3 pos(0.0f, 5.0f, -8.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);
	
	// Set the projection matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
        (float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);
	
    // Set render states.
    Device->SetRenderState(D3DRS_LIGHTING, TRUE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
    Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	
	g_light.setLight(Device, g_mWorld);
	return true;
}

void Cleanup(void)
{
    g_legoPlane.destroy();
	for(int i = 0 ; i < 4; i++) {
		g_legowall[i].destroy();
	}
    destroyAllLegoBlock();
    g_light.destroy();
}


// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
	int i=0;
	int j = 0;


	if( Device )
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();
		
		// update the position of each ball. during update, check whether each ball hit by walls.
		for( i = 0; i < 4; i++) {
			g_sphere[i].ballUpdate(timeDelta);
			for(j = 0; j < 4; j++){ g_legowall[i].hitBy(g_sphere[j]); }
		}

		// check whether any two balls hit together and update the direction of balls
		for(i = 0 ;i < 4; i++){
			for(j = 0 ; j < 4; j++) {
				if(i >= j) {continue;}
				g_sphere[i].hitBy(g_sphere[j] /*TODO::, timeDelta*/);
			}
		}

		// draw plane, walls, and spheres
		g_legoPlane.draw(Device, g_mWorld);
		for (i=0;i<4;i++) 	{
			g_legowall[i].draw(Device, g_mWorld);
			g_sphere[i].draw(Device, g_mWorld);
		}
		g_target_blueball.draw(Device, g_mWorld);
        g_light.draw(Device/*, g_mWorld*/);
		
		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture( 0, NULL );
	}
	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false;
	static bool isReset = true;
    static int old_x = 0;
    static int old_y = 0;
    static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;
	
	switch( msg ) {
	case WM_DESTROY:
        {
			::PostQuitMessage(0);
			break;
        }
	case WM_KEYDOWN:
        {
            switch (wParam) {
            case VK_ESCAPE:
				::DestroyWindow(hwnd);
                break;
            case VK_RETURN:
                if (NULL != Device) {
                    wire = !wire;
                    Device->SetRenderState(D3DRS_FILLMODE,
                        (wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
                }
                break;
            case VK_SPACE:
				
				D3DXVECTOR3 targetpos = g_target_blueball.getCenter();
				D3DXVECTOR3	whitepos = g_sphere[3].getCenter();
				double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
					pow(targetpos.z - whitepos.z, 2)));		// 기본 1 사분면
				if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0) { theta = -theta; }	//4 사분면
				if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0) { theta = PI - theta; } //2 사분면
				if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0){ theta = PI + theta; } // 3 사분면
				double distance = sqrt(pow(targetpos.x - whitepos.x, 2) + pow(targetpos.z - whitepos.z, 2));
				g_sphere[3].setPower(distance * cos(theta), distance * sin(theta));

				break;

			}
			break;
        }
		
	case WM_MOUSEMOVE:
        {
            int new_x = LOWORD(lParam);
            int new_y = HIWORD(lParam);
			float dx;
			float dy;
			
            if (LOWORD(wParam) & MK_LBUTTON) {
				
                if (isReset) {
                    isReset = false;
                } else {
                    D3DXVECTOR3 vDist;
                    D3DXVECTOR3 vTrans;
                    D3DXMATRIX mTrans;
                    D3DXMATRIX mX;
                    D3DXMATRIX mY;
					
                    switch (move) {
                    case WORLD_MOVE:
                        dx = (old_x - new_x) * 0.01f;
                        dy = (old_y - new_y) * 0.01f;
                        D3DXMatrixRotationY(&mX, dx);
                        D3DXMatrixRotationX(&mY, dy);
                        g_mWorld = g_mWorld * mX * mY;
						
                        break;
                    }
                }
				
                old_x = new_x;
                old_y = new_y;

            } else {
                isReset = true;
				
				if (LOWORD(wParam) & MK_RBUTTON) {
					dx = (old_x - new_x);// * 0.01f;
					dy = (old_y - new_y);// * 0.01f;
		
					D3DXVECTOR3 coord3d=g_target_blueball.getCenter();
					g_target_blueball.setCenter(coord3d.x+dx*(-0.007f),coord3d.y,coord3d.z+dy*0.007f );
				}
				old_x = new_x;
				old_y = new_y;
				
                move = WORLD_MOVE;
            }
            break;
        }
	}
	
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
    srand(static_cast<unsigned int>(time(NULL)));
	
	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
	
	if(!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}
	
	d3d::EnterMsgLoop( Display );
	
	Cleanup();
	
	Device->Release();
	
	return 0;
}