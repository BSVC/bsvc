#!/usr/local/bin/wish -f
#
# This Tcl script gets the setup information for the m68681 DUART device.
#
# Notes: - All Procedures and global variables begin with "DeviceSetup".  This
#          should be true for all device scripts.
#        - The toplevel window should be .device for all device scripts.
#        - The script must return a valid argument for the device's
#          constructor.
#
#        - If the cancel button is pressed the empty string should be
#          returned
#        - All device scripts should be modal dialogs

###############################################################################
# This procedure builds the argument for the device's constructor
###############################################################################
proc DeviceSetupGetValues {} {
  global DevicePortAStd
  global DevicePortBStd

  set base [.device.base.entry get]
  set offset_to_first [.device.offsetToFirst get]
  set offset_between [.device.offsetBetween get]
  set interrupt_level [.device.interruptLevel get]
  set port_a_command [.device.portA.entry get]
  if {$DevicePortAStd == 1} {set port_a_std 1} else {set port_a_std 0}
  set port_b_command [.device.portB.entry get] 
  if {$DevicePortBStd == 1} {set port_b_std 1} else {set port_b_std 0}

  set result "BaseAddress = $base OffsetToFirstRegister = $offset_to_first OffsetBetweenRegisters = $offset_between InterruptLevel = $interrupt_level PortAStandardInputOutputFlag = $port_a_std PortBStandardInputOutputFlag = $port_b_std PortACommand = $port_a_command PortBCommand = $port_b_command"

  return "$result"
}

###############################################################################
# Make sure they entered a good address
###############################################################################
proc DeviceSetupCheckValues {} {
  set base [.device.base.entry get]

  if {[regexp {^[0-9A-Fa-f]+$} $base]} {
    destroy .device 
  }
}

###############################################################################
# This is the procedure the User Interface calls
###############################################################################
proc DeviceSetup {} {
  global DeviceSetupReturnValue
  global DevicePortAStd
  global DevicePortBStd

  catch {destroy .device}
 
  toplevel .device
  wm title .device "M68681 Setup"
  wm iconname .device "M68681 Setup"

  message .device.message \
    -text "Please enter appropriate values for the M68681's options" \
    -width 4i -justify left -relief raised -pady 4

  frame .device.base -relief groove -borderwidth 2
    label .device.base.label -text "Base Address:"
    entry .device.base.entry -width 10 -relief sunken
    bind .device.base.entry <Return> { focus .device.portA.entry }
    pack .device.base.label -side left
    pack .device.base.entry -side left -fill x -expand 1 -pady 2 -padx 2

  scale .device.offsetToFirst -from 0 -to 8 -showvalue 0 -orient horizontal \
      -tickinterval 1 -relief groove -label "Offset to first register (MRxA):" 

  scale .device.offsetBetween -from 1 -to 8 -showvalue 0 -orient horizontal \
      -tickinterval 1 -relief groove -label "Offset between registers:"

  scale .device.interruptLevel -from 1 -to 7 -showvalue 0 -orient horizontal \
      -tickinterval 1 -relief groove -label "Interrupt level:"

  frame .device.portA -relief groove -borderwidth 2 
    label .device.portA.label -text "Port A Command:"
    entry .device.portA.entry -width 10 -relief sunken
    bind .device.portA.entry <Return> { focus .device.portB.entry }
    checkbutton .device.portA.stdin -relief flat -variable DevicePortAStd \
        -text "Attach port to command's STDIN/STDOUT?"
    pack .device.portA.stdin -side bottom -fill x -expand 1
    pack .device.portA.label -side left
    pack .device.portA.entry -side left -fill x -expand 1 -pady 4 -padx 4

  frame .device.portB -relief groove -borderwidth 2 
    label .device.portB.label -text "Port B Command:"
    entry .device.portB.entry -width 10 -relief sunken
    bind .device.portB.entry <Return> { focus .device.base.entry }
    checkbutton .device.portB.stdin -relief flat -variable DevicePortBStd \
        -text "Attach port to command's STDIN/STDOUT?"
    pack .device.portB.stdin -side bottom -fill x -expand 1
    pack .device.portB.label -side left
    pack .device.portB.entry -side left -fill x -expand 1 -pady 4 -padx 4

  frame .device.buttons
    button .device.buttons.ok -text "Okay" \
      -command {set DeviceSetupReturnValue [DeviceSetupGetValues]
                DeviceSetupCheckValues}
    button .device.buttons.cancel -text "Cancel" \
      -command {set DeviceSetupReturnValue ""; destroy .device}
    pack .device.buttons.ok -side left -expand 1 -fill x -padx 4
    pack .device.buttons.cancel -side right -expand 1 -fill x -padx 4

  pack .device.message -side top -fill x -pady 4 -padx 4
  pack .device.base -side top -fill x -pady 4 -padx 4
  pack .device.offsetToFirst -side top -fill x -pady 4 -padx 4 \
      -ipadx 4 -ipady 4
  pack .device.offsetBetween -side top -fill x -pady 4 -padx 4 \
      -ipadx 4 -ipady 4
  pack .device.interruptLevel -side top -fill x -pady 4 -padx 4 \
      -ipadx 4 -ipady 4
  pack .device.portA -side top -fill x -pady 4 -padx 4
  pack .device.portB -side top -fill x -pady 4 -padx 4
  pack .device.buttons -side top -fill x -pady 4

  # Set some defaults
  .device.offsetToFirst set 1
  .device.offsetBetween set 2
  .device.interruptLevel set 4
  set DevicePortAStd 0
  set DevicePortBStd 0

  # Set focus to the first entry widget
  tkwait visibility .device
  focus .device.base.entry

  # Make this a modal dialog
  grab set .device
  tkwait window .device

  return $DeviceSetupReturnValue
}
