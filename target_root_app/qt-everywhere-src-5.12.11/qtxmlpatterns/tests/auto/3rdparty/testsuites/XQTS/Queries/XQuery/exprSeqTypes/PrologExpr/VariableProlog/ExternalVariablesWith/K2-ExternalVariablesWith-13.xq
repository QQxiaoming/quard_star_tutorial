(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-13                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: variable declarations doesn't cause numeric promotion. :)
(:*******************************************************:)
declare variable $i as xs:float := 1.1 ; $i