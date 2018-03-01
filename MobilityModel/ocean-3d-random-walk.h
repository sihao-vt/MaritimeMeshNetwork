#ifndef OCEAN_3D_RANDOM_WALK_H
#define OCEAN_3D_RANDOM_WALK_H

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/rectangle.h"
#include "ns3/random-variable-stream.h"
#include "mobility-model.h"
#include "constant-velocity-helper.h"
#include <string>

namespace ns3 {
class Ocean3dRandomWalk : public MobilityModel 
{
public:
  static TypeId GetTypeId (void);
private:
  void Rebound (Time timeLeft);
  void DoWalk (Time timeLeft);
  void DoInitializePrivate (void);
  virtual void DoDispose (void);
  virtual void DoInitialize (void);
  virtual Vector DoGetPosition (void) const;
  virtual void DoSetPosition (const Vector &position);
  virtual Vector DoGetVelocity (void) const;
  virtual int64_t DoAssignStreams (int64_t);
  
  uint64_t m_meshsize;
  uint64_t m_patchsize;
  double m_windspeed;
  double m_timestep;
  uint64_t m_steps;
  std::string m_filename;
  std::vector<std::vector<double>> path;
  double GetHeight(Time simulation_time,double xlength, double ylength);
  void InitPath(void);

  ConstantVelocityHelper m_helper; //!< helper for this object
  EventId m_event; //!< stored event ID 
  Time m_time; //!< Change current direction and speed after this delay
  Ptr<RandomVariableStream> m_speed; //!< rv for picking speed
  Ptr<RandomVariableStream> m_direction; //!< rv for picking direction
  Rectangle m_bounds; //!< Bounds of the area to cruise
};


} // namespace ns3

#endif
