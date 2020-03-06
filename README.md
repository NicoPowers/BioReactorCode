# BioReactorCode

This repository holds all the software that is needed to build a fully functional and open-source bioreactor device.

The **MEGA_Controller.ino** code is meant to the run on the mega and is used to control the stretcher and pump on the Nano and vary the hydrostatic pressure on the UNO.

The **UNO_flow_staticPressure_controller.ino** code is meant to run on the UNO and is used to control the position of the vial and vary the pump sinusoidally

The **NANO_stretch_controller.ino** code is meant to run on the Nano and is used to control the stretching of the chamber

Below is the cheat sheet for the commands that can be given to both the UNO and the Nano from the Serial Monitor Terminal of the Arduino Mega (**this Command Sheet can also be found inside the MEGA_Controller.ino code**):

To send commands that affect stretching, all commands must start with a lowercase **"n"** (to signify nano); below are the following commands that you can send to the nano:

- **"no%f"** to move outwards a distance _%f_
- **"ni%f"** to move inwards a distance _%f_
- **"nx"** to cancel the oscillation and return stretcher to limit switch plus some initial distance
- **"nr%p%i"** to start oscillation where _%p_ is whether or not to use sinusodial flow with the stretching and _%i_ is the repeating distance index (choose from the below array) of oscillation:
  **[1.0, 1.25, 1.50, 1.75, 2.00, 7, 7.50, 7.75, 8.0]**
  Examples:
  - nr01 -> stretch **without** sinusodial flow rate distance of 1.25 mm
  - nr11 -> stretch **with** sinusodial flow rate distance of 1.25 mm
  - nr03 -> stretch **with** sinusodial flow rate distance of 1.75 mm

Likewise, to the **UNO**, all commands must start with a lowercase **"u"** (to signify UNO); below are the following commands that you can send to the UNO:

- **"ud%f"** to move the vial down a certain distance _%f_ in mm
- **"uu%f"** to move the vial up a certain distance _%f_ in mm
- **"uh"** to move back to the home position
- **"us"** to set the current position as the home position
- **"uc"** to check the current distance from the home position

Schematics and full wiring diagrams are yet to come.
