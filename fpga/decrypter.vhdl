----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    19:21:08 01/26/2018 
-- Design Name: 
-- Module Name:    top - Behavioral 
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
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

--                         THIS MODULE ACTS AS A TOP MODULE WHICH CALLS TWO OTHER MODULES 
-- This entity is the DECRYPTER
entity decrypter is
	Port ( C : in  STD_LOGIC_VECTOR (31 downto 0);
			 K : in  STD_LOGIC_VECTOR (31 downto 0);
			 clock : in  STD_LOGIC;
			 enable : in  STD_LOGIC;
			 reset : in  STD_LOGIC;
			 P : out  STD_LOGIC_VECTOR (31 downto 0);
			 done : out std_logic);
end decrypter;

-- The architecture of the DECRYPTER begins here
architecture Behavioral of decrypter is
	component tgen 
	 Port ( P : in  STD_LOGIC_VECTOR (31 downto 0);
           K : in  STD_LOGIC_VECTOR (31 downto 0);
           clock : in  STD_LOGIC;
           enable : in  STD_LOGIC;
           reset : in  STD_LOGIC;
           C : out  STD_LOGIC_VECTOR (31 downto 0);
           T : out  STD_LOGIC_VECTOR (3 downto 0);
			  K1 : out STD_LOGIC_VECTOR (31 downto 0);
           clock1 : out  STD_LOGIC;
           enable1 : out  STD_LOGIC;
           reset1 : out  STD_LOGIC);
	end component;
	component deloop
	Port ( C : in  STD_LOGIC_VECTOR (31 downto 0);
           K : in  STD_LOGIC_VECTOR (31 downto 0);
           T : in STD_LOGIC_VECTOR (3 downto 0);
			  clock : in  STD_LOGIC;
           enable : in  STD_LOGIC;
           reset : in  STD_LOGIC;
			  P : out  STD_LOGIC_VECTOR (31 downto 0);
			  done : out std_logic);
	end component;
	
signal tempT :std_logic_vector(3 downto 0);
signal tempP : std_logic_vector(31 downto 0);
signal tempK : std_logic_vector(31 downto 0);
signal tempClock : std_logic;
signal tempEnable : std_logic;
signal tempReset : std_logic;
begin

-- Here we call two different modules tgen and deloop
-- tgen will generate the T vector 
-- deloop will generate the encoded code from the key and the text
TG : tgen port map(C, K, clock, enable,reset, tempP, tempT,tempK,tempClock,tempEnable, tempReset);
EL : deloop port map (tempP, tempK, tempT, tempClock, tempEnable, tempReset, P,done);
end Behavioral;