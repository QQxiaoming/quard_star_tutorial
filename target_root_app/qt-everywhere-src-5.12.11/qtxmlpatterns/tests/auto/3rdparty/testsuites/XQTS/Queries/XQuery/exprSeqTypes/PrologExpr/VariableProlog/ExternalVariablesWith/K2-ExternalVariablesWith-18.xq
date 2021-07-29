(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-18                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: variable declarations doesn't cause string promotion conversion. :)
(:*******************************************************:)
declare variable $i as xs:string := xs:untypedAtomic("a string") ; $i