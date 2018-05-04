#ifndef OCEAN_3D_RANDOM_WALK_H
#define OCEAN_3D_RANDOM_WALK_H

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "mobility-model.h"
#include <string>

namespace ns3 {
class Ocean3dRandomWalk : public MobilityModel 
{
public:
  static TypeId GetTypeId (void);
  Ocean3dRandomWalk();
  virtual ~Ocean3dRandomWalk (); 
  double GetPredictedHeight(Time t) const;
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
