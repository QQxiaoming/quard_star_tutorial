(:*******************************************************:)
(: Test: K-InternalVariablesWith-5                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A variable declaration whose source expression doesn't match the declared type, and where it can be deduced statically. :)
(:*******************************************************:)
declare variable $myVar as xs:gYear := 2006; $myVar