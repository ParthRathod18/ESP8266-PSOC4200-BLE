-- =============================================================================
-- The following directives assign pins to the locations specific for the
-- CY8CKIT-048 kit.
-- =============================================================================

-- === SCB (Unconfigured) ===
attribute port_location of \Comm:uart_rx_i2c_scl_spi_mosi(0)\ : label is "PORT(4,0)";
attribute port_location of \Comm:uart_tx_i2c_sda_spi_miso(0)\ : label is "PORT(4,1)";

-- === RGB LED ===
attribute port_location of LED_RED(0)   : label is "PORT(1,4)";
attribute port_location of LED_GREEN(0) : label is "PORT(2,6)";
attribute port_location of LED_BLUE(0)  : label is "PORT(1,6)";

-- === USER SWITCH ===
attribute port_location of SW2(0) : label is "PORT(0,3)";
