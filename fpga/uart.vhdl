library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity uart is
port (clk 	 : in std_logic;
		rst 	 : in std_logic;
		rx	 	 : in std_logic;
		tx	 	 : out std_logic;
		rxdone : out std_logic;
		txdone : out std_logic;
		out_rxdata : out std_logic_vector(7 downto 0); --20102016
		txdatain : in std_logic_vector(7 downto 0);
		wr_en : in std_logic;
		rd_en : in std_logic);
end uart;

architecture Structural of uart is

	component baudrate_gen is
	port (clk	: in std_logic;
			rst	: in std_logic;
			sample: out std_logic);
	end component baudrate_gen;
	signal sample : std_logic;
	
	component uart_rx is			
	port (clk	: in std_logic;
			rst	: in std_logic;
			rx		: in std_logic;
			sample: in STD_LOGIC;
			rxdone: out std_logic;
			rxdata: out std_logic_vector(7 downto 0));
	end component uart_rx;
	
	component uart_tx is			
	port (clk    : in std_logic;
			rst    : in std_logic;
			txstart: in std_logic;
			sample : in std_logic;
			txdata : in std_logic_vector(7 downto 0);
			txdone : out std_logic;
			tx	    : out std_logic);
	end component uart_tx;

	COMPONENT fifo
	PORT ( clk : IN STD_LOGIC;
			 srst : IN STD_LOGIC;
			 din : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
			 wr_en : IN STD_LOGIC;
			 rd_en : IN STD_LOGIC;
			 dout : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
			 full : OUT STD_LOGIC;
			 empty : OUT STD_LOGIC);
	END COMPONENT;
	signal full, empty : std_logic;
   signal din, dout : std_logic_vector(7 downto 0);
	signal flag : std_logic;
	-- rx and tx signals
	signal tx_start : std_logic := '0';
	signal rx_data : std_logic_vector(7 downto 0);
	signal tx_data : std_logic_vector(7 downto 0);
	signal rx_done : std_logic := '0';
	signal tx_done : std_logic := '0';

begin
	i_brg : baudrate_gen port map (clk => clk, rst => rst, sample => sample);
	
	i_rx : uart_rx port map( clk => clk, rst => rst,
                            rx => rx, sample => sample,
                            rxdone => rx_done, rxdata => rx_data);
									
	i_tx : uart_tx port map( clk => clk, rst => rst,
                            txstart => tx_start,
                            sample => sample, txdata => tx_data,
                            txdone => tx_done, tx => tx);
									 
	i_fifo : fifo PORT MAP ( clk => clk,
									 srst => rst,
									 din => din,
									 wr_en => wr_en,
									 rd_en => rd_en,
									 dout => dout,
									 full => full,
									 empty => empty);
									 
process(clk)
begin
	if (rising_edge(clk)) then
		rxdone <= rx_done;
		txdone <= tx_done;
	end if;
end process;


----loopback logic
	 p_wr : process(clk,rst,full,rx_done)
	 begin
			if rising_edge(clk) then
				if full = '0' then
					
					din <= rx_data;
					if rx_done = '1' then
	--					wr_en <= '1';
						out_rxdata <= rx_data;
					else
--						wr_en <= '0';
					end if;
				end if;		
			end if;
	end process p_wr;
	
-- standard procedure to generate 1 pulse	
--	p_flag : process(clk,rst,wr_en)
--	begin
--		if rst = '1' then
--			flag <= '0';
--		else
--			if rising_edge(clk) then
--				if wr_en = '1' then
--					flag <= '1';
--				else
--					flag <= '0';
--				end if;
--			end if;
--		end if;
--	end process p_flag;
	
--	rd_en <= '1' when (empty = '0' and tx_done = '1') else -- purely combinational logic
--				'0';
				
	p_rd : process(clk,rst,empty,tx_done,flag,txdatain)
	begin
------some data remains in FIFO. So empty = 0. Hence after long time full = 1. Also, seq+comb logic
--		if rst = '1' then
--			rd_en <= '0';
--			--dout <= (others => '0');
--		else
--			if rising_edge(clk) then
--				if empty = '0' then
--					if txdone = '1' then
--						if flag = '1' then
--							rd_en <= '1';
--						else
--							rd_en <= '0';
--						end if;
--					end if;
--				end if;
--			end if;
--		end if;

		tx_data <= txdatain;
	end process p_rd;
	
	p_TXstart : process(clk,rst,rd_en)
	begin
		if rst = '1' then
			tx_start <= '0';
		else	
			if rising_edge(clk) then
				if rd_en = '1' then
					tx_start <= '1';
				else
					tx_start <= '0';
				end if;
			end if;
		end if;
	end process p_TXstart;
		
end Structural;
