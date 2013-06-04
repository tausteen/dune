//***************************************************************************
// Copyright 2007-2013 Universidade do Porto - Faculdade de Engenharia      *
// Laboratório de Sistemas e Tecnologia Subaquática (LSTS)                  *
//***************************************************************************
// This file is part of DUNE: Unified Navigation Environment.               *
//                                                                          *
// Commercial Licence Usage                                                 *
// Licencees holding valid commercial DUNE licences may use this file in    *
// accordance with the commercial licence agreement provided with the       *
// Software or, alternatively, in accordance with the terms contained in a  *
// written agreement between you and Universidade do Porto. For licensing   *
// terms, conditions, and further information contact lsts@fe.up.pt.        *
//                                                                          *
// European Union Public Licence - EUPL v.1.1 Usage                         *
// Alternatively, this file may be used under the terms of the EUPL,        *
// Version 1.1 only (the "Licence"), appearing in the file LICENCE.md       *
// included in the packaging of this file. You may not use this work        *
// except in compliance with the Licence. Unless required by applicable     *
// law or agreed to in writing, software distributed under the Licence is   *
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF     *
// ANY KIND, either express or implied. See the Licence for the specific    *
// language governing permissions and limitations at                        *
// https://www.lsts.pt/dune/licence.                                        *
//***************************************************************************
// Author: Ricardo Bencatel                                                 *
//***************************************************************************

#ifndef DUNE_SIMULATION_UAV_HPP_INCLUDED_
#define DUNE_SIMULATION_UAV_HPP_INCLUDED_

// ISO C++ 98 headers.
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// DUNE headers.
#include <DUNE/Config.hpp>

namespace DUNE
{
  // Forward declarations.
  namespace Parsers { class Config; }

  namespace Simulation
  {
    // Export DLL Symbol.
    class DUNE_DLL_SYM UAVSimulation;

    class UAVSimulation
    {
    public:
      class Error: public std::runtime_error
      {
      public:
        Error(const std::string& msg):
          std::runtime_error("UAV simulation error: " + msg)
        { }
      };

      //! Constructor.
      //! Create a simulated state based on the initial state.
      //! @param[in] m reference to matrix
      UAVSimulation(const Matrix& m);

      //! This methods updates the simulated state with the defined time step, controls, and wind state.
      //! @param[in] d_t time step for the update
      //! @param[in] vd_ctrl vector of the controls to be applied
      //! @param[in] vd_wind vector of the wind state to be applied
      //! @return reference to the updated state
      Matrix&
      operator+=(const Matrix& m);


    private:
      static double precision;

      size_t m_nrows;
      size_t m_ncols;
      size_t m_size;
      double* m_data;
      double* m_counter;

      //! This method acts as destructor.
      void
      erase(void);

      //! This method creates a unique copy of the data of a Matrix.
      void
      split(void);
    };
  }
}

#endif