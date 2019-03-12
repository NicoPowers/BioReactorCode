# BioReactorCode

This repository holds all the software that is needed to build a fully functional and open-source bioreactor device.

The **control_megaOverseer.ino** code is meant to the run on the mega and is used to control the stretcher and pump on the Nano and vary the hydrostatic pressure on the UNO.

The **control_hydrostaticPressure.ino** code is meant to run on the UNO and is used to control the position of the vial

The **control_stretchCFR.ino** code is meant to run on the Nano and is used to control the stretching of the chamber and the MasterFlex pump

Below is the cheat sheet for the commands that can be given to both the UNO and the Nano from the Serial Monitor Terminal of the Arduino Mega (**this Command Sheet can also be found inside the control_megaOverseer.ino code**):

To send commands to the **Nano**, allow commands must start with a lowercase **"n"** (to signify nano); below are the following commands that you can send to the nano:

- **"ns"** to set the current position as the Home position
- **"nh"** to return to the home position
- **"nf%f"** to set the frequency for the oscillation where _%f_ is the value you want to set
- **"no%f"** to move outwards a distance _%f_
- **"ni%f"** to move inwards a distance _%f_
- **"nq"** to toggle the state of the MasterFlex Pump
- **"ne%f"** to set the flow rate for the MasterFlex Pump, where _%f_ ranges from 0 to 80
- **"nx"** to cancel the oscillation and return stretcher to limit switch
- **"nr%f"** to start oscillation where _%f_ is the repeating distance (in mm) of oscillation

Likewise, to the **UNO**, allow commands must start with a lowercase **"u"** (to signify UNO); below are the following commands that you can send to the UNO:

- **"ud%f"** to move the vial down a certain distance _%f_ in mm
- **"uu%f"** to move the vial up a certain distance _%f_ in mm
- **"uh"** to move back to the home position
- **"us"** to set the current position as the home position
- **"uc"** to check the current distance from the home position

Schematics and full wiring diagrams are yet to come.
