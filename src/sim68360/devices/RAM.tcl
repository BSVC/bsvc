#!/usr/local/bin/wish -f
###############################################################################
#
# This Tcl script gets the setup information for the RAM device.
#
# Notes: - All Procedures and global variables begin with "DeviceSetup".  This
#          should be true for all device scripts.
#        - The toplevel window should be .device for all device scripts.
#        - The script must return a valid argument for the device's
#          constructor. (i.e. "BaseAddress=00000000 Size=00000000")
#        - If the cancel button is pressed the empty string should be
#          returned
#        - All device scripts should be modal dialogs
#
# Copyright (c) 1993
# By: Bradford W. Mott
# October 30,1993
###############################################################################
# $Id: RAM.tcl,v 1.1 1996/08/02 15:03:04 bwmott Exp $
###############################################################################

proc DeviceSetupGetValues {} {
  set base [.device.inputs.entry.address get]
  set size [.device.inputs.entry.size get]

  set result "BaseAddress = $base Size = $size"
  return "$result"
}

proc DeviceSetupCheckValues {} {
  set base [.device.inputs.entry.address get]
  set size [.device.inputs.entry.size get]

  if {[regexp {^[0-9A-Fa-f]+$} $base] && [regexp {^[0-9A-Fa-f]+$} $size]} {
    destroy .device 
  } else {
    
  }
}

###############################################################################
# This is the procedure the User Interface calls
###############################################################################
proc DeviceSetup {} {
  global DeviceSetupReturnValue

  catch {destroy .device}
 
  toplevel .device
  wm title .device "RAM Setup"
  wm iconname .device "RAM Setup"

  message .device.message \
    -text "Please enter the base address and size of the RAM.\n\nAll values are in hexadecimal!" \
    -width 3i -justify left

  frame .device.inputs -relief ridge -borderwidth 2
    frame .device.inputs.label
      label .device.inputs.label.address -text "Base Address:"
      label .device.inputs.label.size -text "Size:"
      pack .device.inputs.label.address -side top 
      pack .device.inputs.label.size -side right
    frame .device.inputs.entry
      entry .device.inputs.entry.address -width 10 -relief sunken
      bind .device.inputs.entry.address \
          <Return> { focus .device.inputs.entry.size }
      entry .device.inputs.entry.size -width 10 -relief sunken
      bind .device.inputs.entry.size \
          <Return> { focus .device.inputs.entry.address }
      pack .device.inputs.entry.address -side top -fill x -expand 1 -pady 2
      pack .device.inputs.entry.size -side top -fill x -expand 1 -pady 2
    pack .device.inputs.label -side left 
    pack .device.inputs.entry -side left -fill x -expand 1 -padx 2

  frame .device.buttons
    button .device.buttons.ok -text "Okay" \
      -command {set DeviceSetupReturnValue [DeviceSetupGetValues]
                DeviceSetupCheckValues}
    button .device.buttons.cancel -text "Cancel" \
      -command {set DeviceSetupReturnValue ""; destroy .device}
    pack .device.buttons.ok -side left -expand 1 -fill x -padx 4
    pack .device.buttons.cancel -side right -expand 1 -fill x -padx 4

  pack .device.message -side top -fill x -pady 4 -padx 4
  pack .device.inputs -side top -fill x -pady 2 -padx 4 -ipady 2
  pack .device.buttons -side top -fill x -pady 4

  # Set input focus to the first entry widget
  tkwait visibility .device
  focus .device.inputs.entry.address

  # Make this a modal dialog
  grab set .device
  tkwait window .device

  return $DeviceSetupReturnValue
}
