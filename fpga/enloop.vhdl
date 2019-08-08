
----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    11:00:24 01/26/2018 
-- Design Name: 
-- Module Name:    enloop - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;


-- module that runs a clocked process for running iterations 
entity enloop is
    Port ( P : in  STD_LOGIC_VECTOR (31 downto 0);				--the plain word
           K : in  STD_LOGIC_VECTOR (31 downto 0);				--the key
           T : in STD_LOGIC_VECTOR (3 downto 0);	 		-- 4 bit vector generated from K by tgen
			  clock : in  STD_LOGIC;
           enable : in  STD_LOGIC;
           reset : in  STD_LOGIC;
			  C : out  STD_LOGIC_VECTOR (31 downto 0):="00000000000000000000000000000000";		--encrypted word
			  done : out std_logic:='0');									--done signal
end enloop;

architecture Behavioral of enloop is
	signal tempC : std_logic_vector(31 downto 0):="00000000000000000000000000000000";														--signal to store intermediate results
	signal tempCont : std_logic_vector(31 downto 0) := "11111111111111111111111111111111";	 --control signal used to check bit positions of Key
	signal tempT : std_logic_vector(3 downto 0):="0000";															--stores intermediate values of T
begin
	process(enable,clock,reset)
		begin
		if(reset='1') then																			--fill everything with zeroes on a reset
				C <= "00000000000000000000000000000000";
				tempC <= "00000000000000000000000000000000";
				tempCont <= "11111111111111111111111111111111";
				done <= '0';
		elsif (clock'event and clock = '1' and enable = '1') then 												--computations to run on positive edge of clock
			if(tempCont="11111111111111111111111111111111") then							--initial assignments of intermediate signals
				tempT <= T;
				tempC <= P;
				tempCont <= "00000000000000000000000000000001";			
				done <= '0';
			elsif((K and tempCont) /= "00000000000000000000000000000000") then 						--checks if a bit of K is one
				tempC <= tempC xor (tempT & tempT & tempT & tempT & tempT & tempT & tempT & tempT);		--update C
				tempT <= std_logic_vector(to_unsigned(to_integer(unsigned( tempT )) + 1, 4));				--increase T by 1 and take only the 4 trailing bits
				tempCont <= std_logic_vector(unsigned(tempCont) sll 1);									--left shift by one bit
				C <= tempC;
			else 											--if bit is not one
				tempCont <= std_logic_vector(unsigned(tempCont) sll 1);				-- then just do a left shift without computations
				C <= tempC;
			end if;
			if (tempCont = "00000000000000000000000000000000") then
				done <= '1';
			end if;
		end if;
	end process;
end Behavioral;
