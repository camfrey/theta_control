library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;


entity pwm_chan is
  generic(
    C_PWM_COUNTER_WIDTH : integer := 11
  );
  Port( 
    PWM_clk : in std_logic;
    PWM_reset : in std_logic;
    duty_cycle : in std_logic_vector(C_PWM_COUNTER_WIDTH - 1 downto 0);
    phase : in std_logic_vector(C_PWM_COUNTER_WIDTH -1 downto 0);
    PWM : out std_logic    
  );
end pwm_chan;

architecture Behavioral of pwm_chan is

signal duty_count : std_logic_vector(C_PWM_COUNTER_WIDTH - 1 downto 0);

begin

    process (PWM_clk)
    begin
        if rising_edge(PWM_clk) then
          if PWM_reset = '1' then
             duty_count <= (others => '0');
             PWM <= '0';
          else
            duty_count <= std_logic_vector(unsigned(duty_count) + 1);
--            if (duty_count < std_logic_vector(unsigned(duty_cycle) + unsigned(phase)) and duty_count > phase) then
            if (std_logic_vector(unsigned(duty_count) + unsigned(phase)) < duty_cycle) then
                PWM <= '1';
            else
                PWM <= '0';
            end if;
          end if;
        end if;
    end process;

end Behavioral;
