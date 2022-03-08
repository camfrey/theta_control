library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

Library xpm;
use xpm.vcomponents.all;

library theta_control_lib;
use theta_control_lib.theta_control_pkg.all;

entity pwm_gen_v1_0 is
	generic (
		-- Users to add parameters name
		C_PWM_COUNTER_WIDTH : integer := 11;
		C_NUM_CHANNELS : natural := 8;
		-- User parameters ends
		-- Do not modify the parameters beyond this line


		-- Parameters of Axi Slave Bus Interface S_AXI
		C_S_AXI_DATA_WIDTH	: integer	:= 32;
		C_S_AXI_ADDR_WIDTH	: integer	:= 7;

		-- Parameters of Axi Slave Bus Interface S_AXIS
		C_S_AXIS_TDATA_WIDTH	: integer	:= 16
	);
	port (
		-- Users to add ports here
		PWM_clk : in std_logic;
		PWM : out std_logic_vector(C_NUM_CHANNELS -1 downto 0);
		PWM_reset : in std_logic;
		
--		phase0 : out std_logic_vector(C_PWM_COUNTER_WIDTH - 1 downto 0);
--		phase1 : out std_logic_vector(C_PWM_COUNTER_WIDTH - 1 downto 0);
--		phase2 : out std_logic_vector(C_PWM_COUNTER_WIDTH - 1 downto 0);
--		phase3 : out std_logic_vector(C_PWM_COUNTER_WIDTH - 1 downto 0);
--		phase4 : out std_logic_vector(C_PWM_COUNTER_WIDTH - 1 downto 0);
--		phase5 : out std_logic_vector(C_PWM_COUNTER_WIDTH - 1 downto 0);
--		phase6 : out std_logic_vector(C_PWM_COUNTER_WIDTH - 1 downto 0);
--		phase7 : out std_logic_vector(C_PWM_COUNTER_WIDTH - 1 downto 0);
		
		-- User ports ends
		-- Do not modify the ports beyond this line


		-- Ports of Axi Slave Bus Interface S_AXI
		s_axi_aclk	: in std_logic;
		s_axi_aresetn	: in std_logic;
		s_axi_awaddr	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		s_axi_awprot	: in std_logic_vector(2 downto 0);
		s_axi_awvalid	: in std_logic;
		s_axi_awready	: out std_logic;
		s_axi_wdata	: in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		s_axi_wstrb	: in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
		s_axi_wvalid	: in std_logic;
		s_axi_wready	: out std_logic;
		s_axi_bresp	: out std_logic_vector(1 downto 0);
		s_axi_bvalid	: out std_logic;
		s_axi_bready	: in std_logic;
		s_axi_araddr	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		s_axi_arprot	: in std_logic_vector(2 downto 0);
		s_axi_arvalid	: in std_logic;
		s_axi_arready	: out std_logic;
		s_axi_rdata	: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		s_axi_rresp	: out std_logic_vector(1 downto 0);
		s_axi_rvalid	: out std_logic;
		s_axi_rready	: in std_logic;

		-- Ports of Axi Slave Bus Interface S_AXIS
		s_axis_aclk	: in std_logic;
		s_axis_aresetn	: in std_logic;
		s_axis_tready	: out std_logic;
		s_axis_tdata	: in std_logic_vector(C_S_AXIS_TDATA_WIDTH-1 downto 0);
		s_axis_tstrb	: in std_logic_vector((C_S_AXIS_TDATA_WIDTH/8)-1 downto 0);
		s_axis_tlast	: in std_logic;
		s_axis_tvalid	: in std_logic
	);
end pwm_gen_v1_0;

architecture arch_imp of pwm_gen_v1_0 is

    signal duty_cycle_AXI_CLK : std_logic_vector(C_S_AXIS_TDATA_WIDTH - 1 downto 0);
    signal duty_cycle : std_logic_vector(C_S_AXIS_TDATA_WIDTH - 1 downto 0);
    signal duty_count : std_logic_vector(C_PWM_COUNTER_WIDTH - 1 downto 0);
    
    signal phase : phase_arr(C_NUM_CHANNELS - 1 downto 0);
    signal phase_sync : phase_arr(C_NUM_CHANNELS - 1 downto 0);

	-- component declaration
	component pwm_gen_v1_0_S_AXI is
		generic (
		C_S_AXI_DATA_WIDTH	: integer	:= 32;
		C_S_AXI_ADDR_WIDTH	: integer	:= 7
		);
		port (
        phase_array : out phase_arr(C_NUM_CHANNELS -1 downto 0);
		S_AXI_ACLK	: in std_logic;
		S_AXI_ARESETN	: in std_logic;
		S_AXI_AWADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_AWPROT	: in std_logic_vector(2 downto 0);
		S_AXI_AWVALID	: in std_logic;
		S_AXI_AWREADY	: out std_logic;
		S_AXI_WDATA	: in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_WSTRB	: in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
		S_AXI_WVALID	: in std_logic;
		S_AXI_WREADY	: out std_logic;
		S_AXI_BRESP	: out std_logic_vector(1 downto 0);
		S_AXI_BVALID	: out std_logic;
		S_AXI_BREADY	: in std_logic;
		S_AXI_ARADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_ARPROT	: in std_logic_vector(2 downto 0);
		S_AXI_ARVALID	: in std_logic;
		S_AXI_ARREADY	: out std_logic;
		S_AXI_RDATA	: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_RRESP	: out std_logic_vector(1 downto 0);
		S_AXI_RVALID	: out std_logic;
		S_AXI_RREADY	: in std_logic
		);
	end component pwm_gen_v1_0_S_AXI;

	component pwm_gen_v1_0_S_AXIS is
		generic (
		C_XADC_DATA_WIDTH : integer := 12;
		C_S_AXIS_TDATA_WIDTH	: integer	:= 16
		);
		port (
		duty_cycle : out std_logic_vector(C_S_AXIS_TDATA_WIDTH-1 downto 0);
		S_AXIS_ACLK	: in std_logic;
		S_AXIS_ARESETN	: in std_logic;
		S_AXIS_TREADY	: out std_logic;
		S_AXIS_TDATA	: in std_logic_vector(C_S_AXIS_TDATA_WIDTH-1 downto 0);
		S_AXIS_TSTRB	: in std_logic_vector((C_S_AXIS_TDATA_WIDTH/8)-1 downto 0);
		S_AXIS_TLAST	: in std_logic;
		S_AXIS_TVALID	: in std_logic
		);
	end component pwm_gen_v1_0_S_AXIS;
	
	
	component pwm_chan is
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
  end component pwm_chan;

begin

-- Instantiation of Axi Bus Interface S_AXI
pwm_gen_v1_0_S_AXI_inst : pwm_gen_v1_0_S_AXI
	generic map (
		C_S_AXI_DATA_WIDTH	=> C_S_AXI_DATA_WIDTH,
		C_S_AXI_ADDR_WIDTH	=> C_S_AXI_ADDR_WIDTH
	)
	port map (
	    phase_array => phase,
		S_AXI_ACLK	=> s_axi_aclk,
		S_AXI_ARESETN	=> s_axi_aresetn,
		S_AXI_AWADDR	=> s_axi_awaddr,
		S_AXI_AWPROT	=> s_axi_awprot,
		S_AXI_AWVALID	=> s_axi_awvalid,
		S_AXI_AWREADY	=> s_axi_awready,
		S_AXI_WDATA	=> s_axi_wdata,
		S_AXI_WSTRB	=> s_axi_wstrb,
		S_AXI_WVALID	=> s_axi_wvalid,
		S_AXI_WREADY	=> s_axi_wready,
		S_AXI_BRESP	=> s_axi_bresp,
		S_AXI_BVALID	=> s_axi_bvalid,
		S_AXI_BREADY	=> s_axi_bready,
		S_AXI_ARADDR	=> s_axi_araddr,
		S_AXI_ARPROT	=> s_axi_arprot,
		S_AXI_ARVALID	=> s_axi_arvalid,
		S_AXI_ARREADY	=> s_axi_arready,
		S_AXI_RDATA	=> s_axi_rdata,
		S_AXI_RRESP	=> s_axi_rresp,
		S_AXI_RVALID	=> s_axi_rvalid,
		S_AXI_RREADY	=> s_axi_rready
	);

-- Instantiation of Axi Bus Interface S_AXIS
pwm_gen_v1_0_S_AXIS_inst : pwm_gen_v1_0_S_AXIS
	generic map (
		C_S_AXIS_TDATA_WIDTH	=> C_S_AXIS_TDATA_WIDTH
	)
	port map (
	    duty_cycle => duty_cycle_AXI_CLK,
		S_AXIS_ACLK	=> s_axis_aclk,
		S_AXIS_ARESETN	=> s_axis_aresetn,
		S_AXIS_TREADY	=> s_axis_tready,
		S_AXIS_TDATA	=> s_axis_tdata,
		S_AXIS_TSTRB	=> s_axis_tstrb,
		S_AXIS_TLAST	=> s_axis_tlast,
		S_AXIS_TVALID	=> s_axis_tvalid
	);
	
	
	-- Add user logic here
	xpm_cdc_array_single_duty : xpm_cdc_array_single
	generic map(
	   DEST_SYNC_FF => 4,
	   INIT_SYNC_FF => 0,
	   SIM_ASSERT_CHK => 0,
	   SRC_INPUT_REG => 1,
	   WIDTH => C_S_AXIS_TDATA_WIDTH
	)
	port map (
	   dest_out => duty_cycle,
	   dest_clk => PWM_clk,
	   src_clk => s_axis_aclk,
	   src_in => duty_cycle_AXI_CLK
	);
	
	PHASE_SYNC_GEN : for I in 0 to C_NUM_CHANNELS-1 generate
        xpm_cdc_array_single_inst : xpm_cdc_array_single
        generic map(
           DEST_SYNC_FF => 4,
           INIT_SYNC_FF => 0,
           SIM_ASSERT_CHK => 0,
           SRC_INPUT_REG => 1,
           WIDTH => C_S_AXI_DATA_WIDTH
        )
        port map (
           dest_out => phase_sync(I),
           dest_clk => PWM_clk,
           src_clk => s_axis_aclk,
           src_in => phase(I)
        );
    
        PWM_inst : pwm_chan
        port map (
            PWM_clk  => PWM_clk,
            PWM_reset => PWM_reset,
            PWM => PWM(I),
            phase => phase_sync(I)(C_PWM_COUNTER_WIDTH - 1 downto 0),
            duty_cycle => duty_cycle(C_S_AXIS_TDATA_WIDTH - 1 downto 5)
        );
    
    end generate PHASE_SYNC_GEN;
    
--    phase0 <= phase(0)(C_PWM_COUNTER_WIDTH - 1 downto 0);
--    phase1 <= phase(1)(C_PWM_COUNTER_WIDTH - 1 downto 0);
--    phase2 <= phase(2)(C_PWM_COUNTER_WIDTH - 1 downto 0);
--    phase3 <= phase(3)(C_PWM_COUNTER_WIDTH - 1 downto 0);
--    phase4 <= phase(4)(C_PWM_COUNTER_WIDTH - 1 downto 0);
--    phase5 <= phase(5)(C_PWM_COUNTER_WIDTH - 1 downto 0);
--    phase6 <= phase(6)(C_PWM_COUNTER_WIDTH - 1 downto 0);
--    phase7 <= phase(7)(C_PWM_COUNTER_WIDTH - 1 downto 0);
   
	-- User logic ends

end arch_imp;
