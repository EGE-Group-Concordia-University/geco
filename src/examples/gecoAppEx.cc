#include <geco.h>
#include "gecoObjEx.h"

using namespace std;

int main(int argc, char **argv)
{
  // creates a new gecoApp
  gecoApp* app = new gecoApp(argc, argv);
  
  // creates a new gecoObj and adds it to the gecoApp
  gecoObjEx* newCmd = new gecoObjEx(app);
  
  // runs the app in the CLI mode
  app->runCLI();
  
  // cleans up everything
  delete newCmd;
  delete app;
  
  return 0;
}