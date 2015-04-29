#!/usr/local/bin/wish -f
#
# Only need base address and port number

proc DeviceSetupGetValues {} {
  set base [.device.inputs.entry.address get]
  set port [.device.inputs.entry.port get]

  set result "BaseAddress = $base PortNumber = $port"
  return "$result"
}

proc DeviceSetupCheckValues {} {
  set base [.device.inputs.entry.address get]
  set port [.device.inputs.entry.port get]

  if {[regexp {^[0-9A-Fa-f]+$} $base] && [regexp {^[0-9]+$} $port]} {
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
  wm title .device "GDB Socket Setup"
  wm iconname .device "GDB Socket Setup"

  message .device.message \
    -text "Please enter the base address and port for GDB socket.\n\nAddress in hexadecimal\n\nPort in decimal!" \
    -width 3i -justify left

  frame .device.inputs -relief ridge -borderwidth 2
    frame .device.inputs.label
      label .device.inputs.label.address -text "Base Address:"
      label .device.inputs.label.port -text "Port:"
      pack .device.inputs.label.address -side top 
      pack .device.inputs.label.port -side right
    frame .device.inputs.entry
      entry .device.inputs.entry.address -width 10 -relief sunken
      bind .device.inputs.entry.address \
          <Return> { focus .device.inputs.entry.port }
      entry .device.inputs.entry.port -width 10 -relief sunken
      bind .device.inputs.entry.port \
          <Return> { focus .device.inputs.entry.address }
      pack .device.inputs.entry.address -side top -fill x -expand 1 -pady 2
      pack .device.inputs.entry.port -side top -fill x -expand 1 -pady 2
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
