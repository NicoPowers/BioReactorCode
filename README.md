# UF's open-source bioreactor device

This repository holds all the software that is used for Dr. Allen's labs' bioreactor device at the University of Florida (Material Science and Engineering department).

The **MEGA_Controller.ino** code is meant to send commands to the stretch controller (NANO) and flow rate controller (UNO).

The **UNO_flow_staticPressure_controller.ino** code is meant to run on the UNO and is used to control the position of the vial and vary the pump sinusoidally.

The **NANO_stretch_controller.ino** code is meant to run on the Nano and is used to control the stretching of the chamber sinusoidally.

Below is the cheat sheet for the commands that can be given to both the UNO and the Nano from the Serial Monitor Terminal of the Arduino Mega (**this Command Sheet can also be found inside the MEGA_Controller.ino code**):

To send commands that affect stretching, all commands must start with a lowercase **n** (to signify nano).
Below are the following commands that you use:

**Command Legend**

**_%f_ => float**

**_%i_ => integer**

**_%b_ => boolean (still expressed as an integer; 1 -> true | 0 -> false)**

- **no%f** to move **outwards** a distance _%f_
- **ni%f** to move **inwards** a distance _%f_
- **nd%f** to set the **initial displacement**, _%f_, between the plates
- **nx** to **cancel the oscillation** and return stretcher to limit switch plus some initial distance
- **nr%b%i** to **start oscillation** where _%b_ is whether or not to use sinusoidal flow and _%i_ is the repeating distance index (choose from the below array):

  **[1.0, 1.25, 1.50, 1.75, 2.00, 7, 7.50, 7.75, 8.0] mm**

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

- **ud%f** to move the vial **down** a certain distance _%f_ (mm)
- **uu%f** to move the vial **up** a certain distance _%f_ (mm)
- **uh** to move back to the **home** position
- **ux** to **set the current position** as the home position
- **uc** to **check the current distance** from the home position
- **uq** to **toggle** the MasterFlex pump on/off
- **ur** to **enable sinusoidal flow rate** (**does not start sinusoidal flow rate, only enables it, Nano triggers it so they are synced**)
  **NOTE:**
  - If you set a constant flow rate, you **MUST** re-enable sinusoidal flow rate before you want to do sinusoidal flow control
- **up%f** to set the **phase**, _%f_ (degrees), of the sinusoidal flow rate
- **us%f** to set the **vertical shift**, _%f_ (mL/min), of the sinusoidal flow rate
- **ua%f** to set the **amplitude**, _%f_ (mL/min), of the sinusoidal flow rate
- **ue%f** to set the manual flow rate (**constant**), where _%f_ is the value
- **ut%i** to set the **tubing size**, _%i_, (**16 or 17: Default selected tubing size is 16**)

External libraries used can also be found in this repositiory.
Schematics and full wiring diagrams are expected to come with version 2.

_Designed, built, and programmed by undergraduate research student Nicolas Montoya._
