
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library axi_adau1772_i2s_ctrl_v1_00_a;
use axi_adau1772_i2s_ctrl_v1_00_a.adau1772_i2s_ctrl;

entity testbench is
end testbench;

architecture adau1772_i2s_ctrl_tb_arch of testbench is


	constant clk_period              : time := 41.666666666666664 ns; -- 24MHz

	signal i_clk_24MHz               : std_logic;
	signal in_reset                  : std_logic := '0';
		
	signal o_audio_codec_lrclk       : std_logic;
	signal o_audio_codec_bclk        : std_logic;
	signal o_audio_codec_dac_sdata   : std_logic;
	signal i_audio_codec_adc_sdata0  : std_logic;
	signal i_audio_codec_adc_sdata1  : std_logic;
	signal o_audio_codec_dmic0_1     : std_logic;
	signal o_audio_codec_dmic2_3     : std_logic;
	signal on_audio_codec_power_down : std_logic;
		
	signal i_sample                  : std_logic_vector(23 downto 0) := x"dead01";
	signal o_next_sample             : std_logic;

begin

	uut: entity adau1772_i2s_ctrl
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
		
		i_sample                  => i_sample,
		o_next_sample             => o_next_sample
	);

	i_clk_24MHz_process: process
	begin
		i_clk_24MHz <= '1';
		wait for clk_period/2;
		i_clk_24MHz <= '0';
		wait for clk_period/2;
	end process i_clk_24MHz_process;


	tb: process
	begin
		wait for clk_period*10;
		in_reset <= '1';
		
	end process tb;

end architecture adau1772_i2s_ctrl_tb_arch;
