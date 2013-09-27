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
// Author: Ricardo Martins                                                  *
//***************************************************************************

// ISO C++ 98 headers.
#include <vector>

#ifndef TRANSPORTS_EVOLOGICS_REPLIES_HPP_INCLUDED_
#define TRANSPORTS_EVOLOGICS_REPLIES_HPP_INCLUDED_

namespace Transports
{
  namespace Evologics
  {
    struct RecvIM
    {
      RecvIM(void):
        src(0),
        dst(0),
        ack(false),
        bitrate(0),
        rssi(0),
        velocity(0),
        propagation_time(0),
        duration(0)
      { }

      unsigned src;
      unsigned dst;
      bool ack;
      unsigned bitrate;
      float rssi;
      unsigned integrity;
      float velocity;
      float propagation_time;
      std::vector<uint8_t> data;
      unsigned duration;
    };
  }
}

#endif