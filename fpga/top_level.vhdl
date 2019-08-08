--
-- Copyright (C) 2009-2012 Chris McClelland
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.
--
library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity top_level is
	port(
		-- FX2LP interface ---------------------------------------------------------------------------
		fx2Clk_in      : in    std_logic;                    -- 48MHz clock from FX2LP
		fx2Addr_out    : out   std_logic_vector(1 downto 0); -- select FIFO: "00" for EP2OUT, "10" for EP6IN
		fx2Data_io     : inout std_logic_vector(7 downto 0); -- 8-bit data to/from FX2LP

		-- When EP2OUT selected:
		fx2Read_out    : out   std_logic;                    -- asserted (active-low) when reading from FX2LP
		fx2OE_out      : out   std_logic;                    -- asserted (active-low) to tell FX2LP to drive bus
		fx2GotData_in  : in    std_logic;                    -- asserted (active-high) when FX2LP has data for us

		-- When EP6IN selected:
		fx2Write_out   : out   std_logic;                    -- asserted (active-low) when writing to FX2LP
		fx2GotRoom_in  : in    std_logic;                    -- asserted (active-high) when FX2LP has room for more data from us
		fx2PktEnd_out  : out   std_logic;                    -- asserted (active-low) when a host read needs to be committed early

		-- Onboard peripherals -----------------------------------------------------------------------
		anode_out      : out   std_logic_vector(3 downto 0); -- seven-segment display anodes (one for each digit)
		led_out        : out   std_logic_vector(7 downto 0); -- eight LEDs
		sw_in          : in    std_logic_vector(7 downto 0) ; -- eight switches
		
		uart_rx	: in std_logic;
		uart_tx	: out std_logic;
		
		up_button : in std_logic;
		down_button : in std_logic;
		right_button : in std_logic;
		left_button : in std_logic;
		reset_button : in std_logic
	);
end entity;

architecture structural of top_level is
	-- Channel read/write interface -----------------------------------------------------------------
	signal chanAddr  : std_logic_vector(6 downto 0);  -- the selected channel (0-127)

	-- Host >> FPGA pipe:
	signal h2fData   : std_logic_vector(7 downto 0);  -- data lines used when the host writes to a channel
	signal h2fValid  : std_logic;                     -- '1' means "on the next clock rising edge, please accept the data on h2fData"
	signal h2fReady  : std_logic;                     -- channel logic can drive this low to say "I'm not ready for more data yet"

	-- Host << FPGA pipe:
	signal f2hData   : std_logic_vector(7 downto 0);  -- data lines used when the host reads from a channel
	signal f2hValid  : std_logic;                     -- channel logic can drive this low to say "I don't have data ready for you"
	signal f2hReady  : std_logic;                     -- '1' means "on the next clock rising edge, put your next byte of data on f2hData"
	-- ----------------------------------------------------------------------------------------------

	-- Needed so that the comm_fpga_fx2 module can drive both fx2Read_out and fx2OE_out
	signal fx2Read   : std_logic;

	-- Reset signal so host can delay startup
	signal fx2Reset  : std_logic;
	component decrypter
	Port ( C : in  STD_LOGIC_VECTOR (31 downto 0);
			 K : in  STD_LOGIC_VECTOR (31 downto 0);
			 clock : in  STD_LOGIC;
			 enable : in  STD_LOGIC;
			 reset : in  STD_LOGIC;
			 P : out  STD_LOGIC_VECTOR (31 downto 0);
			 done : out std_logic);
	end component;

	component encrypter
	Port ( P : in  STD_LOGIC_VECTOR (31 downto 0);
			 K : in  STD_LOGIC_VECTOR (31 downto 0);
			 clock : in  STD_LOGIC;
			 enable : in  STD_LOGIC;
			 reset : in  STD_LOGIC;
			 C : out  STD_LOGIC_VECTOR (31 downto 0);
			 done : out std_logic);
	 end component;
	 signal output_next: std_logic_vector(7 downto 0);
	 
	 component basic_uart 
		 generic (DIVISOR: natural);
	  port (
		 clk: in std_logic;                         -- clock
		 reset: in std_logic;                       -- reset
		 
		 -- Client interface
		 rx_data: out std_logic_vector(7 downto 0); -- received byte
		 rx_enable: out std_logic;                  -- validates received byte (1 system clock spike)
		 tx_data: in std_logic_vector(7 downto 0);  -- byte to send
		 tx_enable: in std_logic;                   -- validates byte to send if tx_ready is '1'
		 tx_ready: out std_logic;                   -- if '1', we can send a new byte, otherwise we won't take it
		 
		 -- Physical interface
		 rx: in std_logic;
		 tx: out std_logic
	  );
	  end component;
--
signal main_state : std_logic_vector(2 downto 0) := "001";
signal button_state : std_logic_vector(1 downto 0) ;
signal three_sec : std_logic := '1';
signal flag_2 : std_logic ;
signal rx_data_temp : std_logic_vector ( 7 downto 0);
--
signal inputs : std_logic_vector(63 downto 0);
signal tohost : std_logic_vector(31 downto 0);
signal fromhost : std_logic_vector(31 downto 0);
signal etohost : std_logic_vector(31 downto 0);
signal dfromhost : std_logic_vector(31 downto 0);
signal encryptdone : std_logic ;
signal decryptdone : std_logic ;
signal enencrypt : std_logic ;
signal endecrypt : std_logic ;
signal enreset : std_logic ;
signal dereset : std_logic ;
signal count_o : std_logic_vector(3 downto 0);
signal count : std_logic_vector(2 downto 0) ;
signal display : std_logic;
signal state : std_logic_vector(3 downto 0) ;
signal count_cycles : std_logic_vector(33 downto 0) := "0000000000000000000000000000000000" ;
signal chan_out : std_logic_vector(6 downto 0);
signal chan_in : std_logic_vector(6 downto 0);
--constants
constant i :std_logic_vector(6 downto 0) := "0000001";
constant coordinates : std_logic_vector(31 downto 0) := "00000000000000000000000000010011";
constant ack1 : std_logic_vector(31 downto 0) := "01000101111100100010010001001111";
constant ack2 : std_logic_vector(31 downto 0) := "00101010100101010010010001010100";
constant key : std_logic_vector(31 downto 0) := "10010101011001001000101110000010";
--uart signals
signal uart_rx_data : std_logic_vector(7 downto 0);
signal uart_tx_data : std_logic_vector(7 downto 0);
signal uart_rx_enable: std_logic;
signal uart_tx_enable: std_logic;
signal uart_tx_ready: std_logic;
signal emitting :std_logic;
signal rx_flag :std_logic := '0';

begin
	basic_uart_inst: basic_uart
	  generic map (DIVISOR => 1250) -- 2400bps @ 48Mhz
	  port map (
		 clk => fx2Clk_in, reset => reset_button,
		 rx_data => uart_rx_data, rx_enable => uart_rx_enable,
		 tx_data => uart_tx_data, tx_enable => uart_tx_enable, tx_ready => uart_tx_ready,
		 rx => uart_rx,
		 tx => uart_tx
	  );
	-- CommFPGA module
	fx2Read_out <= fx2Read;
	fx2OE_out <= fx2Read;
	fx2Addr_out(0) <=  -- So fx2Addr_out(1)='0' selects EP2OUT, fx2Addr_out(1)='1' selects EP6IN
		'0' when fx2Reset = '0'
		else 'Z';
	comm_fpga_fx2 : entity work.comm_fpga_fx2
		port map(
			clk_in         => fx2Clk_in,
			reset_in       => '0',
			reset_out      => fx2Reset,
			
			-- FX2LP interface
			fx2FifoSel_out => fx2Addr_out(1),
			fx2Data_io     => fx2Data_io,
			fx2Read_out    => fx2Read,
			fx2GotData_in  => fx2GotData_in,
			fx2Write_out   => fx2Write_out,
			fx2GotRoom_in  => fx2GotRoom_in,
			fx2PktEnd_out  => fx2PktEnd_out,

			-- DVR interface -> Connects to application module
			chanAddr_out   => chanAddr,
			h2fData_out    => h2fData,
			h2fValid_out   => h2fValid,
			h2fReady_in    => h2fReady,
			f2hData_in     => f2hData,
			f2hValid_in    => f2hValid,
			f2hReady_out   => f2hReady
		);
	
	encrypt : encrypter
	port map (tohost,   key, fx2Clk_in, enencrypt, enreset, etohost, encryptdone);	
	decrypt : decrypter
	port map (fromhost, key, fx2Clk_in, endecrypt, dereset, dfromhost, decryptdone);
	
	process(fx2Clk_in,reset_button)
	begin
		if(reset_button = '1') then
			main_state <= "001";
			three_sec <= '1';
			count_cycles <= "0000000000000000000000000000000000";
			rx_flag <= '0';
		else
			if (rising_edge(fx2Clk_in)) then
				if(uart_rx_enable = '1') then
					rx_flag <= '1';
					rx_data_temp <= uart_rx_data;
				end if;
				if (main_state  = "001") then
						-- 3 seconds
					if (count_cycles /= "0000001000100101010100010000000000") then
						count_cycles <= std_logic_vector(unsigned(count_cycles)+1);
						three_sec <= '1';
					else
						count_cycles <= "0000000000000000000000000000000000";
						three_sec <= '0';
						main_state <= "010";
						state <= "1111";
					end if;
				elsif (main_state = "010") then
					--state to initialise signals
					if(state = "1111") then 
						uart_tx_enable <= '0';
						three_sec <= '0';
						button_state <= "00";
						flag_2 <= '0';
						display <= '0';
						enencrypt <= '0';
						endecrypt <= '0';
						enreset <= '0';
						dereset <= '0';
						count_o <= "0000";
						count <= "000";
						state <= "0000";
						count_cycles<= "0000000000000000000000000000000000";
						output_next(2 downto 0) <= "000";
						chan_out <= std_logic_vector(unsigned(i) sll 1);
						chan_in <= std_logic_vector((unsigned(i) sll 1) + 1);
					--send encrypted coordinates
					elsif (state = "0000") then
						tohost <= coordinates;
						enencrypt <= '1';
						enreset <= '0';
						if (encryptdone  = '1') then
							if(to_integer(unsigned(count)) <= 3) then
								if(chanAddr = chan_out and f2hReady='1') then
									f2hData <= etohost(8*(3 - to_integer(unsigned(count)))+7 downto 8*(3 - to_integer(unsigned(count))));
									count <= std_logic_vector(unsigned(count)+1);
								end if;
							else
								enencrypt <= '0';
								state <= "0001";
								count <= "000";
								enreset<='1';
							end if;
						end if;
					--recieve encrypted coordinates
					elsif (state = "0001") then
						--256 second timeout
						if (to_integer(unsigned(count)) <= 3 and count_cycles /= "1011011100011011000000000000000000") then
							count_cycles <=  std_logic_vector(unsigned(count_cycles)+1);
							if (chanAddr = chan_in and h2fValid = '1') then
								fromhost(8*(3 - to_integer(unsigned(count)))+7 downto 8*(3 - to_integer(unsigned(count)))) <= h2fData;
								count <= std_logic_vector(unsigned(count)+1);							
							end if;
						elsif (count_cycles = "1011011100011011000000000000000000") then
							count_cycles <= "0000000000000000000000000000000000";
							state <= "0000";
							count <= "000";
						else
							endecrypt <= '1';
							dereset <= '0';
							if (decryptdone = '1') then
								count_cycles <= "0000000000000000000000000000000000";
								count <= "000";
								endecrypt <= '0';
								dereset <='1';
								if (coordinates = dfromhost) then
									state <= "0010";
								else		--go back to initial state if wrong
									state <= "0000";
								end if;
							end if;
						end if;
					--send encrpyted ack1
					elsif (state = "0010") then
						tohost <= ack1;
						enencrypt <= '1';
						enreset <='0';
						if (encryptdone  = '1') then
							if(to_integer(unsigned(count)) <= 3) then
								if(chanAddr = chan_out and f2hReady='1') then
									f2hData <= etohost(8*(3 - to_integer(unsigned(count)))+7 downto 8*(3 - to_integer(unsigned(count))));
									count <= std_logic_vector(unsigned(count)+1);
								end if;
							else
								enencrypt <= '0';
								state <= "0011";
								count <= "000";
								enreset <='1';
							end if;
						end if;
					--recieve encrypted ack2
					elsif (state = "0011") then
						--256 second timeout
						if (to_integer(unsigned(count)) <= 3 and count_cycles /= "1011011100011011000000000000000000") then
							count_cycles <=  std_logic_vector(unsigned(count_cycles)+1);
							if (chanAddr = chan_in and h2fValid = '1') then
								fromhost(8*(3 - to_integer(unsigned(count)))+7 downto 8*(3 - to_integer(unsigned(count)))) <= h2fData;
								count <= std_logic_vector(unsigned(count)+1);
							end if;
						elsif (count_cycles = "1011011100011011000000000000000000") then
							count_cycles <= "0000000000000000000000000000000000";
							state <= "0000";
							count <= "000";
						else
							dereset<='0';
							endecrypt <= '1';
							if (decryptdone = '1') then
								count_cycles <= "0000000000000000000000000000000000";
								count <= "000";
								endecrypt <= '0';
								dereset <='1';
								if (ack2 = dfromhost) then 	
									state <= "0100";
								else		--go back to initial state if wrong
									state <= "0000";
								end if;
							end if;
						end if;
					--recieve encrpyted 4 bytes of data
					elsif (state = "0100") then
						if (to_integer(unsigned(count)) <= 3) then
							if (chanAddr = chan_in and h2fValid = '1') then
								fromhost(8*(3 - to_integer(unsigned(count)))+7 downto 8*(3 - to_integer(unsigned(count)))) <= h2fData;
								count <= std_logic_vector(unsigned(count)+1);
							end if;
						else
							dereset <='0';
							endecrypt <= '1';
							if (decryptdone = '1') then
								inputs(63 downto 32) <= dfromhost;
								state <= "0101";
								count <= "000";
								endecrypt <= '0';
								dereset <='1';
							end if;
						end if;
					--send encrypted ack1
					elsif (state = "0101") then
						tohost <= ack1;
						enencrypt <= '1';
						enreset <='0';
						if (encryptdone  = '1') then
							if(to_integer(unsigned(count)) <= 3) then
								if(chanAddr = chan_out and f2hReady='1')  then
									f2hData <= etohost(8*(3 - to_integer(unsigned(count)))+7 downto 8*(3 - to_integer(unsigned(count))));
									count <= std_logic_vector(unsigned(count)+1);
								end if;
							else
								enencrypt <= '0';
								state <= "0110";
								enreset<='1';
								count <= "000";
							end if;
						end if;
					--recieve encrypted 4 bytes of data
					elsif (state = "0110") then
						if (to_integer(unsigned(count)) <= 3) then
							if (chanAddr = chan_in and h2fValid = '1') then
								fromhost(8*(3 - to_integer(unsigned(count)))+7 downto 8*(3 - to_integer(unsigned(count)))) <= h2fData;
								count <= std_logic_vector(unsigned(count)+1);
							end if;
						else
							endecrypt <= '1';
							dereset <='0';
							if (decryptdone = '1') then
								inputs(31 downto 0) <= dfromhost;
								state <= "0111";
								count <= "000";
								dereset <='1';
								endecrypt <= '0';
							end if;
						end if;
					--send encrypted ack1
					elsif (state = "0111") then
						tohost <= ack1;
						enencrypt <= '1';
						enreset <='0';
						if (encryptdone  = '1') then
							if(to_integer(unsigned(count)) <= 3) then
								if(chanAddr = chan_out and f2hReady='1') then
									f2hData <= etohost(8*(3 - to_integer(unsigned(count)))+7 downto 8*(3 - to_integer(unsigned(count))));
									count <= std_logic_vector(unsigned(count)+1);
								end if;
							else
								enencrypt <= '0';
								state <= "1000";
								enreset <='1';
								count <= "000";
							end if;
						end if;
					--recieve encrypted ack2
					elsif (state = "1000") then
						--256 second timeout
						if (to_integer(unsigned(count)) <= 3 and count_cycles /= "1011011100011011000000000000000000") then
							count_cycles <=  std_logic_vector(unsigned(count_cycles)+1);
							if (chanAddr = chan_in and h2fValid = '1') then
								fromhost(8*(3 - to_integer(unsigned(count)))+7 downto 8*(3 - to_integer(unsigned(count)))) <= h2fData;
								count <= std_logic_vector(unsigned(count)+1);
							end if;
						elsif (count_cycles = "1011011100011011000000000000000000") then
							count_cycles <= "0000000000000000000000000000000000";
							state <= "0000";
							count <= "000";
						else
							dereset <='0';
							endecrypt <= '1';
							if (decryptdone = '1') then
									count <= "000";
									dereset <='1';
									endecrypt <= '0';
								if (ack2 = dfromhost) then 	
									state <= "1001";
									count_cycles <= "0000001000100101010100010000000000"; --3 seconds
									if (rx_flag = '1') then	
										if(inputs(7+8*(7-to_integer(unsigned(rx_data_temp(5 downto 3))))) = '1' and rx_data_temp(7) = '1') then 
											if(inputs(6+(8*(7-to_integer(unsigned(rx_data_temp(5 downto 3)))))) = '1' and rx_data_temp(6) = '1') then 
												inputs(7+(8*(7-to_integer(unsigned(rx_data_temp(5 downto 3))))) downto 
																	(8*(7-to_integer(unsigned(rx_data_temp(5 downto 3)))))) <= rx_data_temp;
											else
												inputs(6+8*(7-to_integer(unsigned(rx_data_temp(5 downto 3))))) <= '0';
											end if;
										end if;
									end if;
								else		--go back to initial state if wrong
									count_cycles <= "0000000000000000000000000000000000";
									state <= "0000";
								end if;
							end if;
						end if;
					--display LED outputs
					elsif (state = "1001") then 
						if (to_integer(unsigned(count_o)) <= 7) then
							-- 3 seconds
							display <= '1';
							if(count_cycles = "0000001000100101010100010000000000") then 
								count_cycles <= "0000000000000000000000000000000000";
								count_o <= std_logic_vector(unsigned(count_o) + 1);
								--if track does not exist or exist but is NOT OK
								if((inputs(8*(7-to_integer(unsigned(count_o)))+7) = '0') 
										or ((inputs(8*(7-to_integer(unsigned(count_o)))+7) = '1') 
											and (inputs(8*(7-to_integer(unsigned(count_o)))+6) = '0'))) then
									output_next(0) <= '1';
									output_next(1) <= '0';
									output_next(2) <= '0';				
								--if there is no train				
								elsif(sw_in(to_integer(unsigned(count_o))) = '0') then
									output_next(0) <= '1';	
									output_next(1) <= '0';
									output_next(2) <= '0';	
								--if there is train here but not on the opposite track and next signal is 1 stop away 
								elsif(sw_in(to_integer(unsigned(count_o))) = '1' 
											and sw_in(to_integer(unsigned(std_logic_vector(unsigned(count_o)+4)))) = '0' 
												and inputs((8*(7-to_integer(unsigned(count_o)))+2) downto 8*(7-to_integer(unsigned(count_o)))) ="001") then
									output_next(0) <= '0';	
									output_next(1) <= '1';
									output_next(2) <= '0';
								--if there is train here but not on the opposite track and next signal is more than 1 stop away 
								elsif(sw_in(to_integer(unsigned(count_o))) = '1' 
											and sw_in(to_integer(unsigned(std_logic_vector(unsigned(count_o)+4)))) = '0' 
												and inputs((8*(7-to_integer(unsigned(count_o)))+2) downto 8*(7-to_integer(unsigned(count_o)))) /="001") then
									output_next(0) <= '0';	
									output_next(1) <= '0';
									output_next(2) <= '1';
								--if there is train here and on the opposite track but this direction is smaller
								elsif(sw_in(to_integer(unsigned(count_o))) = '1' 
											and sw_in(to_integer(unsigned(std_logic_vector(unsigned(count_o)+4)))) = '1' 	
												and to_integer(unsigned(count_o)) <= 3) then
									output_next(0) <= '1';	
									output_next(1) <= '0';
									output_next(2) <= '0';
								--if there is train here and on the opposite track and this direction is higher
								elsif(sw_in(to_integer(unsigned(count_o))) = '1' 
											and sw_in(to_integer(unsigned(std_logic_vector(unsigned(count_o)+4)))) = '1' 	
												and to_integer(unsigned(count_o)) > 3) then
									output_next(0) <= '0';	
									output_next(1) <= '0';
									output_next(2) <= '1';								
								end if;
							--1 second
							elsif (count_cycles = "0000000010110111000110110000000000" 
										and sw_in(to_integer(unsigned(count_o))-1) = '1' 
											and sw_in(to_integer(unsigned(std_logic_vector(unsigned(count_o)+3)))) = '1' 
												and to_integer(unsigned(count_o)) > 4 and 
													inputs(8*(8-to_integer(unsigned(count_o)))+7) = '1') then
								count_cycles <= std_logic_vector(unsigned(count_cycles)+1);
								output_next(0) <= '0';	
								output_next(1) <= '1';
								output_next(2) <= '0';								
							--2 seconds
							elsif (count_cycles = "0000000101101110001101100000000000" 
										and sw_in(to_integer(unsigned(count_o))-1) = '1'	
											and sw_in(to_integer(unsigned(std_logic_vector(unsigned(count_o)+3)))) = '1'
												and to_integer(unsigned(count_o)) > 4 and 
													inputs(8*(8-to_integer(unsigned(count_o)))+7) = '1') then
								count_cycles <= std_logic_vector(unsigned(count_cycles)+1);
								output_next(0) <= '1';	
								output_next(1) <= '0';
								output_next(2) <= '0';								
							else 
								count_cycles <= std_logic_vector(unsigned(count_cycles)+1);
							end if;
						else
							--1 second
							if (count_cycles = "0000000010110111000110110000000000" 
										and sw_in(7) = '1' and sw_in(3) = '1' and 
											inputs(7) = '1') then
								count_cycles <= std_logic_vector(unsigned(count_cycles)+1);
								output_next(0) <= '0';	
								output_next(1) <= '1';
								output_next(2) <= '0';								
							--2 seconds
							elsif (count_cycles = "0000000101101110001101100000000000" 
										and sw_in(7) = '1' and sw_in(3) = '1' and 
											inputs(7) = '1') then
								count_cycles <= std_logic_vector(unsigned(count_cycles)+1);
								output_next(0) <= '1';	
								output_next(1) <= '0';
								output_next(2) <= '0';	
							--3 seconds							
							elsif(count_cycles = "0000001000100101010100010000000000") then
								main_state <= "011";
								display <= '0';
								enencrypt <= '0';
								endecrypt <= '0';
								enreset <= '0';
								dereset <= '0';
								count_o <= "0000";
								count <= "000";
								count_cycles<= "0000000000000000000000000000000000";
								output_next(2 downto 0) <= "000";
								chan_out <= std_logic_vector(unsigned(i) sll 1);
								chan_in <= std_logic_vector((unsigned(i) sll 1) + 1);
							else
								count_cycles <= std_logic_vector(unsigned(count_cycles)+1);
							end if;
						end if;
					end if;
				elsif (main_state = "011") then
					if (up_button = '1' or button_state = "01") then
						button_state <= "01";
					else 
						button_state <= "11";
					end if;
					if (button_state = "01") then
						if (down_button = '1' or flag_2 /= '0') then
							tohost(31 downto 8) <= "000000000000000000000000";
							tohost(7 downto 0) <= sw_in;
							enencrypt <= '1';
							enreset <='0';
							flag_2 <= '1';
							if(to_integer(unsigned(count)) <= 3) then
								if(chanAddr = chan_out and f2hReady='1') then
									f2hData <= etohost(8*(3 - to_integer(unsigned(count)))+7 downto 8*(3 - to_integer(unsigned(count))));
									count <= std_logic_vector(unsigned(count)+1);
								end if;
							else
								enencrypt <= '0';						
								enreset<='1';
								count <= "000";
								main_state <= "100";
								button_state <= "00";
								flag_2 <= '0';
							end if;
						end if;
					elsif (button_state = "11") then
						main_state <= "100";
						button_state <= "00";
						flag_2 <= '0';
					end if;
				elsif (main_state = "100") then
					if (left_button = '1' or button_state = "01") then
						button_state <= "01";
						uart_tx_data <= sw_in;
					else
						button_state <= "11";
					end if;
					if (button_state = "01") then 
						if (right_button = '1' or flag_2 /= '0') then
							flag_2 <= '1';
							if (uart_tx_ready = '1') then
								uart_tx_enable <= '1';
								emitting <= '1';
							end if;
							if (emitting = '1') then
								if(uart_tx_ready = '0') then 
									emitting <= '0';
									main_state <="101";
									uart_tx_enable <='0';
									button_state <= "00";
									flag_2 <= '0';
								end if;
							end if;
						end if;
					elsif (button_state = "11") then 
						main_state <= "101";
						button_state <= "00";
						flag_2 <= '0';
					end if;
				elsif (main_state = "101") then
					main_state <= "110";
				elsif (main_state = "110") then
					--42 seconds
					if (count_cycles /= "0001111000001010011011100000000000") then
						count_cycles <= std_logic_vector(unsigned(count_cycles)+1);
					else
						count_cycles <= "0000000000000000000000000000000000";
						main_state <= "010";
						state <= "1111";
					end if;
				end if;
			end if;
		end if;
	end process;
	
	output_next(3) <= '0';
	output_next(4) <= '0';
	output_next(5) <= inputs(8*(8-to_integer(unsigned(count_o)))+3) when display='1' else '0' ;
	output_next(6) <= inputs(8*(8-to_integer(unsigned(count_o)))+4) when display='1' else '0';
	output_next(7) <= inputs(8*(8-to_integer(unsigned(count_o)))+5) when display='1' else '0';
	
	

	-- Assert that there's always data for reading, and always room for writing
	f2hValid <= '1' ;
	h2fReady <= '1';                                                     

	-- LEDs 	
		led_out <= output_next when three_sec = '0' else "11111111";
end architecture;
