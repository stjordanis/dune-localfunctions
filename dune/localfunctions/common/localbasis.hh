// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LOCALBASIS_HH
#define DUNE_LOCALBASIS_HH

#include <iostream>
#include <vector>

#include <dune/common/fvector.hh>

namespace Dune
{

  /**@ingroup LocalBasisInterface
         \brief Type traits for LocalBasisVirtualInterface

         A shape function is a function
         \f[ \hat\phi : \mbox{IR}^n \to \mbox{IR}^m. \f]
         This traits class holds information how the signature of this
         function is represented in C++ types.

         This is just a convenience class for supplying traits to the
         LocalBasisVirtualInterface and its implementations.

         \tparam DF Type to represent the field in the domain.
         \tparam n  Dimension of the domain.
         \tparam D  Type to represent the domain, allows random access.
         \tparam RF Type to represent the field in the range.
         \tparam m  Dimension of the range.
         \tparam R  Type to represent the range, allows random access.
         \tparam J  Type to represent the Jacobian, allows random access.
         \tparam dorder Maximal order of implemented partial derivatives

         \nosubgrouping
   */
  template<class DF, int n, class D, class RF, int m, class R, class J, int dorder=0>
  struct LocalBasisTraits
  {
    //! \brief Export type for domain field
    typedef DF DomainFieldType;

    //! \brief Enum for domain dimension
    enum {
      //! \brief dimension of the domain
      dimDomain = n
    };

    //! \brief domain type
    typedef D DomainType;

    //! \brief Export type for range field
    typedef RF RangeFieldType;

    //! \brief Enum for range dimension
    enum {
      //! \brief dimension of the range
      dimRange = m
    };

    //! \brief range type
    typedef R RangeType;

    /** \brief Type to represent derivative

            When \f$ \hat\phi : \mbox{IR}^n \to \mbox{IR}^m \f$ then JacobianType
            is an 2D-array of m x n components where entry J[i][j] contains
            the derivative  \f$\partial_j \hat\phi_i \f$.
     */
    typedef J JacobianType;

    //! \brief Enum for differentiability order
    enum {
      //! \brief number of partial derivatives supported
      diffOrder=dorder
    };
  };


  /**@ingroup LocalBasisInterface
         \brief Type traits for LocalBasisInterface

         A shape function is a function
         \f[ \hat\phi : \mbox{IR}^n \to \mbox{IR}^m. \f]
         This traits class holds information how the signature of this
         function is represented in C++ types.

         This is just a convenience class for supplying traits to the
         C0LocalBasisInterface.

         \tparam DF Type to represent the field in the domain.
         \tparam n  Dimension of the domain.
         \tparam D  Type to represent the domain, allows random access.
         \tparam RF Type to represent the field in the range.
         \tparam m  Dimension of the range.
         \tparam R  Type to represent the range, allows random access.

         \nosubgrouping
   */
  template<class DF, int n, class D, class RF, int m, class R>
  struct C0LocalBasisTraits
  {
    //! \brief Export type for domain field
    typedef DF DomainFieldType;

    //! \brief Enum for domain dimension
    enum {
      //! \brief dimension of the domain
      dimDomain = n
    };

    //! \brief domain type
    typedef D DomainType;

    //! \brief Export type for range field
    typedef RF RangeFieldType;

    //! \brief Enum for range dimension
    enum {
      //! \brief dimension of the range
      dimRange = m
    };

    //! \brief range type
    typedef R RangeType;

    //! \brief Enum for differentiability order
    enum {
      //! \brief number of derivatives supported
      diffOrder=0
    };
  };


  /**@ingroup LocalBasisInterface
         \brief Type traits for C1LocalBasisInterface

         Extends the traits class LocalBasisTraits for differentiable
         shape functions.

         \tparam DF Type to represent the field in the domain.
         \tparam n  Dimension of the domain.
         \tparam D  Type to represent the domain, allows random access.
         \tparam RF Type to represent the field in the range.
         \tparam m  Dimension of the range.
         \tparam R  Type to represent the range, allows random access.
         \tparam J  Type to represent the Jacobian, allows random access.

         \nosubgrouping
   */
  template<class DF, int n, class D, class RF, int m, class R, class J>
  struct C1LocalBasisTraits : public C0LocalBasisTraits<DF,n,D,RF,m,R>
  {
    /** \brief Type to represent derivative

            When \f$ \hat\phi : \mbox{IR}^n \to \mbox{IR}^m \f$ then JacobianType
            is an 2D-array of m x n components where entry J[i][j] contains
            the derivative  \f$\partial_j \hat\phi_i \f$.
     */
    typedef J JacobianType;

    //! \brief Enum for differentiability order
    enum {
      //! \brief number of derivatives supported
      diffOrder=1
    };
  };

  template<class DF, int n, class D, class RF, int m, class R, class J, int dorder>
  struct CkLocalBasisTraits : public C1LocalBasisTraits<DF,n,D,RF,m,R,J>
  {
    //! \brief Enum for differentiability order
    enum {
      //! \brief number of derivatives supported
      diffOrder=dorder
    };
  };

}
#endif
