#include "CSphere.h"

CSphere::CSphere(void)
{
	D3DXMatrixIdentity(&m_mLocal);
	ZeroMemory(&m_mtrl, sizeof(m_mtrl));
	m_radius = M_RADIUS;
	m_velocity_x = 0;
	m_velocity_z = 0;
	id = 0;
	turncheck = 0;
	m_pSphereMesh = NULL;
}
CSphere::~CSphere(void) {}

bool CSphere::create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
{
	if (NULL == pDevice)
		return false;

	m_mtrl.Ambient = color;
	m_mtrl.Diffuse = color;
	m_mtrl.Specular = color;
	m_mtrl.Emissive = d3d::BLACK;
	m_mtrl.Power = 5.0f;

	if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 20, 20, &m_pSphereMesh, NULL)))
		return false;
	return true;
}

void CSphere::destroy(void)
{
	if (m_pSphereMesh != NULL) {
		m_pSphereMesh->Release();
		m_pSphereMesh = NULL;
	}
}

void CSphere::draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
{
	if (NULL == pDevice)
		return;
	pDevice->SetTransform(D3DTS_WORLD, &mWorld);
	pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
	pDevice->SetMaterial(&m_mtrl);
	m_pSphereMesh->DrawSubset(0);
}

bool CSphere::hasIntersected(CSphere& ball)
{
	D3DXVECTOR3 distance = getCenter() - ball.getCenter();
	float sumRad = getRadius() + ball.getRadius();
	return sumRad * sumRad >= D3DXVec3LengthSq(&distance);
}

void CSphere::hitBy(CSphere& ball)
{
	if (!hasIntersected(ball))
		return;
	else
	{    
		if (ball.getturncheck() == 1)
		{
			setid(1);
		}
		if (getturncheck() == 1)
		{
			ball.setid(1);
		}
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
		//center_y = center_y + diffy;/// 원래의 좌표만큼 서로밀어낸다.
		center_z = center_z + diffz;
		other.x = other.x - diffx;
		//other.y = other.y - diffy;
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
		setPower(vThisNewVec.x, vThisNewVec.z);
		ball.setPower(vBallNewVec.x, vBallNewVec.z);
	}
}

void CSphere::ballUpdate(float timeDiff) /*timeDiff-- 초 단위*/
{
	const float TIME_SCALE = 3.3f;
	D3DXVECTOR3 cord = this->getCenter();
	double vx = abs(this->getVelocity_X());
	double vy = abs(this->getVelocity_Y());
	double vz = abs(this->getVelocity_Z());
	double checkx = vx;
	
	if (vx > 0.01 || vy > 0.01 || vz > 0.01)
	{
		float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
		float tY = cord.y;
		float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;
		this->setCenter(tX, tY, tZ);
	}
	else { this->setPower(0, 0); }
	//this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);
	double rate = 1 - (1 - DECREASE_RATE) * timeDiff * 400;
	if (rate < 0)
		rate = 0;
	this->setPower(getVelocity_X() * rate/*, getVelocity_Y() - 0.3 * GRAVITY_CONST * timeDiff*/, getVelocity_Z() * rate);
}
void CSphere::setturncheck(int tck)//turncheck set (지금누구의턴인지)
{
	this->turncheck = tck;


}
void CSphere::setid(int cid)// 지금의 턴인  ball과 충돌했는지 아닌지
{
	this->id = cid;
}

void CSphere::setPower(float vx, float vz)
{
	this->m_velocity_x = vx;
	this->m_velocity_z = vz;
}
void CSphere::setCenter(float x, float y, float z)
{
	D3DXMATRIX m;
	center_x = x;	center_y = y;	center_z = z;
	D3DXMatrixTranslation(&m, x, y, z);
	setLocalTransform(m);
}

const D3DXMATRIX& CSphere::getLocalTransform(void) const { return m_mLocal; }
void CSphere::setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
D3DXVECTOR3 CSphere::getCenter(void) const
{
	D3DXVECTOR3 org(center_x, center_y, center_z);
	return org;
}
