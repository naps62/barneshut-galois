struct AdvanceBodies {
  // Optimize runtime for no conflict case
  typedef int tt_does_not_need_context;

  AdvanceBodies() { }

  template<typename Context>
  void operator()(Body* bb, Context&) {
    Body& b = *bb;
    Point dvel(b.acc);
    dvel *= config.dthf;

    Point velh(b.vel);
    velh += dvel;

    for (int i = 0; i < 3; ++i)
      b.pos[i] += velh[i] * config.dtime;
    for (int i = 0; i < 3; ++i)
      b.vel[i] = velh[i] + dvel[i];
  }
};