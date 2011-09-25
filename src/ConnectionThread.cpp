/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "ConnectionThread.h"
#include "IceApp.h"
#include <vehicleadmin.h>
#include <SDL.h>


ConnectionThread::ConnectionThread(IceApp *a, 
                                   bool *shutdown_flag,
                                   const std::string &unitproxy,
                                   vehicle::SensorFrameReceiverPrx &sc)
  : app(a),
    shutdown_requested(shutdown_flag),
    unit_str_proxy(unitproxy),
    sensor_callback(sc)
{
}


ConnectionThread::~ConnectionThread()
{
}


void 
ConnectionThread::run()
{
  vehicle::RemoteVehiclePrx unit_prx;

  while(!*(this->shutdown_requested)
        && !this->app->communicator()->isShutdown())
    {
      try
        {
          this->app->printMessage("Connecting to the remote unit");

          Ice::ObjectPrx objprx = 
            this->app->communicator()->stringToProxy(unit_str_proxy);
          unit_prx = vehicle::RemoteVehiclePrx::checkedCast(objprx);

          if(!unit_prx)
            {
              this->app->printMessage("Can not connect to remote unit. Retrying...");
              SDL_Delay(2 * 1000);
              continue;
            }
          else
            {
              std::string statusmsg = "Connected to ";
              statusmsg += unit_prx->getVehicleDescription();
              this->app->printMessage(statusmsg);
              unit_prx->setSensorReceiver(this->sensor_callback);

              vehicleadmin::SensorList sensor_list = unit_prx->getSensorList();
              this->app->setSensorList(sensor_list);

              vehicleadmin::ActuatorList actuator_list = 
                unit_prx->getActuatorList();
              this->app->setActuatorList(actuator_list);
              break;
            }

        }
      catch(const Ice::Exception& ex)
        {
          std::ostringstream os;
          os << "Can not reach remote unit: ";
          ex.ice_print(os);
          os << ". Retrying...";
          this->app->printMessage(os.str());
        }
      SDL_Delay(2 * 1000);
    }

  this->app->setControlProxies(unit_prx);
}
