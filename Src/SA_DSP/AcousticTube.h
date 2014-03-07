// AcousticTube.h: interface for the CAcousticTube class.
//
//////////////////////////////////////////////////////////////////////
#ifndef ACOUSTICTUBE_H
#define ACOUSTICTUBE_H

class CAcousticTube
{
public:
    CAcousticTube(const std::vector<double> & dDiameters, const std::vector<double> & dReflections);
    virtual ~CAcousticTube();

    void SetTransform(const std::vector<double> & dDiameters, const std::vector<double> & dReflection);
    void GetTransform(std::vector<double> * dDiameters, std::vector<double> * dReflection);

    double Tick(double dInput, bool bSourceReflect=true, bool bLattice = false);

private:
    int32 m_nOrder;
    std::vector<double> m_State;
    std::vector<double> m_Diameters;
    std::vector<double> m_Reflection;
    std::vector<double> m_RCoefficientsFromDiameters;
};

#endif
