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
#include "CSphere.h"
#include "CWall.h"
#include "CLight.h"
#include "CStick.h"
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
const float spherePos[4][2] = { {-2.7f,0} , {+2.4f,0} , {3.3f,0} , {0.0f, 1.0f}}; 
// initialize the color of each ball (ball0 ~ ball3)
const D3DXCOLOR sphereColor[4] = {d3d::RED, d3d::RED, d3d::YELLOW, d3d::WHITE};

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;  
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define GRAVITY_CONST 72.128 //gravity constant(=980cm/s = 72.128/s)
// 10.097,  20.189
#define PI 3.14159265
#define M_HEIGHT 0.01 //height of wall

#define PLANE_WIDTH 9.0f //default 9.0f
#define PLANE_DEPTH 6.0f //default 6.0f
#define WALL_THICKNESS 0.12f //default 0.12f
#define WALL_HEIGHT 0.3f  //default 0.3f

// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall	g_legoPlane;
CWall	g_legowall[4];
CSphere	g_sphere[4];
CSphere	g_target_blueball;
CLight	g_light;
CStick  g_stick;


double g_camera_pos[3] = {0.0, 5.0, -8.0};

bool updateDone = true;
/*
updateDone이 true인 동안에는 검사하지 않는다.

스페이스바를 누르면 false로 바꾼다
공이 다 멈춰있는지를 매 턴 검사
공이 다 멈추면 업데이트를 하고 true로 바꾼다
*/

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
    if (false == g_legoPlane.create(Device, PLANE_WIDTH, 0.03f, PLANE_DEPTH, d3d::GREEN)) return false;
    g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);
	
	// create walls and set the position. note that there are four walls
	if (false == g_legowall[0].create(Device, PLANE_WIDTH, WALL_HEIGHT, WALL_THICKNESS, d3d::CYAN)) return false;
	g_legowall[0].setPosition(0.0f, WALL_THICKNESS, (PLANE_DEPTH + WALL_THICKNESS) / 2);
	if (false == g_legowall[1].create(Device, PLANE_WIDTH, WALL_HEIGHT, WALL_THICKNESS, d3d::CYAN)) return false;
	g_legowall[1].setPosition(0.0f, WALL_THICKNESS, (-PLANE_DEPTH - WALL_THICKNESS) / 2);
	if (false == g_legowall[2].create(Device, WALL_THICKNESS, WALL_HEIGHT, PLANE_DEPTH + 2 * WALL_THICKNESS, d3d::DARKRED)) return false;
	g_legowall[2].setPosition((PLANE_WIDTH + WALL_THICKNESS) / 2, WALL_THICKNESS, 0.0f);
	if (false == g_legowall[3].create(Device, WALL_THICKNESS, WALL_HEIGHT, PLANE_DEPTH + 2 * WALL_THICKNESS, d3d::DARKRED)) return false;
	g_legowall[3].setPosition((-PLANE_WIDTH - WALL_THICKNESS) / 2, WALL_THICKNESS, 0.0f);

	if (false == g_stick.create(Device, D3DXCOLOR(1.0f, 0.8f, 0.25f, 1.0f))) return false;
	// create four balls and set the position
	for (i=0;i<4;i++) {
		if (false == g_sphere[i].create(Device, sphereColor[i])) return false;
		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS , spherePos[i][1]);
		g_sphere[i].setPower(0,0);
	}
	g_stick.setPosToward(g_sphere[3].getCenter().x, g_sphere[3].getCenter().y, g_sphere[3].getCenter().z, 1.0f, 0);
	
	// create blue ball for set direction
    if (false == g_target_blueball.create(Device, d3d::BLUE)) return false;
	g_target_blueball.setCenter(.0f, (float)M_RADIUS , .0f);

	g_sphere[3].setturncheck(1);

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
	g_stick.destroy();
    destroyAllLegoBlock();
    g_light.destroy();
}


bool ballAllStopped() {
	for (int i = 0; i < 4; i++) {
		if (g_sphere[i].getVelocity() != D3DXVECTOR3(0,0,0))
			return false;
	}
	return true;
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
			for(j = 0; j < 4; j++){
				g_legowall[i].hitBy(g_sphere[j]);
			}
		}
		if (g_sphere[3].getVelocity_X() == 0 && g_sphere[3].getVelocity_Y() == 0)
			g_stick.setVisible(true);
		else g_stick.setVisible(false);
		// check whether any two balls hit together and update the direction of balls
		for(i = 0 ;i < 4; i++){
			for(j = 0 ; j < 4; j++) {
				if(i >= j) {continue;}
				g_sphere[i].hitBy(g_sphere[j] /*TODO::, timeDelta*/);
			}
		}
		//D3DXVECTOR3 viewPoint = g_sphere[3].getCenter();
		//g_stick.setPosToward(viewPoint.x, viewPoint.z, 2.8f, g_stick.getDirection() + 0.7f * timeDelta);
		
		// draw plane, walls, and spheres
		g_legoPlane.draw(Device, g_mWorld);
		for (i=0;i<4;i++) 	{
			g_legowall[i].draw(Device, g_mWorld);
			g_sphere[i].draw(Device, g_mWorld);
		}
		g_stick.draw(Device, g_mWorld);
		g_target_blueball.draw(Device, g_mWorld);
		//g_light.setLight(Device, g_mWorld);
        g_light.draw(Device/*, g_mWorld*/);
		
		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture( 0, NULL );

		if (updateDone == false && ballAllStopped() == true)
		{
			updateDone = true;
			//만약 모든 공들이 멈췄을때 scount를 검사하여 경우의수를 나눈다.//노란공이 2번
			//g_sphere[2] 노란공 g_sphere[3]=흰공  
			if (g_sphere[1].getid() == 1 && g_sphere[0].getid() == 1)//나머지 두공 다맞았을때
			{
				if (g_sphere[3].getturncheck() == 1)//흰공턴이었을때
				{
					if (g_sphere[2].getid() == 1)//노란공이 맞았을때
					{
						g_sphere[3].setturncheck(0);
						g_sphere[2].setturncheck(1);
						for (i = 0; i < 4; i++) {
							g_sphere[i].setid(0);
						}
						//score--  상대턴
					}
					else  // 노란공이 안맞았을때
					{
						for (i = 0; i < 4; i++) {
							g_sphere[i].setid(0);
						}
					   //score++ 아직도 내턴 
					}
				
				}
				else if (g_sphere[2].getturncheck() == 1) //노란공턴이었을때 
				{
					if (g_sphere[3].getid() == 1)// 흰공이 맞았을때
					{
						g_sphere[2].setturncheck(0);
						g_sphere[3].setturncheck(1);
						for (i = 0; i < 4; i++) {
							g_sphere[i].setid(0);
						}
					//score -- 상대턴
					
					}
					else
					{
						for (i = 0; i < 4; i++) {
							g_sphere[i].setid(0);
						}
						//score++ 내턴
					}
				
				
				
				}
					
			
			}



			else if ((g_sphere[1].getid() == 1) != (g_sphere[0].getid() == 1)) // 나머지두공중 하나만 맞았을때
			{
				if (g_sphere[2].getturncheck() == 1)// 노란공턴이었을때
				{
					if (g_sphere[3].getid() == 1) // 흰공이 맞았을때
					{
						g_sphere[3].setturncheck(1);
						g_sphere[2].setturncheck(0);
						for (i = 0; i < 4; i++) {
							g_sphere[i].setid(0);
						}
						//score-- 턴넘어감
					}
					else
					{
						g_sphere[3].setturncheck(1);
						g_sphere[2].setturncheck(0);
						for (i = 0; i < 4; i++) {
							g_sphere[i].setid(0);
						}
						// 점수안깍이고 턴만넘어감
					}

				}
				else if (g_sphere[3].getturncheck() == 1)// 흰공턴이었을때
				{
					if (g_sphere[2].getid() == 1) // 노란공이 맞았을때
					{
						g_sphere[2].setturncheck(1);
						g_sphere[3].setturncheck(0);
						for (i = 0; i < 4; i++) {
							g_sphere[i].setid(0);
						}
						//score-- 턴넘어감
					}
					else//노란공이 안맞았을때
					{
						g_sphere[2].setturncheck(1);
						g_sphere[3].setturncheck(0);
						for (i = 0; i < 4; i++) {
							g_sphere[i].setid(0);
						}
						// 점수안깍이고 턴만넘어감
					}

				}
				
			
			}


			else//if (g_sphere[1].getid() == 0 && g_sphere[0].getid() == 0)
			{
				if (g_sphere[2].getturncheck() == 1)// 노란공턴이었을때
				{
					g_sphere[2].setturncheck(0);
					g_sphere[3].setturncheck(1);
					for (i = 0; i < 4; i++) {
						g_sphere[i].setid(0);
					}
					//score -- 턴넘어감

				}
				else if(g_sphere[3].getturncheck() == 1)// 흰공턴이었을때
				{
					g_sphere[3].setturncheck(0);
					g_sphere[2].setturncheck(1);
					for (i = 0; i < 4; i++) {
						g_sphere[i].setid(0);
					}
					//score -- 턴넘어감

				}
			
			
			}
			// g-sphere[1].id=1 이고 g_sphere[4].id=1 이고 g_sphere[2]와 g_sphere[3]중 turncheck가 0인공의 id가 0이면 +1 후 자신의턴
			// g-sphere[1].id와 g_sphere[4].id 값중 하나가 1이고 g_sphere[2]와 g_sphere[3]중 turncheck가 0인공의 id가 0이면 +0 후 상대턴
			// 위의 경우가 둘다 아닐경우 -1 하고 상대턴

			

		};
	

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
	static float power = 1.0f;
	
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
			if (ballAllStopped() == false)
				break;
			updateDone = false;
			D3DXVECTOR3 targetpos = g_target_blueball.getCenter();
			// 누구의공인지 체크한걸 가져온다 . 흰공일경우 g_sphere[3]이고 아닐경우 g_sphere[2] 노란공..




			D3DXVECTOR3	whitepos = g_sphere[3].getCenter();

			if (g_sphere[2].getturncheck() == 1)
			{
					whitepos = g_sphere[2].getCenter();
			}


			double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
				pow(targetpos.z - whitepos.z, 2)));		// 기본 1 사분면
			if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0) { theta = -theta; }	//4 사분면
			if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0) { theta = PI - theta; } //2 사분면
			if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0) { theta = PI + theta; } // 3 사분면
			double distance = sqrt(pow(targetpos.x - whitepos.x, 2) + pow(targetpos.z - whitepos.z, 2));
			if (g_sphere[2].getturncheck() == 0)
			 {
			   g_sphere[3].setPower(distance * cos(theta), distance * sin(theta));
		     }
			if (g_sphere[2].getturncheck() == 1)
				{
					g_sphere[2].setPower(distance* cos(theta), distance* sin(theta));
				}
					
				
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
					dx = (float)(old_x - new_x);// * 0.01f;
					dy = (float)(old_y - new_y);// * 0.01f;
		
					D3DXVECTOR3 coord3d=g_target_blueball.getCenter();
					D3DXVECTOR3 coord3d2 = g_sphere[3].getCenter();
					g_target_blueball.setCenter(coord3d.x+dx*(-0.007f),coord3d.y,coord3d.z+dy*0.007f );
					power += dy * (-0.007f);
					if (power < M_RADIUS + 0.1f) power = M_RADIUS + 0.1f;
					g_stick.setPosToward(coord3d2.x, coord3d2.y, coord3d2.z, power, g_stick.getDirection() + dx * 0.007f);
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