/** Loop Entry/Exit Hooks -*- C++ -*-
 * @file
 * @section License
 *
 * Galois, a framework to exploit amorphous data-parallelism in irregular
 * programs.
 *
 * Copyright (C) 2011, The University of Texas at Austin. All rights reserved.
 * UNIVERSITY EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES CONCERNING THIS
 * SOFTWARE AND DOCUMENTATION, INCLUDING ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR ANY PARTICULAR PURPOSE, NON-INFRINGEMENT AND WARRANTIES OF
 * PERFORMANCE, AND ANY WARRANTY THAT MIGHT OTHERWISE ARISE FROM COURSE OF
 * DEALING OR USAGE OF TRADE.  NO WARRANTY IS EITHER EXPRESS OR IMPLIED WITH
 * RESPECT TO THE USE OF THE SOFTWARE OR DOCUMENTATION. Under no circumstances
 * shall University be liable for incidental, special, indirect, direct or
 * consequential damages or loss of profits, interruption of business, or
 * related expenses which may arise from use of Software or Documentation,
 * including but not limited to those resulting from defects in Software and/or
 * Documentation, or loss or inaccuracy of data of any kind.
 *
 * @author Andrew Lenharth <andrewl@lenharth.org>
 */

#ifndef GALOIS_RUNTIME_LOOPHOOKS_H
#define GALOIS_RUNTIME_LOOPHOOKS_H

#include "Galois/Runtime/ll/CompilerSpecific.h"

namespace GaloisRuntime {

void runAllLoopExitHandlers(void);

//Base notification class for parallel loop exits
//Implement the LoopExit function
class AtLoopExit {
  friend void runAllLoopExitHandlers(void);
  virtual void LoopExit(void) = 0;
protected:
  AtLoopExit* next;
public:
  GALOIS_ATTRIBUTE_DEPRECATED
  AtLoopExit();
  ~AtLoopExit();
};

}

#endif //GALOIS_RUNTIME_LOOPHOOKS_H

