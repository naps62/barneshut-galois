#ifndef ___F_BODY_BLOCKS_BUILD_H___
#define ___F_BODY_BLOCKS_BUILD_H___

namespace Barneshut {

/**
 * Computes the position of a body after a time step.
 *
 * TODO: optimize this to reutilize memory
 */
struct BodyBlocksBuild {
  // Optimize runtime for no conflict case
  typedef int tt_does_not_need_aborts;

  BodyBlocks* blocks;
  uint bsize;

  BodyBlocksBuild(BodyBlocks* _blocks, int _bsize) :
    blocks(_blocks),
    bsize(_bsize) {
    blocks->clear();
    blocks->push_back(new BodiesPtr());
  }

  template<typename Context>
  void operator()(Body* bb, Context&) {
    BodiesPtr* currentBlock = blocks->back();

    // if block size is maxed out, create 
    if (currentBlock->size() == bsize) {
      currentBlock = new BodiesPtr();
      blocks->push_back(currentBlock);
    }

    // add current body to current block
    currentBlock->push_back(bb);
  }
};

}

#endif//___F_BODY_BLOCKS_BUILD_H___
