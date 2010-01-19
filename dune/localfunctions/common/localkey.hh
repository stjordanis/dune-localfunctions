// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LOCALKEY_HH
#define DUNE_LOCALKEY_HH

#include <cstddef>

#include <dune/common/array.hh>

namespace Dune
{
  // the number of the beast ...
  enum {
    //! codim that indicates degree of freedom in intersection
    intersectionCodim=666
  };

  /**@ingroup LocalLayoutInterface
         \brief Describe position of one degree of freedom

         A LocalKey associates a degree of freedom with an index
         of a local basis function.

         \nosubgrouping
   */
  class LocalKey
  {
  public:
    //! \brief Standard constructor for uninitialized local index
    LocalKey ()
    {}

    /** \brief Initialize all components
        \param s Local number of the associated subentity
        \param c Codimension of the associated subentity
        \param i Index in the set of all functions associated to this subentity
     */
    LocalKey (unsigned int s, unsigned int c, unsigned int i)
    {
      values_[0] = s;
      values_[1] = c;
      values_[2] = i;
    }

    //! \brief Return number of associated subentity
    inline unsigned int subEntity () const
    {
      return values_[0];
    }

    //! \brief Return codim of associated entity
    inline unsigned int codim () const
    {
      return values_[1];
    }

    //! \brief Return offset within subentity
    inline unsigned int index () const
    {
      return values_[2];
    }

    //! \brief Set index component
    void index (unsigned int i)
    {
      values_[2] = i;
    }

    /** \brief Less-than operator so we can use this class as a key type in stl containers */
    bool operator< (const LocalKey& other) const
    {
      return values_ < other.values_;
    }

  private:

    // We use an array to store the values in order to be able to use the array::operator< implementation
    Dune::array<unsigned int,3> values_;

  };

}
#endif