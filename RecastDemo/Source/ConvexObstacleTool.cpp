#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include "SDL.h"
#include "SDL_opengl.h"
#include "imgui.h"
#include "Recast.h"
#include "Sample_TempObstacles.h"
#include "RecastDebugDraw.h"
#include "DetourDebugDraw.h"
#include "ConvexVolumeTool.h"
#include "ConvexObstacleTool.h"

ConvexObstacleTool::ConvexObstacleTool() :
	m_sample(0),
	m_polyOffset(0.0f),
	m_boxHeight(6.0f),
	m_boxDescent(1.0f),
	m_npts(0),
	m_nhull(0)
{
}

void ConvexObstacleTool::init(Sample* sample)
{
	m_sample = (Sample_TempObstacles*)sample;
}

void ConvexObstacleTool::reset()
{
	m_npts = 0;
	m_nhull = 0;
}

void ConvexObstacleTool::handleMenu()
{
	imguiSlider("Shape Height", &m_boxHeight, 0.1f, 20.0f, 0.1f);
	imguiSlider("Shape Descent", &m_boxDescent, 0.1f, 20.0f, 0.1f);
	imguiSlider("Poly Offset", &m_polyOffset, 0.0f, 10.0f, 0.1f);

	imguiSeparator();

	if (imguiButton("Clear Shape"))
	{
		m_npts = 0;
		m_nhull = 0;
	}

	if (imguiButton("Remove All"))
	{
		m_npts = 0;
		m_nhull = 0;
		m_sample->clearAllTempObstacles();
	}
}

void ConvexObstacleTool::handleClick(const float* s, const float* p, bool shift)
{
	if (!m_sample) return;
	
	if (shift)
	{
		// Delete
		m_sample->removeTempObstacle(s, p);
	}
	else
	{
		// Create

		// If clicked on that last pt, create the shape.
		if (m_npts && rcVdistSqr(p, &m_pts[(m_npts-1)*3]) < rcSqr(0.2f))
		{
			if (m_nhull > 2)
			{
				// Create shape.
				float verts[MAX_PTS*3];
				for (int i = 0; i < m_nhull; ++i)
					rcVcopy(&verts[i*3], &m_pts[m_hull[i]*3]);
					
				float minh = FLT_MAX, maxh = 0;
				for (int i = 0; i < m_nhull; ++i)
					minh = rcMin(minh, verts[i*3+1]);
				minh -= m_boxDescent;
				maxh = minh + m_boxHeight;

				if (m_polyOffset > 0.01f)
				{
					float offset[MAX_PTS*2*3];
					int noffset = rcOffsetPoly(verts, m_nhull, m_polyOffset, offset, MAX_PTS*2);
					m_sample->addTempObstacleConvex(offset, noffset, minh, maxh);
				}
				else
				{
					m_sample->addTempObstacleConvex(verts, m_nhull, minh, maxh);
				}
			}
			
			m_npts = 0;
			m_nhull = 0;
		}
		else
		{
			// Add new point 
			if (m_npts < MAX_PTS)
			{
				rcVcopy(&m_pts[m_npts*3], p);
				m_npts++;
				// Update hull.
				if (m_npts > 1)
					m_nhull = convexhull(m_pts, m_npts, m_hull);
				else
					m_nhull = 0;
			}
		}		
	}
	
}

void ConvexObstacleTool::handleToggle()
{
}

void ConvexObstacleTool::handleStep()
{
}

void ConvexObstacleTool::handleUpdate(const float /*dt*/)
{
}

void ConvexObstacleTool::handleRender()
{
	duDebugDraw& dd = m_sample->getDebugDraw();
	
	// Find height extent of the shape.
	float minh = FLT_MAX, maxh = 0;
	for (int i = 0; i < m_npts; ++i)
		minh = rcMin(minh, m_pts[i*3+1]);
	minh -= m_boxDescent;
	maxh = minh + m_boxHeight;

	dd.begin(DU_DRAW_POINTS, 4.0f);
	for (int i = 0; i < m_npts; ++i)
	{
		unsigned int col = duRGBA(255,255,255,255);
		if (i == m_npts-1)
			col = duRGBA(240,32,16,255);
		dd.vertex(m_pts[i*3+0],m_pts[i*3+1]+0.1f,m_pts[i*3+2], col);
	}
	dd.end();

	dd.begin(DU_DRAW_LINES, 2.0f);
	for (int i = 0, j = m_nhull-1; i < m_nhull; j = i++)
	{
		const float* vi = &m_pts[m_hull[j]*3];
		const float* vj = &m_pts[m_hull[i]*3];
		dd.vertex(vj[0],minh,vj[2], duRGBA(255,255,255,64));
		dd.vertex(vi[0],minh,vi[2], duRGBA(255,255,255,64));
		dd.vertex(vj[0],maxh,vj[2], duRGBA(255,255,255,64));
		dd.vertex(vi[0],maxh,vi[2], duRGBA(255,255,255,64));
		dd.vertex(vj[0],minh,vj[2], duRGBA(255,255,255,64));
		dd.vertex(vj[0],maxh,vj[2], duRGBA(255,255,255,64));
	}
	dd.end();	
}

void ConvexObstacleTool::handleRenderOverlay(double* /*proj*/, double* /*model*/, int* view)
{
	// Tool help
	const int h = view[3];
	if (!m_npts)
	{
		imguiDrawText(280, h-40, IMGUI_ALIGN_LEFT, "LMB: Create new shape.  SHIFT+LMB: Delete existing shape (click inside a shape).", imguiRGBA(255,255,255,192));	
	}
	else
	{
		imguiDrawText(280, h-40, IMGUI_ALIGN_LEFT, "Click LMB to add new points. Click on the red point to finish the shape.", imguiRGBA(255,255,255,192));	
		imguiDrawText(280, h-60, IMGUI_ALIGN_LEFT, "The shape will be convex hull of all added points.", imguiRGBA(255,255,255,192));	
	}
	
}
