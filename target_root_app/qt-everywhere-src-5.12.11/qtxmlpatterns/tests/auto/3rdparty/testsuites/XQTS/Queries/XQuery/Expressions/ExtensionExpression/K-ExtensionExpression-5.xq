(:*******************************************************:)
(: Test: K-ExtensionExpression-5                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A pragma expression cannot be in the empty namespace even though a prefix is used. :)
(:*******************************************************:)
declare namespace prefix = "";
(# prefix:notRecognized #){1}