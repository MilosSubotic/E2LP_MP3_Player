-------------------------------------------------------------------------------
-- @file adau1772_i2s_ctrl.vhd
-- @date Apr 22, 2014
--
-- @author Milos Subotic <milos.subotic.sm@gmail.com>
-- @license MIT
--
-- @brief I2S controller for ADAU1772 on E2LP board.
--
-- @version: 1.0
-- Changelog:
-- 1.0 - Initial version.
--
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;


entity adau1772_i2s_ctrl is
	port (
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
		
		i_sample                  : in  std_logic_vector(23 downto 0);
		o_next_sample             : out std_logic 
	);
end entity adau1772_i2s_ctrl;

