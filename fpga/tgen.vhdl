-----------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    09:20:06 01/26/2018 
-- Design Name: 
-- Module Name:    tgen - Behavioral 
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

--module to generate 4 bit vector T from the key
entity tgen is
    Port ( P : in  STD_LOGIC_VECTOR (31 downto 0);		--input word (might be encrypted or plain)
           K : in  STD_LOGIC_VECTOR (31 downto 0);		--key
           clock : in  STD_LOGIC;		
           enable : in  STD_LOGIC;
           reset : in  STD_LOGIC;
           T : out  STD_LOGIC_VECTOR (3 downto 0);		--output computed T
			  K1 : out STD_LOGIC_VECTOR (31 downto 0);		--below outputs are just mirrors of input
           C : out  STD_LOGIC_VECTOR (31 downto 0);		
           clock1 : out  STD_LOGIC;
           enable1 : out  STD_LOGIC;
           reset1 : out  STD_LOGIC);
end tgen;

architecture Behavioral of tgen is
	begin
		clock1 <= clock;			--mirror inputs to output ports
		enable1 <= enable;
		reset1 <= reset;
		C <= P;
		K1 <= K;
		--compute T bit by bit
		T(3) <= K(3) XOR K(7) XOR K(11) XOR K(15) XOR K(19) XOR K(23) XOR K(27) XOR K(31);
		T(2) <= K(2) XOR K(6) XOR K(10) XOR K(14) XOR K(18) XOR K(22) XOR K(26) XOR K(30);    
		T(1) <= K(1) XOR K(5) XOR K(9) XOR K(13) XOR K(17) XOR K(21) XOR K(25) XOR K(29);
		T(0) <= K(0) XOR K(4) XOR K(8) XOR K(12) XOR K(16) XOR K(20) XOR K(24) XOR K(28);		
end Behavioral;

