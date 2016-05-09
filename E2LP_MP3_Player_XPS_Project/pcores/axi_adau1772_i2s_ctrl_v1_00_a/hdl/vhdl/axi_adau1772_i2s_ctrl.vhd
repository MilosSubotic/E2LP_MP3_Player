------------------------------------------------------------------------------
-- axi_adau1772_i2s_ctrl.vhd - entity/architecture pair
------------------------------------------------------------------------------
-- IMPORTANT:
-- DO NOT MODIFY THIS FILE EXCEPT IN THE DESIGNATED SECTIONS.
--
-- SEARCH FOR --USER TO DETERMINE WHERE CHANGES ARE ALLOWED.
--
-- TYPICALLY, THE ONLY ACCEPTABLE CHANGES INVOLVE ADDING NEW
-- PORTS AND GENERICS THAT GET PASSED THROUGH TO THE INSTANTIATION
-- OF THE USER_LOGIC ENTITY.
------------------------------------------------------------------------------
--
-- ***************************************************************************
-- ** Copyright (c) 1995-2012 Xilinx, Inc.  All rights reserved.            **
-- **                                                                       **
-- ** Xilinx, Inc.                                                          **
-- ** XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"         **
-- ** AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND       **
-- ** SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,        **
-- ** OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,        **
-- ** APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION           **
-- ** THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,     **
-- ** AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE      **
-- ** FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY              **
-- ** WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE               **
-- ** IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR        **
-- ** REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF       **
-- ** INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS       **
-- ** FOR A PARTICULAR PURPOSE.                                             **
-- **                                                                       **
-- ***************************************************************************
--
------------------------------------------------------------------------------
-- Filename:          axi_adau1772_i2s_ctrl.vhd
-- Version:           1.00.a
-- Description:       Top level design, instantiates library components and user logic.
-- Date:              Sat Aug 29 19:54:52 2015 (by Create and Import Peripheral Wizard)
-- VHDL Standard:     VHDL'93
------------------------------------------------------------------------------
-- Naming Conventions:
--   active low signals:                    "*_n"
--   clock signals:                         "clk", "clk_div#", "clk_#x"
--   reset signals:                         "rst", "rst_n"
--   generics:                              "C_*"
--   user defined types:                    "*_TYPE"
--   state machine next state:              "*_ns"
--   state machine current state:           "*_cs"
--   combinatorial signals:                 "*_com"
--   pipelined or register delay signals:   "*_d#"
--   counter signals:                       "*cnt*"
--   clock enable signals:                  "*_ce"
--   internal version of output port:       "*_i"
--   device pins:                           "*_pin"
--   ports:                                 "- Names begin with Uppercase"
--   processes:                             "*_PROCESS"
--   component instantiations:              "<ENTITY_>I_<#|FUNC>"
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library axi_adau1772_i2s_ctrl_v1_00_a;
use axi_adau1772_i2s_ctrl_v1_00_a.adau1772_i2s_ctrl;

------------------------------------------------------------------------------
-- Entity section
------------------------------------------------------------------------------
-- Definition of Generics:
--   C_S_AXI_DATA_WIDTH           -- AXI4LITE slave: Data width
--   C_S_AXI_ADDR_WIDTH           -- AXI4LITE slave: Address Width
--   C_S_AXI_MIN_SIZE             -- AXI4LITE slave: Min Size
--   C_USE_WSTRB                  -- AXI4LITE slave: Write Strobe
--   C_DPHASE_TIMEOUT             -- AXI4LITE slave: Data Phase Timeout
--   C_BASEADDR                   -- AXI4LITE slave: base address
--   C_HIGHADDR                   -- AXI4LITE slave: high address
--   C_FAMILY                     -- FPGA Family
--   C_NUM_REG                    -- Number of software accessible registers
--   C_NUM_MEM                    -- Number of address-ranges
--   C_SLV_AWIDTH                 -- Slave interface address bus width
--   C_SLV_DWIDTH                 -- Slave interface data bus width
--
-- Definition of Ports:
--   S_AXI_ACLK                   -- AXI4LITE slave: Clock 
--   S_AXI_ARESETN                -- AXI4LITE slave: Reset
--   S_AXI_AWADDR                 -- AXI4LITE slave: Write address
--   S_AXI_AWVALID                -- AXI4LITE slave: Write address valid
--   S_AXI_WDATA                  -- AXI4LITE slave: Write data
--   S_AXI_WSTRB                  -- AXI4LITE slave: Write strobe
--   S_AXI_WVALID                 -- AXI4LITE slave: Write data valid
--   S_AXI_BREADY                 -- AXI4LITE slave: Response ready
--   S_AXI_ARADDR                 -- AXI4LITE slave: Read address
--   S_AXI_ARVALID                -- AXI4LITE slave: Read address valid
--   S_AXI_RREADY                 -- AXI4LITE slave: Read data ready
--   S_AXI_ARREADY                -- AXI4LITE slave: read addres ready
--   S_AXI_RDATA                  -- AXI4LITE slave: Read data
--   S_AXI_RRESP                  -- AXI4LITE slave: Read data response
--   S_AXI_RVALID                 -- AXI4LITE slave: Read data valid
--   S_AXI_WREADY                 -- AXI4LITE slave: Write data ready
--   S_AXI_BRESP                  -- AXI4LITE slave: Response
--   S_AXI_BVALID                 -- AXI4LITE slave: Resonse valid
--   S_AXI_AWREADY                -- AXI4LITE slave: Wrte address ready
------------------------------------------------------------------------------

entity axi_adau1772_i2s_ctrl is
  generic
  (
    -- ADD USER GENERICS BELOW THIS LINE ---------------
    --USER generics added here
    -- ADD USER GENERICS ABOVE THIS LINE ---------------

    -- DO NOT EDIT BELOW THIS LINE ---------------------
    -- Bus protocol parameters, do not add to or delete
    C_S_AXI_DATA_WIDTH             : integer              := 32;
    C_S_AXI_ADDR_WIDTH             : integer              := 32;
    C_S_AXI_MIN_SIZE               : std_logic_vector     := X"000001FF";
    C_USE_WSTRB                    : integer              := 0;
    C_DPHASE_TIMEOUT               : integer              := 8;
    C_BASEADDR                     : std_logic_vector     := X"FFFFFFFF";
    C_HIGHADDR                     : std_logic_vector     := X"00000000";
    C_FAMILY                       : string               := "virtex6";
    C_NUM_REG                      : integer              := 1;
    C_NUM_MEM                      : integer              := 1;
    C_SLV_AWIDTH                   : integer              := 32;
    C_SLV_DWIDTH                   : integer              := 32
    -- DO NOT EDIT ABOVE THIS LINE ---------------------
  );
  port
  (
    -- ADD USER PORTS BELOW THIS LINE ------------------
      i_clk_24MHz               : in  std_logic;
      in_reset                  : in  std_logic;
		
		o_audio_codec_lrclk       : out std_logic;
		o_audio_codec_bclk        : out std_logic;
		o_audio_codec_dac_sdata   : out std_logic; -- Line out (green jack).
		i_audio_codec_adc_sdata0  : in  std_logic;
		i_audio_codec_adc_sdata1  : in  std_logic;
		o_audio_codec_dmic0_1     : out std_logic;
      o_audio_codec_dmic2_3     : out std_logic;
      on_audio_codec_power_down : out std_logic;
		
		o_sample_interrupt        : out std_logic;
    -- ADD USER PORTS ABOVE THIS LINE ------------------

    -- DO NOT EDIT BELOW THIS LINE ---------------------
    -- Bus protocol ports, do not add to or delete
    S_AXI_ACLK                     : in  std_logic;
    S_AXI_ARESETN                  : in  std_logic;
    S_AXI_AWADDR                   : in  std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
    S_AXI_AWVALID                  : in  std_logic;
    S_AXI_WDATA                    : in  std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
    S_AXI_WSTRB                    : in  std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
    S_AXI_WVALID                   : in  std_logic;
    S_AXI_BREADY                   : in  std_logic;
    S_AXI_ARADDR                   : in  std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
    S_AXI_ARVALID                  : in  std_logic;
    S_AXI_RREADY                   : in  std_logic;
    S_AXI_ARREADY                  : out std_logic;
    S_AXI_RDATA                    : out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
    S_AXI_RRESP                    : out std_logic_vector(1 downto 0);
    S_AXI_RVALID                   : out std_logic;
    S_AXI_WREADY                   : out std_logic;
    S_AXI_BRESP                    : out std_logic_vector(1 downto 0);
    S_AXI_BVALID                   : out std_logic;
    S_AXI_AWREADY                  : out std_logic
    -- DO NOT EDIT ABOVE THIS LINE ---------------------
  );

  attribute MAX_FANOUT : string;
  attribute SIGIS : string;
  attribute MAX_FANOUT of S_AXI_ACLK       : signal is "10000";
  attribute MAX_FANOUT of S_AXI_ARESETN       : signal is "10000";
  attribute SIGIS of S_AXI_ACLK       : signal is "Clk";
  attribute SIGIS of S_AXI_ARESETN       : signal is "Rst";
end entity axi_adau1772_i2s_ctrl;

------------------------------------------------------------------------------
-- Architecture section
------------------------------------------------------------------------------

architecture IMP of axi_adau1772_i2s_ctrl is
	constant BASE_ADDR : signed(C_S_AXI_ADDR_WIDTH-1 downto 0) := signed(C_BASEADDR);
	
	subtype t_addr is signed(C_S_AXI_ADDR_WIDTH-1 downto 2);
	subtype t_word is std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		
	type t_read_states is (WAIT_READ_ADDR, ASSERT_READ_DATA);
	signal r_read_state : t_read_states;
	
	signal accept_write    : std_logic;
	signal r_write_response : std_logic;
	
	signal local_write_addr : t_addr;
	
	
	signal r_sample    : t_word;
	signal sample      : std_logic_vector(23 downto 0);
	signal next_sample : std_logic;
begin

	-- Read transaction.

	read_fsm: process(S_AXI_ACLK)
	begin
		if rising_edge(S_AXI_ACLK) then
			if S_AXI_ARESETN = '0' then
				r_read_state <= WAIT_READ_ADDR;
			else
				case r_read_state is
					when WAIT_READ_ADDR =>
						if S_AXI_ARVALID = '1' then
							r_read_state <= ASSERT_READ_DATA;
						end if;
					when ASSERT_READ_DATA =>
						if S_AXI_RREADY = '1' then
							r_read_state <= WAIT_READ_ADDR;
						end if;
				end case;
			end if;
		end if;
	end process read_fsm;
	
	S_AXI_ARREADY <= '1' when r_read_state = WAIT_READ_ADDR else '0';
	
	S_AXI_RVALID  <= '1' when r_read_state = ASSERT_READ_DATA else '0';
	
	S_AXI_RRESP   <=  "00"; -- Always OK response.
	
	S_AXI_RDATA <= x"deadbeef";
	
	
	
	
	-- Write transaction.
	
	-- When both valid signals are asserted and response is not in progress
	-- then say valid to master, write data and give response.
	
	accept_write <= S_AXI_AWVALID and S_AXI_WVALID and not r_write_response;
	S_AXI_AWREADY <= accept_write;
	S_AXI_WREADY <= accept_write;
		
	write_regs: process(S_AXI_ACLK)
	begin
		if rising_edge(S_AXI_ACLK) then
			if S_AXI_ARESETN = '0' then
				--r_regs <= (others => (others => '0'));
				r_write_response <= '0';
			else
				if accept_write = '1' then
					r_write_response <= '1';
					case to_integer(local_write_addr) is
						when 0 =>
							for byte in 0 to C_S_AXI_DATA_WIDTH/8-1 loop
								if S_AXI_WSTRB(byte) = '1' then
									r_sample(8*byte+7 downto 8*byte)
										<= S_AXI_WDATA(8*byte+7 downto 8*byte);
								end if;
							end loop;
						when others =>
					end case;
				else
					if S_AXI_BREADY = '1' then
						r_write_response <= '0';
					end if;
				end if;
			end if;
		end if;
	end process write_regs;
	
	S_AXI_BRESP  <= "00"; -- Always OK response.
	S_AXI_BVALID <= r_write_response;	
	
	
	sample <= r_sample(23 downto 0);
	ctrl: entity adau1772_i2s_ctrl
	port map (
      i_clk_24MHz               => i_clk_24MHz,
      in_reset                  => in_reset,
		
		o_audio_codec_lrclk       => o_audio_codec_lrclk,
		o_audio_codec_bclk        => o_audio_codec_bclk,
		o_audio_codec_dac_sdata   => o_audio_codec_dac_sdata,
		i_audio_codec_adc_sdata0  => i_audio_codec_adc_sdata0,
		i_audio_codec_adc_sdata1  => i_audio_codec_adc_sdata1,
		o_audio_codec_dmic0_1     => o_audio_codec_dmic0_1,
      o_audio_codec_dmic2_3     => o_audio_codec_dmic2_3,
      on_audio_codec_power_down => on_audio_codec_power_down,
		
		i_sample                  => sample,
		o_next_sample             => next_sample
	);
	
	o_sample_interrupt <= next_sample;

end IMP;
