library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.numeric_std.ALL;

entity clock_div is
     Generic(
        C_NUM_DIVIDERS : integer := 3
      );
      Port (
             clk_in: in std_logic;
             clk_out: out std_logic
      );
end clock_div;

architecture Behavioral of clock_div is

signal clocks : unsigned(C_NUM_DIVIDERS - 1 downto 0);

begin

process (clk_in)
begin
    if rising_edge(clk_in) then
        clocks <= clocks + 1;
        clk_out <= clocks(C_NUM_DIVIDERS - 1);
    end if;
end process;

end Behavioral;
