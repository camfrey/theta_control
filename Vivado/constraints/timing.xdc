set_clock_groups -physically_exclusive -group [get_clocks -include_generated_clocks sys_clk_pin] -group [get_clocks -include_generated_clocks sys_clock]
set_clock_groups -physically_exclusive -group [get_clocks -include_generated_clocks clkfbout_theta_design_clk_wiz_1_0] -group [get_clocks -include_generated_clocks clkfbout_theta_design_clk_wiz_1_0_1]
set_clock_groups -physically_exclusive -group [get_clocks -include_generated_clocks clk_100M_theta_design_clk_wiz_1_0] -group [get_clocks -include_generated_clocks clk_100M_theta_design_clk_wiz_1_0_1]
set_clock_groups -physically_exclusive -group [get_clocks -include_generated_clocks clk_PWM_theta_design_clk_wiz_1_0] -group [get_clocks -include_generated_clocks clk_PWM_theta_design_clk_wiz_1_0_1]
set_clock_groups -physically_exclusive -group [get_clocks -include_generated_clocks clk_SPI_theta_design_clk_wiz_1_0] -group [get_clocks -include_generated_clocks clk_SPI_theta_design_clk_wiz_1_0_1]

