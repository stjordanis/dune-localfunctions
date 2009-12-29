// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_REFINED_P0_LOCALINTERPOLATION_HH
#define DUNE_REFINED_P0_LOCALINTERPOLATION_HH

#include "../common/localinterpolation.hh"

namespace Dune
{
  template<class LB>
  class RefinedP0LocalInterpolation
#ifdef DUNE_VIRTUAL_SHAPEFUNCTIONS
    : public LocalInterpolationInterface<typename LB::Traits::DomainType, typename LB::Traits::RangeType>
#endif
  {};

  template<class D, class R>
  class RefinedP0LocalInterpolation<RefinedP0LocalBasis<D,R,2> >
#ifdef DUNE_VIRTUAL_SHAPEFUNCTIONS
    : public LocalInterpolationInterface<typename RefinedP0LocalBasis<D,R,2>::Traits::DomainType, typename RefinedP0LocalBasis<D,R,2>::Traits::RangeType>
#endif
  {
    typedef RefinedP0LocalBasis<D,R,2> LB;
    typedef typename LB::Traits::DomainType DT;

  public:
    RefinedP0LocalInterpolation() :
      interpolationPoints_(4)
    {
      // Interpolation is done by evaluating at the subtriangle centers
      interpolationPoints_[0][0] = 1.0/6;
      interpolationPoints_[0][1] = 1.0/6;

      interpolationPoints_[1][0] = 4.0/6;
      interpolationPoints_[1][1] = 1.0/6;

      interpolationPoints_[2][0] = 1.0/6;
      interpolationPoints_[2][1] = 4.0/6;

      interpolationPoints_[3][0] = 2.0/6;
      interpolationPoints_[3][1] = 2.0/6;
    }


    template<typename F, typename C>
    void interpolate (const F& f, std::vector<C>& out) const
    {
      typename LB::Traits::RangeType y;
      out.resize(4);
      for (int i = 0; i < 4; ++i)
      {
        f.evaluate(interpolationPoints_[i], y);
        out[i] = y;
      }
    }

#if DUNE_VIRTUAL_SHAPEFUNCTIONS
    typedef LocalInterpolationInterface<typename LB::Traits::DomainType, typename LB::Traits::RangeType> Base;

    void interpolate(const typename Base::FunctionType& f, typename std::vector<typename Base::CoefficientType>& out) const
    {
      interpolate<typename Base::FunctionType, typename Base::CoefficientType>(f, out);
    }
#endif

  private:
    std::vector<DT> interpolationPoints_;
  };
}

#endif