#ifndef OCEAN_MOBILITY_MODEL_H
#define OCEAN_MOBILITY_MODEL_H

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "mobility-model.h"
#include <string>

namespace ns3 {
class OceanMobilityModel : public MobilityModel 
{
public:
  static TypeId GetTypeId (void);
  OceanMobilityModel();
  virtual ~OceanMobilityModel (); 
  double GetPredictedHeight(Time t);
  double GetHeight(void);
private:
  virtual void DoDispose (void);
  virtual void DoInitialize (void);
  virtual Vector DoGetPosition (void) const;
  virtual void DoSetPosition (const Vector &position);
  virtual Vector DoGetVelocity (void) const; 
  uint64_t m_meshsize;
  uint64_t m_patchsize;
  double m_timestep;
  uint64_t m_steps;
  std::string m_filename;
  std::vector<double> m_path;

  void InitPath(void);

  Vector m_position;
};


} // namespace ns3

#endif
