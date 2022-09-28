.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/07/21
The following terms are used in this specification:
.gS
.gT "can"
This describes a permissible optional
feature or behaviour available to the user or application;
all systems support such features or behaviour as mandatory requirements.
.gT "implementation-dependent"
The value or behaviour is not consistent across all implementations.
The provider of an implementation 
normally documents the requirements for correct program construction and
correct data in the use of that value or behaviour.  When the value or
behaviour in the implementation is designed to be variable or customisable on
each instantiation of the system, the provider of the implementation
normally documents the nature and permissible ranges of this variation.
Applications that are intended to be portable must not rely on
implementation-dependent values or behaviour.
.gT "may"
With respect to implementations, the feature or behaviour is optional.  
Applications should not rely on the existence of the feature.  To avoid
ambiguity, the reverse sense of 
.I may
is expressed as
.I "need not" ,
instead of 
.I "may not" .
.gT "must"
This describes a requirement on the application or user.
.gT "obsolescent"
Certain features are
.IR obsolescent ,
which means that they
may be considered for withdrawal in future revisions of this
document.
They are retained in this version because of their widespread use.
Their use in new applications is discouraged.
.gT "should"
With respect to implementations, the feature is recommended, but it
is not mandatory.
Applications should not rely on the existence of the feature.
.P
With respect to users or applications, the word means 
recommended programming practice that is necessary for maximum portability.
.gT "undefined"
A value or behaviour is undefined if this document imposes no 
portability requirements
on applications for erroneous program constructs or erroneous data.
Implementations
may specify the result of using that value or causing that behaviour,
but such specifications
are not guaranteed to be consistent across all implementations.
An application using such behaviour is not fully portable to all systems.
.gT "unspecified"
A value or behaviour is 
unspecified if this document imposes no portability requirements
on applications for correct program construct or correct data.  Implementations
may specify the result of using that value or causing that behaviour, 
but such specifications
are not guaranteed to be consistent across all implementations.
An application requiring a specific behaviour, rather
than tolerating any behaviour when using that functionality, is
not fully portable to all systems.
.gE
