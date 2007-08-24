// AcousticTube.h: interface for the CAcousticTube class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACOUSTICTUBE_H__B7D28925_F9CC_4EBA_ABF3_5A2C8D9F9CF9__INCLUDED_)
#define AFX_ACOUSTICTUBE_H__B7D28925_F9CC_4EBA_ABF3_5A2C8D9F9CF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAcousticTube  
{
public:
	CAcousticTube(const std::vector<double> &dDiameters, const std::vector<double> &dReflections);
	virtual ~CAcousticTube();

	void SetTransform(const std::vector<double> &dDiameters, const std::vector<double> &dReflection);
	void GetTransform(std::vector<double> *dDiameters, std::vector<double> *dReflection);

  double Tick(double dInput, bool bSourceReflect=true, bool bLattice = false);

private:
  int32 m_nOrder;
  std::vector<double> m_State;
  std::vector<double> m_Diameters;
  std::vector<double> m_Reflection;
  std::vector<double> m_RCoefficientsFromDiameters;
};

#endif // !defined(AFX_ACOUSTICTUBE_H__B7D28925_F9CC_4EBA_ABF3_5A2C8D9F9CF9__INCLUDED_)
