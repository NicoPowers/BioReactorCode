# UF's open-source bioreactor device

This repository holds all the software that is used for Dr. Allen's labs' bioreactor device at the University of Florida (Material Science and Engineering department).

_Designed, built, and programmed by undergraduate research student Nicolas Montoya._

The **MEGA_Controller.ino** code is meant to the run on the mega and is used to control the stretcher and pump on the Nano and vary the hydrostatic pressure on the UNO.

The **UNO_flow_staticPressure_controller.ino** code is meant to run on the UNO and is used to control the position of the vial and vary the pump sinusoidally

The **NANO_stretch_controller.ino** code is meant to run on the Nano and is used to control the stretching of the chamber

Below is the cheat sheet for the commands that can be given to both the UNO and the Nano from the Serial Monitor Terminal of the Arduino Mega (**this Command Sheet can also be found inside the MEGA_Controller.ino code**):

To send commands that affect stretching, all commands must start with a lowercase **n** (to signify nano).
Below are the following commands that you use:

- **no%f** to move outwards a distance _%f_
- **ni%f** to move inwards a distance _%f_
- **nd%f** to set the initial displacement, _%f_, between the plates
- **nx** to cancel the oscillation and return stretcher to limit switch plus some initial distance
- **nr%p%i** to start oscillation where _%p_ is whether or not to use sinusoidal flow with the stretching and _%i_ is the repeating distance index (choose from the below array) of oscillation:

  **[1.0, 1.25, 1.50, 1.75, 2.00, 7, 7.50, 7.75, 8.0] _%mm_**

  Examples:

  - nr01 -> stretch **without** sinusoidal flow rate distance of 1.25 mm
  - nr11 -> stretch **with** sinusoidal flow rate distance of 1.25 mm
  - nr03 -> stretch **with** sinusoidal flow rate distance of 1.75 mm

    The following repeating distances currently work at 1 Hz:
    5% Strain:
    **1.00 mm**
    **1.25 mm**
    **1.50 mm**
    **1.75 mm**
    **2.00 mm**

    The following repeating distances currently do not work at 1 Hz (they are less than 1 Hz since our current stepper motor cannot move any faster):
    10% Strain:
    7 mm
    7.25 mm
    7.5 mm
    7.75 mm
    8.0 mm

To send commands to change the vertical height of the water vial and to set constant or sinusoidal flow rate, all commands must start with a lowercase **u** (to signify UNO).
Below are the following commands that you can use:

- **ud%f** to move the vial down a certain distance _%f_ in mm
- **uu%f** to move the vial up a certain distance _%f_ in mm
- **uh** to move back to the home position
- **us** to set the current position as the home position
- **uc** to check the current distance from the home position
- **uh** to move back to the home position
- **us** to set the current position as the home position
- **uc** to check the current distance from the home position

Schematics and full wiring diagrams are yet to come.
