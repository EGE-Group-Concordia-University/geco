#ifndef gecoObjEx_SEEN_
#define gecoObjEx_SEEN_
#include <tcl.h>
#include <geco.h>

using namespace std;

// -----------------------------------------------------------------------
//
// class gecoObjEx : Example for creating a new gecoObj
//

class gecoObjEx : public virtual gecoObj
{

private:
  double       x;
  
public:
  gecoObjEx(gecoApp* App);
  ~gecoObjEx();
  virtual int cmd(int &i,int objc,Tcl_Obj *const objv[]);
  virtual Tcl_DString* info(const char* frontStr = "");
};

#endif /* gecoObjEx_SEEN_ */