// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LAGRANGECOEFFICIENTS_HH
#define DUNE_LAGRANGECOEFFICIENTS_HH

#include <vector>

#include <dune/common/fvector.hh>
#include <dune/finiteelements/common/field.hh>
#include <dune/common/forloop.hh>
#include <dune/finiteelements/generic/topologyfactory.hh>

#include <dune/grid/genericgeometry/topologytypes.hh>
#include <dune/grid/genericgeometry/subtopologies.hh>

#include <dune/finiteelements/common/localcoefficients.hh>

namespace Dune
{

  template< template <class,unsigned int> class LP, unsigned int dim, class F >
  class LagrangeCoefficientsFactory;

  template< template <class,unsigned int> class LP,
      unsigned int dim, class F>
  struct LagrangeCoefficientsFactoryTraits
  {
    static const unsigned int dimension = dim;
    const typedef LP<F,dim> Object;
    typedef unsigned int Key;
    typedef LagrangeCoefficientsFactory< LP,dim,F > Factory;
  };

  template< template <class,unsigned int> class LP,
      unsigned int dim, class F>
  struct LagrangeCoefficientsFactory :
    public TopologyFactory< LagrangeCoefficientsFactoryTraits< LP,dim,F> >
  {
    typedef LagrangeCoefficientsFactoryTraits<LP,dim,F> Traits;
    static const unsigned int dimension = dim;
    typedef typename Traits::Object Object;
    typedef typename Traits::Key Key;

    template< class T >
    static Object *createObject ( const Key &order )
    {
      if (!Object::template supports<T>(order))
        return 0;
      typedef typename remove_const<Object>::type LagrangeCoefficients;
      LagrangeCoefficients *object = new LagrangeCoefficients(order);
      if ( !object->template build<T>() )
      {
        delete object;
        object = 0;
      }
      return object;
    }
  };

}

#endif // DUNE_LAGRANGECOEFFICIENTS_HH
