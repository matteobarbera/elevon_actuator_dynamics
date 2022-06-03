#include <math.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Servo.h>
#include <MPU6050_light.h>
unsigned long int start_time_offset, Test_time, t1, t2, t_ramp, t_end;
int Test_value;
float frequency, min_freq, max_freq;
//////////////////VARIABLES FOR SD MEMORY
File logfile;
int close_var;

//////////////////VARIABLES FOR MPU6050
MPU6050 mpu(Wire);

//////////////////VARIABLES FOR SERVOS
Servo Servo_elevon;
Servo Servo_motor;
int PWM_elevon, PWM_motor;

int PIN_servo_elevon = 6; // FIXME determine
int PIN_servo_motor = 3;

int Lower_limit_elevon = 1100;
int Zero_value_elevon = 1500;
int Higher_limit_elevon = 1800;  // FIXME double check!!

int Lower_limit_motor = 1000;
int Zero_value_motor = 1500;
int Higher_limit_motor = 2000;  // FIXME double check!!

void setup() {
  
  // Initialize the serial communication(s)
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(3400000);

  //Initialize servos  FIXME
  Servo_elevon.attach(PIN_servo_elevon);
  Servo_motor.attach(PIN_servo_motor);
  Servo_elevon.writeMicroseconds(Zero_value_elevon);
  Servo_motor.writeMicroseconds(Zero_value_motor);
  
  
  // Initialize the MPU9250

  if(!mpu.begin()){
    Serial.println("MPU9250 does not respond");
  }
  else{
    Serial.println("MPU9250 is connected");
  }


  /* The slope of the curve of acceleration vs measured values fits quite well to the theoretical 
   * values, e.g. 16384 units/g in the +/- 2g range. But the starting point, if you position the 
   * MPU9250 flat, is not necessarily 0g/0g/1g for x/y/z. The autoOffset function measures offset 
   * values. It assumes your MPU9250 is positioned flat with its x,y-plane. The more you deviate 
   * from this, the less accurate will be your results.
   * The function also measures the offset of the gyroscope data. The gyroscope offset does not   
   * depend on the positioning.
   * This function needs to be called at the beginning since it can overwrite your settings!
   */
  Serial.println("Position you MPU9250 flat and don't move it - calibrating...");
  delay(100);
  mpu.setGyroOffsets(1.32,-0.54,1.64);
  mpu.setAccOffsets(0.13,0.07,-0.07);
  Serial.println("Done!");
  
  /*  This is a more accurate method for calibration. You have to determine the minimum and maximum 
   *  raw acceleration values of the axes determined in the range +/- 2 g. 
   *  You call the function as follows: setAccOffsets(xMin,xMax,yMin,yMax,zMin,zMax);
   *  Use either autoOffset or setAccOffsets, not both.
   */
  //myMPU9250.setAccOffsets(-14240.0, 18220.0, -17280.0, 15590.0, -20930.0, 12080.0);

  /*  The gyroscope data is not zero, even if you don't move the MPU9250. 
   *  To start at zero, you can apply offset values. These are the gyroscope raw values you obtain
   *  using the +/- 250 degrees/s range. 
   *  Use either autoOffset or setGyrOffsets, not both.
   */
  //myMPU9250.setGyrOffsets(45.0, 145.0, -105.0);

  /*  You can enable or disable the digital low pass filter (DLPF). If you disable the DLPF, you 
   *  need to select the bandwdith, which can be either 8800 or 3600 Hz. 8800 Hz has a shorter delay,
   *  but higher noise level. If DLPF is disabled, the output rate is 32 kHz.
   *  MPU9250_BW_WO_DLPF_3600 
   *  MPU9250_BW_WO_DLPF_8800
   */
   // myMPU9250.enableGyrDLPF();
     //mpu.disableGyrDLPF(MPU9250_BW_WO_DLPF_3600); // bandwdith without DLPF
  
  /*  Digital Low Pass Filter for the gyroscope must be enabled to choose the level. 
   *  MPU9250_DPLF_0, MPU9250_DPLF_2, ...... MPU9250_DPLF_7 
   *  
   *  DLPF    Bandwidth [Hz]   Delay [ms]   Output Rate [kHz]
   *    0         250            0.97             8
   *    1         184            2.9              1
   *    2          92            3.9              1
   *    3          41            5.9              1
   *    4          20            9.9              1
   *    5          10           17.85             1
   *    6           5           33.48             1
   *    7        3600            0.17             8
   *    
   *    You achieve lowest noise using level 6  
   */
    // myMPU9250.setGyrDLPF(MPU9250_DLPF_0);

  /*  Sample rate divider divides the output rate of the gyroscope and accelerometer.
   *  Sample rate = Internal sample rate / (1 + divider) 
   *  It can only be applied if the corresponding DLPF is enabled and 0<DLPF<7!
   *  Divider is a number 0...255
   */
    // myMPU9250.setSampleRateDivider(0);

  /*  MPU9250_GYRO_RANGE_250       250 degrees per second (default)
   *  MPU9250_GYRO_RANGE_500       500 degrees per second
   *  MPU9250_GYRO_RANGE_1000     1000 degrees per second
   *  MPU9250_GYRO_RANGE_2000     2000 degrees per second
   */
  //mpu.setGyrRange(MPU9250_GYRO_RANGE_1000);

  /*  MPU9250_ACC_RANGE_2G      2 g   (default)
   *  MPU9250_ACC_RANGE_4G      4 g
   *  MPU9250_ACC_RANGE_8G      8 g   
   *  MPU9250_ACC_RANGE_16G    16 g
   */
  //mpu.setAccRange(MPU9250_ACC_RANGE_2G);

  /*  Enable/disable the digital low pass filter for the accelerometer 
   *  If disabled the bandwidth is 1.13 kHz, delay is 0.75 ms, output rate is 4 kHz
   */
  // myMPU9250.enableAccDLPF(true);

  /*  Digital low pass filter (DLPF) for the accelerometer, if enabled 
   *  MPU9250_DPLF_0, MPU9250_DPLF_2, ...... MPU9250_DPLF_7 
   *   DLPF     Bandwidth [Hz]      Delay [ms]    Output rate [kHz]
   *     0           460               1.94           1
   *     1           184               5.80           1
   *     2            92               7.80           1
   *     3            41              11.80           1
   *     4            20              19.80           1
   *     5            10              35.70           1
   *     6             5              66.96           1
   *     7           460               1.94           1
   */
  // myMPU9250.setAccDLPF(MPU9250_DLPF_7);

  /* You can enable or disable the axes for gyroscope and/or accelerometer measurements.
   * By default all axes are enabled. Parameters are:  
   * MPU9250_ENABLE_XYZ  //all axes are enabled (default)
   * MPU9250_ENABLE_XY0  // X, Y enabled, Z disabled
   * MPU9250_ENABLE_X0Z   
   * MPU9250_ENABLE_X00
   * MPU9250_ENABLE_0YZ
   * MPU9250_ENABLE_0Y0
   * MPU9250_ENABLE_00Z
   * MPU9250_ENABLE_000  // all axes disabled
   */
  //myMPU9250.enableAccAxes(MPU9250_ENABLE_XYZ);
  //myMPU9250.enableGyrAxes(MPU9250_ENABLE_XYZ);
  
  /*
   * AK8963_PWR_DOWN       
   * AK8963_CONT_MODE_8HZ         default
   * AK8963_CONT_MODE_100HZ
   * AK8963_FUSE_ROM_ACC_MODE 
   */
  // myMPU9250.setMagOpMode(AK8963_CONT_MODE_100HZ);
    
  // Initialize the SD card
  if (!SD.begin(14)) 
  {
    Serial.println("WARNING, No SD card detected."); 
  }
  else
  {
    Serial.println("SD card detected."); 
  }

  //Flush previous data test:
  SD.remove("Data.csv");
  logfile = SD.open("Data.csv", FILE_WRITE);
  logfile.println("Timestamp[ms],PWM_motor,PWM_elevon,Rate_x[deg/s],Rate_y[deg/s],Rate_z[deg/s]");
  
}

void loop() {
  // FIXME initialization? no motor for my test
  mpu.update();
  while(millis() < 1000){
    mpu.update();
    Servo_motor.writeMicroseconds(Zero_value_motor);

    start_time_offset = millis();
  }
  
  Test_time = millis() - start_time_offset;

  char buffer[40];  // FIXME
  // xyzFloat magValue = myMPU9250.getMagValues();
  
  // FIXME
  sprintf( buffer,"%d,%d,%d,%d,%d,%d",(int) Test_time, PWM_motor, PWM_elevon,(int) round(mpu.getGyroX()),(int) round(mpu.getGyroY()),(int) round(mpu.getGyroZ()));
  logfile.println(buffer);
  // Serial.println(buffer);

  min_freq = 0.5;
  max_freq = 10;
  t1 = 2000;  // 2 seconds of constant frequency
  t_ramp = 120000;  // 120 seconds of ramp increase
  t2 = t1 + t_ramp;
  t_end = t1 + t2;  // Keep max frequency for 2 seconds
  frequency = min_freq + ((max_freq - min_freq) / t_ramp) * (Test_time - t1);  // Frequency ramp
  
  if(Test_time < t1){
    frequency = min_freq;
  }
  else if(Test_time >= t2) { 
    frequency = max_freq;
  }
  float quarter_amplitude = (Higher_limit_elevon - Lower_limit_elevon)/4;
  PWM_elevon = (int)((Zero_value_elevon + quarter_amplitude) + quarter_amplitude * sin(Test_time * .001 * 2 * PI * frequency));
  
  if (Test_time >= t_end) {
    if(close_var == 0){
      logfile.close();
      Serial.println("Test done, closing the log file"); 
      close_var = 1;
    }
    PWM_elevon = Zero_value_elevon;
    PWM_motor = Zero_value_motor;    
  }
  Servo_elevon.writeMicroseconds(PWM_elevon);
  Servo_motor.writeMicroseconds(PWM_motor);
  delay(1);
}
