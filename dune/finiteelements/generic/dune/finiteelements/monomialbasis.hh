// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MONOMIALBASIS_HH
#define DUNE_MONOMIALBASIS_HH

#include <vector>

#include <dune/common/fvector.hh>

#include <dune/grid/genericgeometry/topologytypes.hh>
#include <dune/grid/genericgeometry/misc.hh>

#include <dune/common/field.hh>

#include <dune/finiteelements/basisprovider.hh>

namespace Dune
{

  // Internal Forward Declarations
  // -----------------------------

  template< class Topology, class F >
  class MonomialBasis;



  // MonomialBasisImpl
  // -----------------

  template< class Topology, class F >
  class MonomialBasisImpl;

  template< class F >
  class MonomialBasisImpl< GenericGeometry::Point, F >
  {
    typedef MonomialBasisImpl< GenericGeometry::Point, F > This;

  public:
    typedef GenericGeometry::Point Topology;
    typedef F Field;

    static const unsigned int dimDomain = Topology::dimension;

    typedef FieldVector< Field, dimDomain > DomainVector;

  private:
    friend class MonomialBasis< Topology, Field >;
    friend class MonomialBasisImpl< GenericGeometry::Prism< Topology >, Field >;
    friend class MonomialBasisImpl< GenericGeometry::Pyramid< Topology >, Field >;

    mutable unsigned int maxOrder_;
    // sizes_[ k ]: number of basis functions of exactly order k
    mutable unsigned int *sizes_;
    // numBaseFunctions_[ k ] = sizes_[ 0 ] + ... + sizes_[ k ]
    mutable unsigned int *numBaseFunctions_;

    MonomialBasisImpl ()
      : sizes_( 0 ),
        numBaseFunctions_( 0 )
    {
      computeSizes( 2 );
    }

    ~MonomialBasisImpl ()
    {
      delete[] sizes_;
      delete[] numBaseFunctions_;
    }

    template< int dimD >
    void evaluate ( const unsigned int order,
                    const FieldVector< Field, dimD > &x,
                    const unsigned int *const offsets,
                    Field *const values ) const
    {
      values[ 0 ] = Unity< Field >();
    }

    void integral ( const unsigned int order,
                    const unsigned int *const offsets,
                    Field *const values ) const
    {
      values[ 0 ] = Unity< Field >();
    }

    unsigned int maxOrder () const
    {
      return maxOrder_;
    }

    void computeSizes ( unsigned int order ) const
    {
      maxOrder_ = order;

      delete [] sizes_;
      delete [] numBaseFunctions_;
      sizes_            = new unsigned int [ order+1 ];
      numBaseFunctions_ = new unsigned int [ order+1 ];

      sizes_[ 0 ] = 1;
      numBaseFunctions_[ 0 ] = 1;
      for( unsigned int k = 1; k <= order; ++k )
      {
        sizes_[ k ]            = 0;
        numBaseFunctions_[ k ] = 1;
      }
    }
  };

  template< class BaseTopology, class F >
  class MonomialBasisImpl< GenericGeometry::Prism< BaseTopology >, F >
  {
    typedef MonomialBasisImpl< GenericGeometry::Prism< BaseTopology >, F > This;

  public:
    typedef GenericGeometry::Prism< BaseTopology > Topology;
    typedef F Field;

    static const unsigned int dimDomain = Topology::dimension;

    typedef FieldVector< Field, dimDomain > DomainVector;

  private:
    friend class MonomialBasis< Topology, Field >;
    friend class MonomialBasisImpl< GenericGeometry::Prism< Topology >, Field >;
    friend class MonomialBasisImpl< GenericGeometry::Pyramid< Topology >, Field >;

    MonomialBasisImpl< BaseTopology, Field > baseBasis_;
    // sizes_[ k ]: number of basis functions of exactly order k
    mutable unsigned int *sizes_;
    // numBaseFunctions_[ k ] = sizes_[ 0 ] + ... + sizes_[ k ]
    mutable unsigned int *numBaseFunctions_;

    MonomialBasisImpl ()
      : sizes_( 0 ),
        numBaseFunctions_( 0 )
    {
      computeSizes( 2 );
    }

    ~MonomialBasisImpl ()
    {
      delete[] sizes_;
      delete[] numBaseFunctions_;
    }

    template< int dimD >
    void evaluate ( const unsigned int order,
                    const FieldVector< Field, dimD > &x,
                    const unsigned int *const offsets,
                    Field *const values ) const
    {
      const Field &z = x[ dimDomain-1 ];

      // fill first column
      baseBasis_.evaluate( order, x, offsets, values );
      const unsigned int *const baseSizes = baseBasis_.sizes_;

      Field *row0 = values;
      for( unsigned int k = 1; k <= order; ++k )
      {
        Field *const row1begin = values + offsets[ k-1 ];
        Field *const colkEnd = row1begin + (k+1)*baseSizes[ k ];
        assert( (unsigned int)(colkEnd - values) <= offsets[ k ] );
        Field *const row1End = row1begin + sizes_[ k ];
        assert( (unsigned int)(row1End - values) <= offsets[ k ] );

        Field *row1 = row1begin;
        Field *it;
        for( it = row1begin + baseSizes[ k ]; it != colkEnd; ++row1, ++it )
          *it = z * (*row1);
        for( ; it != row1End; ++row0, ++it )
          *it = z * (*row0);
        row0 = row1;
      }
    }

    void integral ( const unsigned int order,
                    const unsigned int *const offsets,
                    Field *const values ) const
    {
      // fill first column
      baseBasis_.integral( order, offsets, values );
      const unsigned int *const baseSizes = baseBasis_.sizes_;

      Field *row0 = values;
      for( unsigned int k = 1; k <= order; ++k )
      {
        Field *const row1begin = values + offsets[ k-1 ];
        Field *const row1End = row1begin + sizes_[ k ];
        assert( (unsigned int)(row1End - values) <= offsets[ k ] );

        Field *row1 = row1begin;
        Field *it = row1begin + baseSizes[ k ];
        for( unsigned int j = 1; j <= k; ++j )
        {
          Field *const end = it + baseSizes[ k ];
          assert( (unsigned int)(end - values) <= offsets[ k ] );
          for( ; it != end; ++row1, ++it )
            *it = (Field( j ) / Field( j+1 )) * (*row1);
        }
        for( ; it != row1End; ++row0, ++it )
          *it = (Field( k ) / Field( k+1 )) * (*row0);
        row0 = row1;
      }
    }

    unsigned int maxOrder() const
    {
      return baseBasis_.maxOrder();
    }

    void computeSizes ( unsigned int order ) const
    {
      delete[] sizes_;
      delete[] numBaseFunctions_;
      sizes_            = new unsigned int[ order+1 ];
      numBaseFunctions_ = new unsigned int[ order+1 ];

      baseBasis_.computeSizes( order );
      const unsigned int *const baseSizes = baseBasis_.sizes_;
      const unsigned int *const baseNBF   = baseBasis_.numBaseFunctions_;

      sizes_[ 0 ] = 1;
      numBaseFunctions_[ 0 ] = 1;
      for( unsigned int k = 1; k <= order; ++k )
      {
        sizes_[ k ]            = baseNBF[ k ] + k*baseSizes[ k ];
        numBaseFunctions_[ k ] = numBaseFunctions_[ k-1 ] + sizes_[ k ];
      }
    }
  };

  template< class BaseTopology, class F >
  class MonomialBasisImpl< GenericGeometry::Pyramid< BaseTopology >, F >
  {
    typedef MonomialBasisImpl< GenericGeometry::Pyramid< BaseTopology >, F > This;

  public:
    typedef GenericGeometry::Pyramid< BaseTopology > Topology;
    typedef F Field;

    static const unsigned int dimDomain = Topology::dimension;

    typedef FieldVector< Field, dimDomain > DomainVector;

  private:
    friend class MonomialBasis< Topology, Field >;
    friend class MonomialBasisImpl< GenericGeometry::Prism< Topology >, Field >;
    friend class MonomialBasisImpl< GenericGeometry::Pyramid< Topology >, Field >;

    MonomialBasisImpl< BaseTopology, Field > baseBasis_;
    // sizes_[ k ]: number of basis functions of exactly order k
    mutable unsigned int *sizes_;
    // numBaseFunctions_[ k ] = sizes_[ 0 ] + ... + sizes_[ k ]
    mutable unsigned int *numBaseFunctions_;

    MonomialBasisImpl ()
      : sizes_( 0 ),
        numBaseFunctions_( 0 )
    {
      computeSizes( 2 );
    }

    ~MonomialBasisImpl ()
    {
      delete[] sizes_;
      delete[] numBaseFunctions_;
    }

    template< int dimD >
    void evaluateSimplex ( const unsigned int order,
                           const FieldVector< Field, dimD > &x,
                           const unsigned int *const offsets,
                           Field *const values ) const
    {
      const Field &z = x[ dimDomain-1 ];

      // fill first column
      baseBasis_.evaluate( order, x, offsets, values );

      const unsigned int *const baseSizes = baseBasis_.sizes_;
      Field *row0 = values;
      for( unsigned int k = 1; k <= order; ++k )
      {
        Field *const row1 = values+offsets[ k-1 ];
        Field *const row1End = row1+sizes_[ k ];
        assert( (unsigned int)(row1End - values) <= offsets[ k ] );
        for( Field *it = row1 + baseSizes[ k ]; it != row1End; ++row0, ++it )
          *it = z * (*row0);
        row0 = row1;
      }
    }

    template< int dimD >
    void evaluatePyramid ( const unsigned int order,
                           const FieldVector< Field, dimD > &x,
                           const unsigned int *const offsets,
                           Field *const values ) const
    {
      const Field &z = x[ dimDomain-1 ];
      Field omz = Unity< Field >() - z;

      if( Zero<Field>() < omz )
      {
        const Field invomz = Unity<Field>() / omz;
        FieldVector< Field, dimDomain-1 > y;
        for( unsigned int i = 0; i < dimDomain-1; ++i )
          y[ i ] = x[ i ] * invomz;

        // fill first column
        baseBasis_.evaluate( order, y, offsets, values );

        const unsigned int *const baseSizes = baseBasis_.sizes_;
        Field *row0 = values;
        Field omzk = omz;
        for( unsigned int k = 1; k <= order; ++k )
        {
          Field *const row1 = values + offsets[ k-1 ];
          Field *const row1End = row1 + sizes_[ k ];
          assert( (unsigned int)(row1End - values) <= offsets[ k ] );
          Field *const col0End = row1 + baseSizes[ k ];
          Field *it = row1;
          for( ; it != col0End; ++it )
            *it = (*it) * omzk;
          for( ; it != row1End; ++row0, ++it )
            *it = z * (*row0);
          row0 = row1;
          omzk *= omz;
        }
      }
      else {
        Field *it = values;
        for( unsigned int k = 0; k <= order; ++k )
        {
          Field *const rowEnd = it + (sizes_[ k ] - 1);
          for( ; it != rowEnd; ++it )
            *it = Zero<Field>();
          *it = Unity<Field>();
          ++it;
        }
      }

    }

    template< int dimD >
    void evaluate ( const unsigned int order,
                    const FieldVector< Field, dimD > &x,
                    const unsigned int *const offsets,
                    Field *const values ) const
    {
      if( GenericGeometry::IsSimplex< Topology >::value )
        evaluateSimplex( order, x, offsets, values );
      else
        evaluatePyramid( order, x, offsets, values );
    }

    void integral ( const unsigned int order,
                    const unsigned int *const offsets,
                    Field *const values ) const
    {
      // fill first column
      baseBasis_.integral( order, offsets, values );

      const unsigned int *const baseSizes = baseBasis_.sizes_;

      Field *const col0End = values + baseSizes[ 0 ];
      for( Field *it = values; it != col0End; ++it )
        *it *= Field( 1 ) /  Field( double(dimDomain) );  // ??? double cast due to error in Linker
      Field *row0 = values;

      for( unsigned int k = 1; k <= order; ++k )
      {
        const Field factor = (Field( 1 ) / Field( k + dimDomain ));

        Field *const row1 = values+offsets[ k-1 ];
        Field *const col0End = row1 + baseSizes[ k ];
        Field *it = row1;
        for( ; it != col0End; ++it )
          *it *= factor;
        for( unsigned int i = 1; i <= k; ++i )
        {
          Field *const end = it + baseSizes[ k-i ];
          assert( (unsigned int)(end - values) <= offsets[ k ] );
          for( ; it != end; ++row0, ++it )
            *it = (factor * Field( i )) * (*row0);
        }
        row0 = row1;
      }
    }

    unsigned int maxOrder() const
    {
      return baseBasis_.maxOrder();
    }

    void computeSizes ( unsigned int order ) const
    {
      delete[] sizes_;
      delete[] numBaseFunctions_;
      sizes_            = new unsigned int[ order+1 ];
      numBaseFunctions_ = new unsigned int[ order+1 ];

      baseBasis_.computeSizes( order );

      const unsigned int *const baseNBF = baseBasis_.numBaseFunctions_;
      sizes_[ 0 ] = 1;
      numBaseFunctions_[ 0 ] = 1;
      for( unsigned int k = 1; k <= order; ++k )
      {
        sizes_[ k ]            = baseNBF[ k ];
        numBaseFunctions_[ k ] = numBaseFunctions_[ k-1 ] + sizes_[ k ];
      }
    }
  };



  // MonomialBasis
  // -------------

  template< class Topology, class F >
  class MonomialBasis
    : public MonomialBasisImpl< Topology, F >
  {
    typedef MonomialBasis< Topology, F > This;
    typedef MonomialBasisImpl< Topology, F > Base;

  public:
    typedef typename Base::Field Field;

    typedef typename Base::DomainVector DomainVector;

    MonomialBasis ()
      : Base()
    {}

    const unsigned int *sizes ( unsigned int order ) const
    {
      if( order > Base::maxOrder() )
        Base::computeSizes( order );
      return Base::numBaseFunctions_;
    }

    const unsigned int size ( unsigned int order ) const
    {
      return sizes( order )[ order ];
    }

    void evaluate ( const unsigned int order,
                    const DomainVector &x,
                    Field *const values ) const
    {
      Base::evaluate( order, x, sizes( order ), values );
    }

    void evaluate ( const unsigned int order,
                    const DomainVector &x,
                    FieldVector<Field,1> *const values ) const
    {
      evaluate( order, x, reinterpret_cast< Field * >( values ) );
    }

    template <class RangeVector>
    void evaluate ( const unsigned int order,
                    const DomainVector &x,
                    std::vector< RangeVector > &values ) const
    {
      evaluate( order, x, &(values[ 0 ]) );
    }

    void integral ( const unsigned int order,
                    Field *const values ) const
    {
      Base::integral( order, sizes( order ), values );
    }

    void integral ( const unsigned int order,
                    FieldVector<Field,1> *const values ) const
    {
      integral( order, reinterpret_cast< Field * >( values ) );
    }

    template <class RangeVector>
    void integral ( const unsigned int order,
                    std::vector< RangeVector > &values ) const
    {
      integral( order, &(values[ 0 ]) );
    }
  private:
    MonomialBasis(const This&);
    This& operator=(const This&);
  };
  template< int dim, class F >
  class VirtualMonomialBasis
  {
    typedef VirtualMonomialBasis< dim, F > This;

  public:
    typedef F Field;
    static const int dimension = dim;

    typedef FieldVector<Field,dimension> DomainVector;

    virtual ~VirtualMonomialBasis() {}

    virtual const unsigned int *sizes ( unsigned int order ) const = 0;

    const unsigned int size ( unsigned int order ) const
    {
      return sizes( order )[ order ];
    }

    virtual void evaluate ( const unsigned int order,
                            const DomainVector &x,
                            Field *const values ) const = 0;
    void evaluate ( const unsigned int order,
                    const DomainVector &x,
                    FieldVector<Field,1> *const values ) const
    {
      evaluate( order, x, reinterpret_cast< Field * >( values ) );
    }
    template <class RangeVector>
    void evaluate ( const unsigned int order,
                    const DomainVector &x,
                    std::vector< RangeVector > &values ) const
    {
      evaluate( order, x, &(values[ 0 ]) );
    }

    virtual void integral ( const unsigned int order,
                            Field *const values ) const = 0;
    void integral ( const unsigned int order,
                    FieldVector<Field,1> *const values ) const
    {
      integral( order, reinterpret_cast< Field * >( values ) );
    }
    template <class RangeVector>
    void integral ( const unsigned int order,
                    std::vector< RangeVector > &values ) const
    {
      integral( order, &(values[ 0 ]) );
    }
  };
  template< class Topology, class F >
  class VirtualMonomialBasisImpl
    : public VirtualMonomialBasis< Topology::dimension, F >
  {
    typedef VirtualMonomialBasis< Topology::dimension, F > Base;
    typedef VirtualMonomialBasisImpl< Topology, F > This;

  public:
    typedef typename Base::Field Field;
    typedef typename Base::DomainVector DomainVector;

    const unsigned int *sizes ( unsigned int order ) const
    {
      return basis_.sizes(order);
    }

    void evaluate ( const unsigned int order,
                    const DomainVector &x,
                    Field *const values ) const
    {
      basis_.evaluate(order,x,values);
    }

    void integral ( const unsigned int order,
                    Field *const values ) const
    {
      basis_.integral(order,values);
    }

  private:
    MonomialBasis<Topology,Field> basis_;
  };

  template< int dim, class F >
  struct MonomialBasisCreator
  {
    typedef F StorageField;
    typedef VirtualMonomialBasis<dim,StorageField> Basis;
    static const int dimension = dim;
    typedef unsigned int Key;

    template <class Topology>
    struct Maker
    {
      static void apply(unsigned int order,Basis* &basis)
      {
        basis = new VirtualMonomialBasisImpl<Topology,StorageField>;
      }
    };
  };

  template< int dim, class SF >
  struct MonomialBasisProvider
    : public BasisProvider<MonomialBasisCreator<dim,SF> >
  {};


#if 0
  template< int dim, class F >
  struct MonomialBasisProvider
  {
    static const int dimension = dim;
    typedef F Field;
    typedef VirtualMonomialBasis<dimension,Field> Basis;

    static const Basis &basis(unsigned int id)
    {
      return instance().getBasis(id);
    }
    template <class Topology>
    static const Basis &basis()
    {
      return instance().template getBasis<Topology>();
    }

    template <class Impl>
    static void callback(unsigned int topologyId)
    {
      GenericGeometry::IfTopology<CallBack<Impl>::template Helper,dim>::apply(topologyId);
    }
    template <class Impl,class T1>
    static void callback(unsigned int topologyId, T1 &t1)
    {
      GenericGeometry::IfTopology<CallBack<Impl>::template Helper,dim>::apply(topologyId,t1);
    }
    template <class Impl,class T1,class T2>
    static void callback(unsigned int topologyId, T1 &t1, T2 &t2)
    {
      GenericGeometry::IfTopology<CallBack<Impl>::template Helper,dim>::apply(topologyId,t1,t2);
    }

  private:
    enum { numTopologies = (1 << dimension) };
    MonomialBasisProvider()
    {}
    static MonomialBasisProvider &instance()
    {
      static MonomialBasisProvider instance;
      return instance;
    }
    const Basis &getBasis(unsigned int id)
    {
      if (basis_[id] == 0)
        MonomialBasisProvider::callback(*this,id);
      return basis_[id];
    }
    template <class Topology>
    const Basis &getBasis()
    {
      const unsigned int id = Topology::id;
      if (basis_[id] == 0)
        basis_[id] = new VirtualMonomialBasisImpl<Topology,Field>;
      return *(basis_[id]);
    }
    template <class Topology>
    static void apply(const Basis &)
    {}

    template <class Impl>
    struct CallBack
    {
      template <class Topology>
      struct Helper
      {
        static void apply()
        {
          Impl::template apply<Topology>(instance().template getBasis<Topology>());
        }
        template <class T1>
        static void apply(T1 &t1)
        {
          Impl::template apply<Topology>(instance().template getBasis<Topology>(),t1);
        }
        template <class T1,class T2>
        static void apply(T1 &t1, T2 &t2)
        {
          Impl::template apply<Topology>(instance().template getBasis<Topology>(),t1,t2);
        }
      };
    };

    FieldVector<Basis*,numTopologies> basis_;
  };
#endif


  // StdMonomialTopology
  // -------------------

  template <int dim>
  struct StdMonomialTopology {
    typedef StdMonomialTopology<dim-1> BaseType;
    typedef GenericGeometry::Pyramid<typename BaseType::Type> Type;
  };
  template <>
  struct StdMonomialTopology<0> {
    typedef GenericGeometry::Point Type;
  };
  template <int dim>
  struct StdBiMonomialTopology {
    typedef GenericGeometry::Prism<typename StdBiMonomialTopology<dim-1>::Type> Type;
  };
  template <>
  struct StdBiMonomialTopology<0> {
    typedef GenericGeometry::Point Type;
  };
  template< int dim,class F >
  class StandardMonomialBasis
    : public MonomialBasis< typename StdMonomialTopology<dim>::Type, F >
  {
  public:
    typedef typename StdMonomialTopology<dim>::Type Topology;
    static const int dimension = dim;
  private:
    typedef StandardMonomialBasis< dim, F > This;
    typedef MonomialBasis< Topology, F > Base;
  public:
    StandardMonomialBasis ()
      : Base()
    {}
  };
  template< int dim, class F >
  class StandardBiMonomialBasis
    : public MonomialBasis< typename StdBiMonomialTopology<dim>::Type, F >
  {
  public:
    typedef typename StdBiMonomialTopology<dim>::Type Topology;
    static const int dimension = dim;
  private:
    typedef StandardBiMonomialBasis< dim, F > This;
    typedef MonomialBasis< Topology, F > Base;
  public:
    StandardBiMonomialBasis ()
      : Base()
    {}
  };

}

#endif
