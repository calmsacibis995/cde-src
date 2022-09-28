.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Calculator Services"
.xR1 calcsvcs
.H 2 "Introduction"
.mc 5
The \*(Zx calculator services provide basic computation
capabilities to users of the \*(ZX desktop.
They are designed to address the needs of professionals in business,
engineering and computer science.
.br
.mc
.H 2 "Actions" S
This section defines the actions that provide \*(Zx calculator
services to support application portability at the C-language source
or shell script levels.
.so ../hx/dtcalcaction.mm
.ds XC Capabilities
.SK
.H 2 "Capabilities"
A conforming implementation of the \*(Zx calculator services
supports at least the following capabilities:
.AL
.LI
Provides calculator services
as described in the following subsections.
.LI
Conforms to the Required items in the Application Style Checklist in
.cX checklist .
.LI
Has been internationalised using the standard interfaces
in the \*(ZK, the \*(Zl, and the \*(ZM,
and supports any locale available in the underlying X Window System
Xlib implementation.
.LE
.H 3 "General Calculator Capabilities"
The following general capabilities are supported by the calculator:
.AL
.LI
The user can perform calculations
using a keypad and display area modelled after a desk calculator.
.LI
The user can input from the keyboard as well as through buttons
in the calculator window.
.LI
The user can input numbers in binary, octal, decimal and hexadecimal bases.
Decimal is the default.
.LI
Calculations are executed in the order that they are entered.
The user can use parentheses to control the order of operations.
.LI
The user can set the accuracy of calculations.
.LI
Arithmetic, scientific, logical and financial functions,
described in the following subsections,
may be grouped into modes.
The arithmetic functions are always available.
.LI
The user can access at least ten memory registers.
.LI
The user can store, recall and exchange values
between a specified register and the display area.
.LI
The user can view the contents of all memory registers.
.LE
.H 3 "Arithmetic Operations"
The user can perform the following arithmetic operations:
.AL
.LI
Addition
.LI
Subtraction
.LI
Multiplication
.LI
Division
.LI
Reciprocal
.LI
Square root
.LI
Square
.LI
The value of a specified percentage of a number (for
example, 32 percent of 50)
.LI
Change the sign of a number
.LI
Integer component of a number
.LI
Fractional component of a number
.LI
Absolute value
.LE
.H 3 "Scientific Operations"
.EQ
delim $$
.EN
The user can perform the following scientific calculation operations:
.AL
.LI
$e sup x$
.LI
$10 sup x$
.LI
$y sup x$
.LI
Factorial
.LI
Trigonometric functions:
.AL a
.LI
Sine, cosine and tangent
.LI
Arc sine, arc cosine and arc tangent
.LI
Hyperbolic sine, hyperbolic cosine and hyperbolic tangent
.LE
.LI
Natural log and log base 10.
.LI
Random number between 0 and 1.
.LE
.P
The user can select one of three trigonometric bases when in scientific mode:
degrees, radiants or gradients.
Degrees is the default.
.br
.EQ
delim off
.EN
.H 3 "Financial Operations"
The user can perform the following financial calculation operations:
.AL
.LI
Time value of money based on
.AL a
.LI
Number of periods
.LI
Annual interest rate
.LI
Present value
.LI
Payment amount
.LI
Future value
.LI
Payments per year
.LE
.LI
Depreciation using the following rules:
.AL a
.LI
Double declining balance
.LI
Straight line
.LI
Sum of years digits
.LE
.LE
.P
The user can view and clear all financial registers.
.H 3 "Logical Operations"
The user can perform the following logical operations:
.AL
.LI
Bitwise logical OR, AND, NOT, XOR and XNOR.
.LI
Bitwise shift left and right.
.LI
Truncate value to 16 or 32 bits.
.LE
.H 2 "Rationale and Open Issues" S
.I
The following is text copied from the
.R
CDE X/Open Specification Framework Proposal, Draft 2.
.I
It is not intended to remain in this specification when it is published
by X/Open.
.R
.P
Capabilities are required to ensure that conforming implementations
provide the minimum expected services, without overly constraining tool
design.
.mc 5
CLIs are not planned for standardisation because Actions
are superior interfaces for these services.
.br
.mc
.eF e
