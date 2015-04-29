#!/usr/local/bin/wish -f
#
# CREATED: 	7-24-98
# OWNER :	Xavier Plasencia
# ORG:		SDSU
# DESC:		Allows the programmer to set the memory location of the timer
# NOTES: - All Procedures and global variables begin with "DeviceSetup".  This
#          should be true for all device scripts.
#        - The toplevel window should be .device for all device scripts.
#        - The script must return a valid argument for the device's
#          constructor. (i.e. "BaseAddress= 0x10021 IRQ = 0x5")
#        - All device scripts should be modal dialogs

proc DeviceSetupGetValues {} {
  set base [.device.inputs.entry.address get]
  set irq [.device.inputs.entry.interrupt get]

  set result "BaseAddress = $base IRQ = $irq"
  return "$result"
}

proc DeviceSetupCheckValues {} {
  set base [.device.inputs.entry.address get]
  set irq [.device.inputs.entry.interrupt get]

  if {[regexp {^[0-9A-Fa-f]+$} $base] && [regexp {^[57]$} $irq]} {
    destroy .device 
  }
}

###############################################################################
# This is the procedure the User Interface calls
###############################################################################
proc DeviceSetup {} {
  global DeviceSetupReturnValue

  catch {destroy .device}
 
  toplevel .device
  wm title .device "Timer Setup"
  wm iconname .device "Timer Setup"

  message .device.message -width 4.25i -justify left \
    -text "This device is modeled after the digital timer feature of the M68230 PI/T.\n\nPlease enter the base address of the timer as well as its interrupt level.  Common values are 10021 for the base address and 5 or 7 for the interrupt level:"

  frame .device.inputs -relief ridge -borderwidth 2
    frame .device.inputs.label
      label .device.inputs.label.address -text "Base Address:"
      label .device.inputs.label.interrupt -text "Interrupt Level:"
      pack .device.inputs.label.address -side top 
      pack .device.inputs.label.interrupt -side top 
    frame .device.inputs.entry
      entry .device.inputs.entry.address -width 10 -relief sunken
      bind .device.inputs.entry.address \
          <Return> { focus .device.inputs.entry.interrupt }
      entry .device.inputs.entry.interrupt -width 10 -relief sunken
      bind .device.inputs.entry.interrupt \
          <Return> { focus .device.inputs.entry.address }
      pack .device.inputs.entry.address -side top -fill x -expand 1 -pady 2
      pack .device.inputs.entry.interrupt -side top -fill x -expand 1 -pady 2
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

