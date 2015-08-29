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
use ieee.numeric_std.all;

library axi_adau1772_i2s_ctrl_v1_00_a;
use axi_adau1772_i2s_ctrl_v1_00_a.adau1772_i2s_ctrl;

architecture adau1772_i2s_ctrl_arch_v1 of adau1772_i2s_ctrl is
	
	signal i2s_cnt       : unsigned(7 downto 0); -- [0, 250)
	signal lrclk         : std_logic;
	signal bclk_cnt      : unsigned(7 downto 3); -- [0, 25)
	
begin

   on_audio_codec_power_down <= '1';
	o_audio_codec_dmic0_1 <= '0';
	o_audio_codec_dmic2_3 <= '0';
	
	process(i_clk_24MHz, in_reset)
	begin
		if in_reset = '0' then
			i2s_cnt <= (others => '0');
		elsif rising_edge(i_clk_24MHz) then
			if i2s_cnt = 250-1 then
				i2s_cnt <= (others => '0');
			else
				i2s_cnt <= i2s_cnt + 1;
			end if;
		end if;
	end process;
	
	bclk_cnt <= i2s_cnt(7 downto 3);
	
	o_audio_codec_bclk <= i2s_cnt(2) when bclk_cnt < 25 else '0'; -- BCLK is 8 CLKs period.
	
	process(i_clk_24MHz, in_reset)
	begin
		if in_reset = '0' then
			lrclk <= '0';
		elsif rising_edge(i_clk_24MHz) then
			if i2s_cnt = 250-1 then
				lrclk <= not lrclk;
			end if;
		end if;
	end process;
	o_audio_codec_lrclk <= lrclk;
	
	o_audio_codec_dac_sdata <= i_sample(to_integer(24 - bclk_cnt)) when 0 < bclk_cnt and bclk_cnt <= 24 else '0';
	
	o_next_sample <= '1' when i2s_cnt = 250-1 and lrclk = '1' else '0';
	
end architecture adau1772_i2s_ctrl_arch_v1;
