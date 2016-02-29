clear;
close all;

temp_table = [
-55.0 963050; -50.0 670100; -45.0 471690; -40.0 336500; -35.0 242590;
-30.0 177000; -25.0 130370;  -20.0 97070;  -15.0 72929;  -10.0 55330;
  -5.0 42315;    0.0 32650;    5.0 25388;   10.0 19900;   15.0 15708;  20.0 12490;
  25.0 10000;    30.0 8057;    35.0 6531;    40.0 5327;    45.0 4369;   50.0 3603;  55.0 2986;
   60.0 2488;    65.0 2083;    70.0 1752;    75.0 1481;    80.0 1258;   85.0 1072; 90.0 917.7;
  95.0 788.5;  100.0 680.0;  105.0 588.6;  110.0 511.2;  115.0 445.4; 120.0 389.3;
 125.0 341.7;  130.0 300.9;  135.0 265.4;  140.0 234.8;  145.0 208.3; 150.0 185.3; 155.0 165.3
];

temp_index = 1;
resistance_index = 2;
max_digital_value = 1023;
reference_resistance = 9950;
output_digital_value = reference_resistance./(temp_table(:,resistance_index) + reference_resistance);
output_digital_value = round(output_digital_value * max_digital_value);
