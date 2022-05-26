<h1>IoT-based Application of Software Over-The-Air (OTA) in Automotive Embedded Systems</h1>

<p>Software Over-the-Air (OTA) updates started winning the ground as a fast way of supplying new software without disturbing the user or normal operation of targeted device.</p>
<p>This requires the development of a platform with possibility of dynamic deployment and update of applications.</p>
<p>These is an application of the solution for vehicles update that includes integration of IoT technologies with usage of security and access policies in a platform implemented similar to automotive software architecture.</p>
<p>The project has 3 parts:</p>
<li>Visual Studio C++ Encription aplication</li>
<li>Gateway module based on Wemos D1 Pro with ESP8266</li>
<li>Cluster module based on TTGO Display with ESP32</li>
<h2>PC Encription</h2>
<p>The encryption algorithm of choice is AES core algorithm found within the "libraries/Crypto" directory in the repository (https://rweather.github.io/arduinolibs/files.html), with a key of 128bits.</p>
<p>The algorithm has been optimized to run on the embedded platform and has been tested with ESP8266-based Arduino devices so a natural approach was to create a PC program that integrate it and use the same classes to encrypt on PC as the gateway will use to decrypt the firmware.</p>
The PC application has been created as a console application and can be invoked from the command line with two arguments, see Figure 5:
AES_GatewayFirmwareEncrypt.exe Firmware Mode  
Where Mode can be:
<li>“1” - for decryption or</li>
<li>“2” - for encryption</li>

<h2>Gateway</h2>
<p>On power-on, the gateway will connect to the hardcoded SSID using the hardcoded password. On a successful connection, the allocated IP address is printed on the serial communication to the PC.</p>
