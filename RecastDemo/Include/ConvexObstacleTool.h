#pragma once
#include "Sample.h"
#include "DetourTileCache.h"

class Sample_TempObstacles;
class ConvexObstacleTool : public SampleTool
{
	Sample_TempObstacles* m_sample;
	float m_polyOffset;
	float m_boxHeight;
	float m_boxDescent;
	
	static const int MAX_PTS = DT_OBSTACLE_CONVEX_MAX_PT;
	float m_pts[MAX_PTS*3];
	int m_npts;
	int m_hull[MAX_PTS];
	int m_nhull;
	
public:
	ConvexObstacleTool();
	
	virtual int type() { return TOOL_CONVEX_OBSTACLE; }
	virtual void init(Sample* sample);
	virtual void reset();
	virtual void handleMenu();
	virtual void handleClick(const float* s, const float* p, bool shift);
	virtual void handleToggle();
	virtual void handleStep();
	virtual void handleUpdate(const float dt);
	virtual void handleRender();
	virtual void handleRenderOverlay(double* proj, double* model, int* view);
};
