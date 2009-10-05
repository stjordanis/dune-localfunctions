// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ORTHONORMALCOMPUTE_HH
#define DUNE_ORTHONORMALCOMPUTE_HH

#include <iostream>
#include <fstream>
#include <map>
#include <iomanip>
#include <cassert>
#include <alglib/qr.h>
#include <alglib/sevd.h>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/alglib/multiprecision.hh>

namespace ONB {
  template <class scalar_t>
  scalar_t factorial(int start,int end) {
    scalar_t ret(1);
    for (int j=start; j<=end; j++)
      ret*=j;
    return ret;
  }
  template <class Topology>
  struct Integral;
  template <class Base>
  struct Integral<Dune::GenericGeometry::Pyramid<Base> > {
    enum {d = Base::dimension+1};
    template <int dim,class scalar_t>
    static int compute(const Dune::MultiIndex<dim>& alpha,
                       scalar_t& p,scalar_t& q) {
      int i = alpha.z(d-1);
      int ord = Integral<Base>::compute(alpha,p,q);
      p *= factorial<scalar_t>(1,i);
      q *= factorial<scalar_t>(d+ord,d+ord+i);
      return ord+i;
    }
  };
  template <class Base>
  struct Integral<Dune::GenericGeometry::Prism<Base> > {
    enum {d = Base::dimension+1};
    template <int dim,class scalar_t>
    static int compute(const Dune::MultiIndex<dim>& alpha,
                       scalar_t& p,scalar_t& q) {
      int i = alpha.z(d-1);
      int ord = Integral<Base>::compute(alpha,p,q);
      Integral<Base>::compute(alpha,p,q);
      p *= 1.;
      q *= (i+1.);
      return ord+i;
    }
  };
  template <>
  struct Integral<Dune::GenericGeometry::Point> {
    template <int dim,class scalar_t>
    static int compute(const Dune::MultiIndex<dim>& alpha,
                       scalar_t& p,scalar_t& q) {
      p = 1.;
      q = 1.;
      return 0;
    }
  };
  template <class scalar_t>
  class Compute
  {
    typedef ap::template_1d_array< scalar_t > vec_t;
    typedef ap::template_2d_array< scalar_t > mat_t;


    /**************************************************/
    template <class Topology>
    struct CalcCoeffsBase {
      enum {dim=Topology::dimension};
      vec_t d;
      mat_t S;
      mat_t& res;
      CalcCoeffsBase(mat_t& pres) : res(pres) {}
      virtual ~CalcCoeffsBase() {}
      void test() {
        int N = this->res.gethighbound(1);
        // Nun schauen wir noch, ob wirklich C^T S C = E gilt
        scalar_t prod;
        for (int i=1; i<=N; ++i) {
          for (int j=1; j<=N; j++) {
            prod = 0;
            for (int k=1; k<=N; k++) {
              for (int l=1; l<=N; l++) {
                prod += res(l,i)*S(l,k)*res(k,j);
              }
            }
            assert((i==j) ? 1 : // fabs(prod.toDouble()-1)<1e-10 :
                   fabs(prod.toDouble())<1e-10);
            // std::cout << "(" << i << "," << j << ")" << prod.toDouble() << std::endl;
          }
        }
      }
      virtual void compute() = 0;
      void compute(int ord) {
        // get all multiindecies for monomial basis
        typedef Dune::MultiIndex<dim> MI;
        typedef Dune::StandardMonomialBasis< dim, MI  > Basis;
        Basis basis;
        const unsigned int size = basis.sizes( ord )[ ord ];
        std::vector< Dune::FieldVector< MI,1> > y( size );
        Dune::FieldVector< MI, dim > x;
        for (int i=0; i<dim; ++i) {
          x[i].set(i);
        }
        basis.evaluate( ord , x, y );
        // set bounds of data
        res.setbounds(1,size,1,size);
        S.setbounds(1,size,1,size);
        d.setbounds(1,size);
        // Aufstellen der Matrix fuer die Bilinearform xSy
        // S_ij = int_A x^(i+j)
        scalar_t p,q;
        for( unsigned int i=1; i<=size; ++i )
        {
          for( unsigned int j=1; j<=size; j++ )
          {
            Integral<Topology>::compute(y[i-1]*y[j-1],p,q);
            S(i,j) = p;
            S(i,j) /= q;
          }
        }
        compute();
        test();
        return;
      }
    };
    template <class Topology>
    struct CalcCoeffsGM : public CalcCoeffsBase<Topology> {
      CalcCoeffsGM(mat_t& pres)
        : CalcCoeffsBase<Topology>(pres) {}
      virtual ~CalcCoeffsGM() {}
      void sprod(int col1,int col2,
                 scalar_t& ret) {
        ret = 0;
        for (int k=1; k<=col1; k++) {
          for (int l=1; l<=col2; l++) {
            ret += this->res(l,col2)*this->S(l,k)*this->res(k,col1);
          }
        }
      }
      virtual void compute() {
        int N = this->res.gethighbound(1);
        scalar_t s;
        for (int i=1; i<=N; ++i)
          for (int j=1; j<=N; ++j)
            this->res(i,j) = (i==j) ? 1 : 0;
        sprod(1,1,s);
        // s = 1./amp::sqrt<Precision>(s);
        s = 1./amp::sqrt(s);
        vmul(this->res.getcolumn(1,1,1),s);
        for (int i=1; i<N; i++) {
          for (int k=1; k<=i; ++k) {
            sprod(i+1,k,s);
            vsub(this->res.getcolumn(i+1,1,i+1),
                 this->res.getcolumn(k,  1,i+1),s);
          }
          sprod(i+1,i+1,s);
          // s = 1./amp::sqrt<Precision>(s);
          s = 1./amp::sqrt(s);
          vmul(this->res.getcolumn(i+1,1,i+1),s);
        }
      }
    };
#if 0
    template <int dim,int ord>
    struct CalcCoeffsQR : public CalcCoeffsBase<dim,ord> {
      enum {N = Monoms<dim,ord>::size};
      mat_t a,q,r,z,Ssqrt,Ssqrtinv,tmp,tmpinv;
      CalcCoeffsQR(mat_t& pres) : CalcCoeffsBase<dim,ord>(pres) {
        q.setbounds(1,N,1,N);
        r.setbounds(1,N,1,N);
        z.setbounds(1,N,1,N);
        Ssqrt.setbounds(1,N,1,N);
        tmp.setbounds(1,N,1,N);
        Ssqrtinv.setbounds(1,N,1,N);
        tmpinv.setbounds(1,N,1,N);
      }
      virtual ~CalcCoeffsQR() {}
      void compute() {

        /*
           for (int i=1;i<=N;++i) {
           for (int j=1;j<=N;j++)
            std::cout << S(i,j).toDouble() << "\t\t" << std::flush;
           std::cout << std::endl;
           }
         */

        // Berechne die Eigenwerte d_1,..,d_N und ein ONS von
        // Rechtseigenvektoren
        sevd::symmetricevd(this->S,N,1,true,this->d,z);
        // Berechne SQRT(S) und SQRT(S)^(-1)
        // Dazu: S=R^T D R und S^(-1) = R^T D^(-1) R
        for (int i=1; i<=N; ++i) {
          this->d(i) = amp::sqrt<Precision>(this->d(i));
        }
        for (int i=1; i<=N; ++i)
          for (int j=1; j<=N; j++) {
            Ssqrt(i,j) = 0;
            Ssqrtinv(i,j) = 0;
            tmp(i,j) = 0;
            tmpinv(i,j) = 0;
          }
        for (int i=1; i<=N; ++i) {
          for (int j=1; j<=N; j++) {
            tmp(i,j) = this->d(i)*z(j,i);
            tmpinv(i,j) = z(j,i)/this->d(i);
          }
        }
        for (int i=1; i<=N; ++i) {
          for (int j=1; j<=N; j++) {
            for (int k=1; k<=N; k++) {
              Ssqrt(i,j) = Ssqrt(i,j) + z(i,k)*tmp(k,j);
              Ssqrtinv(i,j) = Ssqrtinv(i,j) + z(i,k)*tmpinv(k,j);
            }
          }
        }
        // Testen das alles so weit stimmt....
        for (int i=1; i<=N; ++i)
          for (int j=1; j<=N; j++) {
            tmp(i,j) = 0;
            tmpinv(i,j) = 0;
          }
        // SQRT(S), SQRT(S)^(-1) sind symmetrisch ...
        for (int i=1; i<=N; ++i) {
          for (int j=1; j<=N; j++) {
            assert(fabs(Ssqrt(i,j).toDouble() - Ssqrt(j,i).toDouble())<1e-10);
            assert(fabs(Ssqrtinv(i,j).toDouble() - Ssqrtinv(j,i).toDouble())<1e-10);
            for (int k=1; k<=N; k++) {
              tmp(i,j) = tmp(i,j) + Ssqrt(k,i)*Ssqrt(k,j);
              tmpinv(i,j) = tmpinv(i,j) + Ssqrt(k,i)*Ssqrtinv(k,j);
            }
          }
        }
        // ... und SQRT(S)^2 = S
        // ... und SQRT(S)*SQRT(S)^(-1) = E
        for (int i=1; i<=N; ++i) {
          for (int j=1; j<=N; j++) {
            assert(fabs(tmp(i,j).toDouble() - this->S(i,j).toDouble())<1e-10);
            assert((i==j) ? (tmpinv(i,j).toDouble()-1)<1e-10 : (tmpinv(i,j).toDouble())<1e-10);
          }
        }
        // Nun berechne die QR Zerlegung von SQRT(S)
        qr::qrdecompositionunpacked(Ssqrt,N,N,q,r);
        // Nun die gesuchten Koeffizienten:
        // C = SQRT(S)^(-1)q
        // Es gilt jetzt:
        // SQRT(S) = qr
        // ->   E = SQRT(S)^(-1)qr = Cr
        // Da: q^T = q^(-1) bzw. q^T SQRT(S) = r
        // ->   E = Cr = Cq^T SQRT(S)
        // C^T S C = q^T SQRT(S)^(-T)S SQRT(S)^(-1) q = q^T S^(-1)S q = q^Tq = E
        // d.h. die Spalten sind orthogonal bzw. der Bilinearform S!
        for (int i=1; i<=N; ++i) {
          for (int j=1; j<=N; j++) {
            this->res(i,j) = 0;
            for (int k=1; k<=N; k++) {
              this->res(i,j) = this->res(i,j) + Ssqrtinv(k,i)*q(k,j);
            }
          }
        }
        // ... wir wollen noch einen positiven f\"uhrenden Koeffizienten ...
        for (int i=1; i<=N; ++i) {
          if (this->res(i,i).isNegativeNumber())
            for (int j=1; j<=N; j++) {
              this->res(j,i) *= -1.0;
            }
          // ... event. Normieren ...
          /*
             for (int j=1;j<=N;j++) {
             res(j,i) = res(j,i) / res(i,i);
             }
           */
        }
      }
    };
#endif
  public:
    template <class Topology>
    struct CalcCoeffs {
      mat_t res;
      CalcCoeffsBase<Topology> *method;
      CalcCoeffs(int use_method) {
        if (use_method==1)
          method = new CalcCoeffsGM<Topology>(res);
        else
          method = 0; // new CalcCoeffsQR<dim,ord>(res);
      }
      ~CalcCoeffs() {
        delete method;
      }
      void compute(int order) {
        method->compute(order);
      }
    };
  };
  template <class Topology,class scalar_t>
  struct ONBMatrix {
    enum {dim = Topology::dimension};
    typedef ap::template_1d_array< scalar_t > vec_t;
    typedef ap::template_2d_array< scalar_t > mat_t;
    ONBMatrix(int maxOrder)
      : calc(1)
    {
      calc.compute(maxOrder);
    }
    int colSize(int row) const {
      return row+1;
    }
    int rowSize() const {
      return calc.res.gethighbound(1);
    }

    const Dune::FieldVector< scalar_t, 1 > operator() ( int r, int c ) const
    {
      return calc.res(c+1,r+1);
    }

    void print(std::ostream& out) {
      int N = rowSize();
      for (int i=0; i<N; ++i) {
        out << "Polynomial : " << i << std::endl;
        for (int j=0; j<colSize(i); j++) {
          double v = calc.res(j+1,i+1).toDouble();
          if (fabs(v)<1e-20)
            out << 0 << "\t\t" << std::flush;
          else {
            Dune::AlgLib::MultiPrecision<128> v = calc.res(j+1,i+1);
            out << v << "\t\t" << std::flush;
          }
        }
        for (int j=colSize(i); j<N; j++) {
          assert(fabs(calc.res(j+1,i+1).toDouble())<1e-10);
        }
        out << std::endl;
      }
    }
    typename Compute<scalar_t>::template CalcCoeffs<Topology> calc;
  };
}
#endif