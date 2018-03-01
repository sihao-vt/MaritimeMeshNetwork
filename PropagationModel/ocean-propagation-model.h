#ifndef OCEAN_PROPAGATION_MODEL_H
#define OCEAN_PROPAGATION_MODEL_H

#include"ns3/object.h"
#include"ns3/nstime.h"
#include"propagation-loss-model.h"
#include<vector>

namespace ns3{


class OceanPropagationModel : public PropagationLossModel
{
public:
  static TypeId GetTypeId(void);
  OceanPropagationModel();

  void SetFreq(double freq);
  double GetFreq() const;
  double GetWindspeed() const;
  void SetWindspeed(double windspeed);

private:
  double m_lambda;
  double m_freq;
  uint32_t m_patchSize;
  double m_windSpeed;
  std::string m_filename;
  double m_timeStep;
  uint64_t m_meshSize;
  uint32_t m_steps;

  OceanPropagationModel(const OceanPropagationModel &);
  OceanPropagationModel & operator = (const OceanPropagationModel & );
  virtual double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel>a,Ptr<MobilityModel>b) const;
  virtual int64_t DoAssignStreams(int64_t stream);
  double DbmToW(double dbm) const;
  double DbmFromW(double w)const;
  bool checkBlock(double distance, double txHeight, double rxHeight, Time simulation_time, double xlength, double ylength, double refangle)const;
  double compProb(double x)const;
  std::vector<std::vector<double>> path;
  void InitPath();
};
}
#endif



