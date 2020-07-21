// See LICENSE_CELLO file for license and copyright information

/// @file     problem_MethodFluxCorrect.hpp 
/// @author   James Bordner (jobordner@ucsd.edu) 
/// @date     2020-03-19
/// @brief    [\ref Problem] Declaration for the MethodFluxCorrect class

#ifndef PROBLEM_METHOD_FLUX_CORRECT_HPP
#define PROBLEM_METHOD_FLUX_CORRECT_HPP

class MethodFluxCorrect : public Method
{
  /// @class    MethodFluxCorrect
  /// @ingroup  MethodFluxCorrect
  /// @brief    [\ref MethodFluxCorrect] Declaration of MethodFluxCorrect
  ///
  /// Correct fluxes of conserved fields at AMR refinement level jumps

public: // interface

  /// Create a new MethodFluxCorrect
  MethodFluxCorrect (const std::string group, int sign) throw() ;

  /// Destructor
  virtual ~MethodFluxCorrect() throw()
  {};

  /// Charm++ PUP::able declarations
  PUPable_decl(MethodFluxCorrect);

  /// Charm++ PUP::able migration constructor
  MethodFluxCorrect (CkMigrateMessage *m)
    : Method(m)
  { }

  /// CHARM++ Pack / Unpack function
  void pup (PUP::er &p)
  {
    TRACEPUP;
    Method::pup(p);
    p | ir_pre_;
    p | group_;
    p | sign_;
    p | field_sum_;
    p | field_sum_0_;
  };

  void compute_continue_refresh ( Block * block) throw();
  void compute_continue_sum_fields ( Block * block, CkReductionMsg * msg) throw();

public: // virtual functions

  /// Apply the method to advance a block one timestep 

  virtual void compute ( Block * block) throw();

  /// Return the name of this MethodFluxCorrect
  virtual std::string name () throw ()
  { return "flux_correct"; }

protected: // functions

  void flux_correct_ (Block * block);
  
protected: // attributes

  /// Refresh id
  int ir_pre_;
  /// Field group to apply flux-correction to
  std::string group_;

  /// Temporary attribute to test sign for correction (may be 0 to disable)
  int sign_;

  std::vector<long double> field_sum_;
  std::vector<long double> field_sum_0_;
};


#endif /* PROBLEM_METHOD_FLUXCORRECT_HPP */
