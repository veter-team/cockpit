#include <stdexcept>
#include "IceApp.h"
#include "PrintStatusMsg.h"


int 
main(int argc, char *argv[])
{
  int res = 0;

  try
    {
      IceApp app;
      res = app.main(argc, argv);
    }
  catch(const std::exception &ex)
    {
      printStatusMessage1(ex.what());
      res = 101;
    }
  return res;
}
