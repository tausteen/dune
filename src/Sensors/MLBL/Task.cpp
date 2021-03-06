//***************************************************************************
// Copyright 2007-2014 Universidade do Porto - Faculdade de Engenharia      *
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
#include <memory>
#include <string>
#include <cstddef>

// DUNE headers.
#include <DUNE/DUNE.hpp>

namespace Sensors
{
  namespace MLBL
  {
    using DUNE_NAMESPACES;

    //! Duration of a Mini-Packet in seconds.
    static const double c_mpk_duration = 0.90;
    //! Default command reply timeout.
    static const double c_cmd_reply_tout = 4.0;
    //! Cycle Init Timeout (s).
    static const unsigned c_cto = 10;
    //! Input Timeout (s).
    static const double c_input_tout = c_cto + 5;
    //! Abort code.
    static const unsigned c_code_abort = 0x000a;
    //! Abort acked code.
    static const unsigned c_code_abort_ack = 0x000b;
    //! Start plan acknowledge code.
    static const unsigned c_code_plan_ack = 0x000c;
    //! Restart system code.
    static const unsigned c_code_sys_restart = 0x01a6;
    //! Restart system ack code.
    static const unsigned c_code_sys_restart_ack = 0x01a7;

    enum EntityStates
    {
      STA_BOOT,
      STA_NO_BEACONS,
      STA_IDLE,
      STA_ACTIVE,
      STA_ERR_COM,
      STA_ERR_SRC,
      STA_ERR_STP,
      STA_MAX
    };

    enum Results
    {
      // Nothing happened.
      RS_NONE = 0,
      // Address set reply.
      RS_SRC_ACKD = 1 << 1,
      // NRV set reply.
      RS_NRV_ACKD = 1 << 2,
      // Address set reply.
      RS_CTO_ACKD = 1 << 3,
      // NB Ping send acknowledgment.
      RS_PNG_ACKD = 1 << 4,
      // NB travel time.
      RS_PNG_TIME = 1 << 5,
      // Mini-Packet send acknowledgment.
      RS_MPK_ACKD = 1 << 6,
      // Mini-Packet transmission started.
      RS_MPK_STAR = 1 << 7,
      // Mini-Packet transmission ended.
      RS_MPK_SENT = 1 << 8,
      // Turn around time configured reply.
      RS_TAT_ACKD = 1 << 9,
      // XST set reply.
      RS_XST_ACKD = 1 << 10
    };

    // Configuration parameters.
    struct Arguments
    {
      // Serial port device.
      std::string uart_dev;
      // Serial port baud rate.
      unsigned uart_baud;
      // Maximum time without ranges.
      double range_tout;
      // True to send periodic reports.
      bool report;
      // True to send periodic verbose reports.
      bool report_verbose;
      // Delay before sending range reports.
      double report_delay_bef;
      // Delay after sending range reports.
      double report_delay_aft;
      // Delay before sending Mini-Packet.
      double mpk_delay_bef;
      // Delay after sending Mini-Packet.
      double mpk_delay_aft;
      // Maximum age of a good range (for reporting).
      double good_range_age;
      // Time between range reports.
      double report_period;
      // Ping Period.
      double ping_period;
      // Ping Timeout.
      unsigned ping_tout;
      // Length of transmit pings.
      unsigned tx_length;
      // Length of receive pings.
      unsigned rx_length;
      // Sound speed on water.
      double sound_speed_def;
      //! Entity label of sound speed provider.
      std::string sound_speed_elabel;
      // Turn around time (ms).
      unsigned turn_around_time;
    };

    struct Beacon
    {
      // Beacon name.
      std::string name;
      // Ping command.
      std::string ping_cmd;
      // Beacon id.
      unsigned id;
      // Beacon receiving channel.
      unsigned rx_channel;
      // Beacon receiving frequency.
      unsigned rx_frequency;
      // Beacon transmission channel.
      unsigned tx_channel;
      // Beacon transmission frequency.
      unsigned tx_frequency;
      // Last range.
      unsigned range;
      // Last range timestamp.
      double range_time;
      // Latitude.
      double lat;
      // Longitude.
      double lon;
      // Depth
      float depth;
      // Delay
      uint8_t delay;

      Beacon(void):
        id(0),
        rx_channel(0),
        rx_frequency(0),
        tx_channel(0),
        tx_frequency(0),
        range(0),
        range_time(0),
        lat(0),
        lon(0),
        depth(0)
      { }
    };

    struct Task: public DUNE::Tasks::Task
    {
      // Change plan source address.
      static const unsigned c_plan_addr = 15;
      // Maximum buffer size.
      static const int c_bfr_size = 256;
      // Beacons.
      std::vector<Beacon> m_beacons;
      // Serial port handle.
      SerialPort* m_uart;
      // Range.
      IMC::LblRange m_range;
      // Range.
      IMC::LblDetection m_detect;
      // Abort message.
      IMC::Abort m_abort;
      // Entity states.
      IMC::EntityState m_states[STA_MAX];
      // Commands to/from modem.
      IMC::DevDataText m_cmds;
      // Time of last range.
      double m_last_range;
      // Internal buffer.
      char m_bfr[c_bfr_size];
      // Task arguments.
      Arguments m_args;
      // Current state.
      EntityStates m_state;
      // Modem address.
      unsigned m_addr;
      // Bitfield with results of last processInput().
      unsigned m_result;
      // Time of last serial port input.
      double m_last_input;
      // Current sound speed (m/s).
      double m_sound_speed;
      //! Sound speed entity id.
      int m_sound_speed_eid;
      // Estimated state.
      IMC::EstimatedState m_estate;
      //! Report timer.
      Counter<double> m_report_timer;
      //! Stop reports on the ground.
      bool m_stop_reports;
      //! Last progress.
      float m_progress;
      //! Last fuel level.
      float m_fuel_level;
      //! Last fuel level confidence.
      float m_fuel_conf;

      Task(const std::string& name, Tasks::Context& ctx):
        DUNE::Tasks::Task(name, ctx),
        m_uart(NULL),
        m_last_range(0),
        m_result(RS_NONE),
        m_sound_speed_eid(-1)
      {
        // Define configuration parameters.
        paramActive(Tasks::Parameter::SCOPE_MANEUVER,
                    Tasks::Parameter::VISIBILITY_USER);

        param("Serial Port - Device", m_args.uart_dev)
        .defaultValue("")
        .description("Serial port device used to communicate with the sensor");

        param("Serial Port - Baud Rate", m_args.uart_baud)
        .defaultValue("19200")
        .description("Serial port baud rate");

        param("Sound Speed - Default Value", m_args.sound_speed_def)
        .defaultValue("1500")
        .minimumValue("1375")
        .maximumValue("1900")
        .units(Units::MeterPerSecond)
        .description("Water sound speed");

        param("Sound Speed - Entity Label", m_args.sound_speed_elabel)
        .description("Entity label of sound speed provider");

        param("Length of Transmit Pings", m_args.tx_length)
        .units(Units::Millisecond)
        .defaultValue("3")
        .minimumValue("0");

        param("Length of Receive Pings", m_args.rx_length)
        .units(Units::Millisecond)
        .defaultValue("3")
        .minimumValue("0");

        param("Ping Timeout", m_args.ping_tout)
        .units(Units::Millisecond)
        .defaultValue("1000")
        .minimumValue("0");

        param("Ping Periodicity", m_args.ping_period)
        .units(Units::Second)
        .defaultValue("2")
        .minimumValue("2");

        param(DTR_RT("Enable Reports"), m_args.report)
        .visibility(Tasks::Parameter::VISIBILITY_USER)
        .defaultValue("true")
        .description("Report data acoustically");

        param(DTR_RT("Make Reports Verbose"), m_args.report_verbose)
        .visibility(Tasks::Parameter::VISIBILITY_USER)
        .defaultValue("false")
        .description("Report more verbose data acoustically");

        param(DTR_RT("Reports Periodicity"), m_args.report_period)
        .visibility(Tasks::Parameter::VISIBILITY_USER)
        .units(Units::Second)
        .defaultValue("60")
        .minimumValue("30")
        .maximumValue("600")
        .description("Reports periodicity");

        param("Good Range Age", m_args.good_range_age)
        .units(Units::Second)
        .defaultValue("5")
        .minimumValue("0");

        param("Mini-Packet Delay - Before", m_args.mpk_delay_bef)
        .units(Units::Second)
        .defaultValue("1.0")
        .minimumValue("0");

        param("Mini-Packet Delay - After", m_args.mpk_delay_aft)
        .units(Units::Second)
        .defaultValue("0.5")
        .minimumValue("0");

        param("Range Reports Delay - Before", m_args.report_delay_bef)
        .units(Units::Second)
        .defaultValue("0.5")
        .minimumValue("0");

        param("Range Reports Delay - After", m_args.report_delay_aft)
        .units(Units::Second)
        .defaultValue("1.0")
        .minimumValue("0");

        param("Turn Around Time", m_args.turn_around_time)
        .units(Units::Millisecond)
        .defaultValue("20")
        .minimumValue("0");

        // Initialize state messages.
        m_states[STA_BOOT].state = IMC::EntityState::ESTA_BOOT;
        m_states[STA_BOOT].description = DTR("initializing");
        m_states[STA_NO_BEACONS].state = IMC::EntityState::ESTA_BOOT;
        m_states[STA_NO_BEACONS].description = DTR("waiting beacons configuration");
        m_states[STA_IDLE].state = IMC::EntityState::ESTA_NORMAL;
        m_states[STA_IDLE].description = DTR("idle");
        m_states[STA_ACTIVE].state = IMC::EntityState::ESTA_NORMAL;
        m_states[STA_ACTIVE].description = DTR("active");
        m_states[STA_ERR_COM].state = IMC::EntityState::ESTA_ERROR;
        m_states[STA_ERR_COM].description = DTR("serial port communication error, modem not responding");
        m_states[STA_ERR_STP].state = IMC::EntityState::ESTA_ERROR;
        m_states[STA_ERR_STP].description = DTR("failed to configure modem, possible serial port communication error");
        m_states[STA_ERR_SRC].state = IMC::EntityState::ESTA_ERROR;
        m_states[STA_ERR_SRC].description = DTR("failed to set modem address");

	m_stop_reports = false;

        // Register handlers.
        bind<IMC::EstimatedState>(this);
        bind<IMC::FuelLevel>(this);
        bind<IMC::LblConfig>(this);
        bind<IMC::PlanControlState>(this);
        bind<IMC::QueryEntityState>(this);
        bind<IMC::SoundSpeed>(this);
        bind<IMC::VehicleMedium>(this);
      }

      void
      onResourceAcquisition(void)
      {
        m_uart = new SerialPort(m_args.uart_dev, m_args.uart_baud);
        m_uart->setCanonicalInput(true);
        m_uart->flush();
        setAndSendState(STA_BOOT);
      }

      void
      onResourceRelease(void)
      {
        Memory::clear(m_uart);
      }

      void
      onUpdateParameters(void)
      {
        m_sound_speed = m_args.sound_speed_def;
        m_report_timer.setTop(m_args.report_period);
      }

      void
      onResourceInitialization(void)
      {
        // Get modem address.
        std::string agent = getSystemName();
        m_ctx.config.get("Micromodem Addresses", agent, "1024", m_addr);
        if (m_addr == 1024)
          throw std::runtime_error(String::str(DTR("modem address for agent '%s' is invalid"), agent.c_str()));

        // Set modem address.
        {
          NMEAWriter stn("CCCFG");
          stn << "SRC" << m_addr;
          std::string cmd = stn.sentence();
          m_uart->write(cmd.c_str(), cmd.size());

          processInput();

          if (!consumeResult(RS_SRC_ACKD))
          {
            setAndSendState(STA_ERR_SRC);
            throw std::runtime_error(m_states[m_state].description);
          }
        }

        // Set NRV parameter.
        {
          NMEAWriter stn("CCCFG");
          stn << "NRV" << 0;
          std::string cmd = stn.sentence();
          m_uart->write(cmd.c_str(), cmd.size());

          processInput();

          if (!consumeResult(RS_NRV_ACKD))
          {
            setAndSendState(STA_ERR_STP);
            throw std::runtime_error(m_states[m_state].description);
          }
        }

        // Set CTO parameter.
        {
          NMEAWriter stn("CCCFG");
          stn << "CTO" << c_cto;
          std::string cmd = stn.sentence();
          m_uart->write(cmd.c_str(), cmd.size());

          processInput();

          if (!consumeResult(RS_CTO_ACKD))
          {
            setAndSendState(STA_ERR_STP);
            throw std::runtime_error(m_states[m_state].description);
          }
        }

        // Set TAT parameter.
        {
          NMEAWriter stn("CCCFG");
          stn << "TAT" << m_args.turn_around_time;
          std::string cmd = stn.sentence();
          m_uart->write(cmd.c_str(), cmd.size());

          processInput();

          if (!consumeResult(RS_TAT_ACKD))
          {
            setAndSendState(STA_ERR_STP);
            throw std::runtime_error(m_states[m_state].description);
          }
        }

        // Set XST parameter.
        {
          NMEAWriter stn("CCCFG");
          stn << "XST" << 0;
          std::string cmd = stn.sentence();
          m_uart->write(cmd.c_str(), cmd.size());

          processInput();

          if (!consumeResult(RS_XST_ACKD))
          {
            setAndSendState(STA_ERR_STP);
            throw std::runtime_error(m_states[m_state].description);
          }
        }

        if (m_beacons.empty())
          setAndSendState(STA_NO_BEACONS);
        else
          setAndSendState(STA_IDLE);
      }

      bool
      consumeResult(Results value)
      {
        bool rv = false;

        if (m_result & value)
          rv = true;
        m_result &= ~value;

        return rv;
      }

      void
      addResult(Results value)
      {
        m_result |= value;
      }

      void
      setAndSendState(EntityStates state)
      {
        m_state = state;
        dispatch(m_states[m_state]);
      }

      void
      onEntityResolution(void)
      {
        try
        {
          m_sound_speed_eid = resolveEntity(m_args.sound_speed_elabel);
        }
        catch (...)
        {
          inf(DTR("dynamic sound speed corrections are disabled"));
          m_sound_speed = m_args.sound_speed_def;
        }
      }

      void
      onReportEntityState(void)
      {
        dispatch(m_states[m_state]);
      }

      unsigned
      channelToFrequency(unsigned channel)
      {
        return channel * 1000 + 22000;
      }

      void
      handleConfigParam(std::auto_ptr<NMEAReader>& stn)
      {
        std::string arg;
        *stn >> arg;

        if (arg == "SRC")
        {
          unsigned src;
          *stn >> src;
          if (src == m_addr)
            addResult(RS_SRC_ACKD);
        }
        else if (arg == "NRV")
        {
          unsigned value;
          *stn >> value;
          if (value == 0)
            addResult(RS_NRV_ACKD);
        }
        else if (arg == "CTO")
        {
          unsigned value;
          *stn >> value;
          if (value == c_cto)
            addResult(RS_CTO_ACKD);
        }
        else if (arg == "XST")
        {
          unsigned value;
          *stn >> value;
          if (value == 0)
            addResult(RS_XST_ACKD);
        }
        else if (arg == "TAT")
        {
          unsigned value;
          *stn >> value;
          if (value == m_args.turn_around_time)
            addResult(RS_TAT_ACKD);
        }
      }

      void
      handleMiniPacket(std::auto_ptr<NMEAReader>& stn)
      {
        unsigned src = 0;
        *stn >> src;
        unsigned dst = 0;
        *stn >> dst;

        // Not for me.
        if (dst != m_addr)
          return;

        // Change plan request.
        if (src == c_plan_addr)
        {
          std::string val;
          unsigned value = 0;
          *stn >> val;
          inf("%s", val.c_str());
          if (std::sscanf(val.c_str(), "%04X", &value) != 1)
          {
            err(DTR("invalid change plan message"));
            return;
          }

          IMC::OperationalLimits ol;
          ol.mask = 0;
          dispatch(ol);

          Delay::wait(1.0);

          char plan_name[2] = {(char)value, 0};

          IMC::PlanControl pc;
          pc.type = IMC::PlanControl::PC_REQUEST;
          pc.op = IMC::PlanControl::PC_START;
          pc.plan_id.assign(plan_name);
          pc.flags = IMC::PlanControl::FLG_IGNORE_ERRORS;
          dispatch(pc);

          war(DTR("start plan detected"));

          std::string cmd = String::str("$CCMUC,%u,%u,%04x\r\n", m_addr, src, c_code_plan_ack);
          processInput(m_args.mpk_delay_bef);
          m_uart->write(cmd.c_str(), cmd.size());
          processInput(c_mpk_duration + m_args.mpk_delay_aft);

          if (consumeResult(RS_MPK_ACKD) && consumeResult(RS_MPK_STAR) && consumeResult(RS_MPK_SENT))
            inf(DTR("plan acknowledged"));
          else
            inf(DTR("failed to acknowledge plan start"));

          return;
        }

        // Get value.
        std::string val;
        *stn >> val;

        unsigned value = 0;
        std::sscanf(val.c_str(), "%04X", &value);

        if (value == c_code_sys_restart)
        {
          war(DTR("received system restart request"));

          std::string cmd = String::str("$CCMUC,%u,%u,%04x\r\n", m_addr, src, c_code_sys_restart_ack);
          processInput(m_args.mpk_delay_bef);
          m_uart->write(cmd.c_str(), cmd.size());
          processInput(c_mpk_duration + m_args.mpk_delay_aft);

          if (consumeResult(RS_MPK_ACKD) && consumeResult(RS_MPK_STAR) && consumeResult(RS_MPK_SENT))
            inf(DTR("restart request acknowledged"));
          else
            inf(DTR("failed to acknowledge restart request"));

          IMC::RestartSystem restart;
          dispatch(restart);
        }
        else if (value == c_code_abort)
        {
          war(DTR("acoustic abort detected"));
          m_abort.setDestination(getSystemId());
          dispatch(m_abort);

          std::string cmd = String::str("$CCMUC,%u,%u,%04x\r\n", m_addr, src, c_code_abort_ack);
          processInput(m_args.mpk_delay_bef);
          m_uart->write(cmd.c_str(), cmd.size());
          processInput(c_mpk_duration + m_args.mpk_delay_aft);

          if (consumeResult(RS_MPK_ACKD) && consumeResult(RS_MPK_STAR) && consumeResult(RS_MPK_SENT))
          {
            inf(DTR("abort acknowledged"));
          }
          else
          {
            inf(DTR("failed to acknowledge abort"));
          }
        }
      }

      void
      handleTransponderTravelTimes(std::auto_ptr<NMEAReader>& stn)
      {
        m_range.setTimeStamp();

        for (unsigned i = 0; i < Navigation::c_max_transponders; ++i)
        {
          try
          {
            double travel = 0;
            *stn >> travel;
            m_detect.tx = 0;
            m_detect.channel = m_beacons[i].tx_channel;
            m_detect.timer = (uint16_t)(Clock::get() * 1000);
            dispatch(m_detect);

            // Compute range and dispatch message.
            double range = travel * m_sound_speed;
            if (range > 0.0)
            {
              m_last_range = m_range.getTimeStamp();
              m_range.id = i;
              m_range.range = range;
              dispatch(m_range, DF_KEEP_TIME);

              // Update beacon statistics.
              m_beacons[i].range = (unsigned)m_range.range;
              m_beacons[i].range_time = m_last_range;
            }
            else
            {
              war(DTR("discarded invalid range %0.2f"), range);
            }
          }
          catch (...)
          { }
        }

        addResult(RS_PNG_TIME);
      }

      void
      processInput(double timeout = c_cmd_reply_tout)
      {
        double deadline = Clock::get() + timeout;

        while (Clock::get() <= deadline)
        {
          consumeMessages();

          if (!Poll::poll(*m_uart, 0.01))
            continue;

          m_uart->readString(m_bfr, c_bfr_size);
          m_last_input = Clock::get();

          if (m_state != STA_NO_BEACONS)
            m_state = isActive() ? STA_ACTIVE : STA_IDLE;

          m_cmds.value.assign(sanitize(m_bfr));
          dispatch(m_cmds);

          try
          {
            std::auto_ptr<NMEAReader> stn = std::auto_ptr<NMEAReader>(new NMEAReader(m_bfr));
            if (std::strcmp(stn->code(), "CAMUA") == 0)
              handleMiniPacket(stn);
            else if (std::strcmp(stn->code(), "SNTTA") == 0)
              handleTransponderTravelTimes(stn);
            else if (std::strcmp(stn->code(), "SNPNT") == 0)
              addResult(RS_PNG_ACKD);
            else if (std::strcmp(stn->code(), "CAMUC") == 0)
              addResult(RS_MPK_ACKD);
            else if (std::strcmp(stn->code(), "CATXP") == 0)
              addResult(RS_MPK_STAR);
            else if (std::strcmp(stn->code(), "CATXF") == 0)
              addResult(RS_MPK_SENT);
            else if (std::strcmp(stn->code(), "CACFG") == 0)
              handleConfigParam(stn);
          }
          catch (std::exception& e)
          {
            err("%s", e.what());
          }
        }
      }

      void
      ping(void)
      {
        std::vector<unsigned> freqs;
        for (unsigned i = 0; i < Navigation::c_max_transponders; ++i)
        {
          if (i < m_beacons.size())
            freqs.push_back(m_beacons[i].tx_frequency);
          else
            freqs.push_back(0);
        }

        std::string cmd = String::str("$CCPNT,%u,%u,%u,%u,%u,%u,%u,%u,1\r\n",
                                      m_beacons[0].rx_frequency, m_args.tx_length,
                                      m_args.rx_length, m_args.ping_tout,
                                      freqs[0], freqs[1], freqs[2], freqs[3]);

        m_uart->writeString(cmd.c_str());

        processInput(m_args.ping_period);
        if (consumeResult(RS_PNG_ACKD) && consumeResult(RS_PNG_TIME))
        {
          m_state = STA_ACTIVE;
        }
        else
        {
          war(DTR("failed to ping beacons, modem seems busy"));
        }
      }

      void
      sendVerboseReport(void)
      {
        double lat;
        double lon;
        Coordinates::toWGS84(m_estate, lat, lon);

        float f_lat = lat;
        float f_lon = lon;
        uint8_t u_depth = (uint8_t)m_estate.depth;
        int16_t i_yaw = (int16_t)(m_estate.psi * 100.0);
        int16_t i_alt = (int16_t)(m_estate.alt * 10.0);
        uint16_t ranges[2] = {0};
        uint8_t fuel = (uint8_t)m_fuel_level;
        uint8_t conf = (uint8_t)m_fuel_conf;
        int8_t prog = (int8_t)m_progress;

        if (m_beacons.size() == 2)
        {
          ranges[0] = m_beacons[0].range;
          ranges[1] = m_beacons[1].range;
        }

        std::vector<char> msg(32, 0);
        std::memcpy(&msg[0], &f_lat, 4);
        std::memcpy(&msg[4], &f_lon, 4);
        std::memcpy(&msg[8], &u_depth, 1);
        std::memcpy(&msg[9], &i_yaw, 2);
        std::memcpy(&msg[11], &i_alt, 2);
        std::memcpy(&msg[13], &ranges[0], 2);
        std::memcpy(&msg[15], &ranges[1], 2);
        std::memcpy(&msg[17], &prog, 1);
        std::memcpy(&msg[18], &fuel, 1);
        std::memcpy(&msg[19], &conf, 1);

        std::string hex = String::toHex(msg);
        std::string cmd = String::str("$CCTXD,%u,%u,0,%s\r\n",
                                      m_addr, 0, hex.c_str());
        m_uart->writeString(cmd.c_str());

        std::string cyc = String::str("$CCCYC,0,%u,%u,0,0,1\r\n",
                                      m_addr, 0);
        m_uart->writeString(cyc.c_str());

        int i = 0;
        for (i = 0; i < 7; ++i)
        {
          consumeMessages();
          Delay::wait(1.0);
        }
      }

      void
      consume(const IMC::LblConfig* msg)
      {
        if (msg->op == IMC::LblConfig::OP_SET_CFG)
        {
            m_beacons.clear();
            IMC::MessageList<IMC::LblBeacon>::const_iterator itr = msg->beacons.begin();
            for (unsigned i = 0; itr != msg->beacons.end(); ++itr, ++i)
            {
              if (*itr == NULL)
                continue;

              Beacon beacon;
              beacon.id = i;
              beacon.name = (*itr)->beacon;
              beacon.rx_channel = (*itr)->query_channel;
              beacon.rx_frequency = channelToFrequency((*itr)->query_channel);
              beacon.tx_channel = (*itr)->reply_channel;
              beacon.tx_frequency = channelToFrequency((*itr)->reply_channel);
              beacon.ping_cmd = String::str("$CCPNT,%u,%u,%u,%u,%u,0,0,0,1\r\n",
                                            beacon.rx_frequency, m_args.tx_length,
                                            m_args.rx_length, m_args.ping_tout,
                                            beacon.tx_frequency);
              beacon.lat = (*itr)->lat;
              beacon.lon = (*itr)->lon;
              beacon.depth = (*itr)->depth;
              beacon.delay = (*itr)->transponder_delay;

              m_beacons.push_back(beacon);
            }

            if (m_state != STA_ERR_COM && m_state != STA_ERR_SRC && m_state != STA_ERR_STP)
              m_state = isActive() ? STA_ACTIVE : STA_IDLE;
        }

        if (msg->op == IMC::LblConfig::OP_GET_CFG)
        {
          IMC::LblConfig cfg;
          cfg.op = IMC::LblConfig::OP_CUR_CFG;

          for (unsigned i = 0; i < m_beacons.size(); i++)
          {
            LblBeacon beacon;
            beacon.beacon = m_beacons[i].name;
            beacon.lat = m_beacons[i].lat;
            beacon.lon = m_beacons[i].lon;
            beacon.depth = m_beacons[i].depth;
            beacon.query_channel = m_beacons[i].rx_channel;
            beacon.reply_channel = m_beacons[i].tx_channel;
            beacon.transponder_delay = m_beacons[i].delay;

            cfg.beacons.push_back(beacon);
          }

          // Dispatch LblConfig to bus.
          dispatch(cfg);
        }
      }

      void
      consume(const IMC::QueryEntityState* msg)
      {
        (void)msg;
        onReportEntityState();
      }

      void
      consume(const IMC::SoundSpeed* msg)
      {
        if ((int)msg->getSourceEntity() != m_sound_speed_eid)
          return;

        if (msg->value < 0)
          return;

        m_sound_speed = msg->value;
      }

      void
      consume(const IMC::EstimatedState* msg)
      {
        m_estate = *msg;
      }

      void
      consume(const IMC::VehicleMedium* msg)
      {
        if (msg->medium == IMC::VehicleMedium::VM_GROUND)
	  m_stop_reports = true;
	else
	  m_stop_reports = false;
      }

      void
      consume(const IMC::PlanControlState* msg)
      {
        m_progress = msg->plan_progress;
      }

      void
      consume(const IMC::FuelLevel* msg)
      {
        m_fuel_level = msg->value;
        m_fuel_conf = msg->confidence;
      }

      void
      reportRanges(double now)
      {
        bool first = true;

        for (unsigned i = 0; i < m_beacons.size(); ++i)
        {
          if ((now - m_beacons[i].range_time) < m_args.good_range_age)
          {
            if (first)
            {
              processInput(m_args.report_delay_bef);
              first = false;
            }

            uint16_t code = 0x1000 | ((i & 0x03) << 10) | m_beacons[i].range;
            std::string code_str = String::str("%04X", code);
            NMEAWriter stn("CCMUC");
            stn << m_addr << 15 << code_str;
            std::string cmd = stn.sentence();

            processInput(m_args.mpk_delay_bef);
            m_uart->write(cmd.c_str(), cmd.size());
            processInput(c_mpk_duration + m_args.mpk_delay_aft);

            if (consumeResult(RS_MPK_ACKD) && consumeResult(RS_MPK_STAR) && consumeResult(RS_MPK_SENT))
            {
              inf(DTR("reported range to %s = %u m"), m_beacons[i].name.c_str(), m_beacons[i].range);
            }
            else
            {
              inf(DTR("failed to report range to %s"), m_beacons[i].name.c_str());
            }
          }
        }

        if (!first)
          processInput(m_args.report_delay_aft);
      }

      void
      onMain(void)
      {
        m_report_timer.reset();

        while (!stopping())
        {
          // Report.
          if (m_args.report && !m_stop_reports)
          {
            if (m_report_timer.overflow())
            {
              m_report_timer.reset();

              if (m_args.report_verbose)
                sendVerboseReport();
              else
                reportRanges(Clock::get());
            }
          }

          if (m_beacons.empty())
          {
            waitForMessages(1.0);
            processInput();
            continue;
          }

          if (Clock::get() >= (m_last_input + c_input_tout))
            m_state = STA_ERR_COM;

          if (isActive())
          {
            ping();
          }
          else
          {
            processInput();
          }
        }
      }
    };
  }
}

DUNE_TASK
