THE ID CHECK:
to check id and to record those id

Video Demo: https://www.youtube.com/watch?v=-svoGMkCblA
the excel file: https://docs.google.com/spreadsheets/d/12JXY7RjvgY2Zw0kpSRs3cqpdkiOFfciyogjagmTJXEo/edit?gid=0#gid=0

The components that I used
- Microcontroller: ESP 32
- Sensors/Inputs: ID Scanner, Buzzer
- Outputs/Displays: TFT SPI Screen
- Other: Breadboard, Jumper wires

How to wire it:
Check it out at the screenshot of the this github page

Before uploading the code, make sure you install these libraries in the Arduino IDE / PlatformIO:
- MFRC522
- TFT_eSPI

This project is still in progress

What currently works: 
- Reading IDs through the scanner.
- Displaying UI text on the TFT SPI screen.
- The excel file works

What I plan to do: 
- Combining all the files to main to make it easier for people to use
- Adding a voice password to make it more secure (If funds are approved)
- adding a finger print scanner to make it more secure (If funds are approved)
- adding a camera to make it more secure (If funds are approved)

The wiring diagram 
<img width="1207" height="756" alt="image" src="https://github.com/user-attachments/assets/50b2fc9f-8e5b-4c15-b9f6-c619a677a886" />
<img width="1162" height="571" alt="image" src="https://github.com/user-attachments/assets/eb96b1d2-a851-430b-8232-4cc7a63d8566" />
<img width="1300" height="480" alt="image" src="https://github.com/user-attachments/assets/e88f1b47-38a4-4313-91c3-895b91901860" />
