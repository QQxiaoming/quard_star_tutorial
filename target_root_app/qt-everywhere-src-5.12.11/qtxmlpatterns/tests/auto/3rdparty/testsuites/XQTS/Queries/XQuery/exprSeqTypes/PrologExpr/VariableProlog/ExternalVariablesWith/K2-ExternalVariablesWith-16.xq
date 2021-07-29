(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-16                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: variable declarations doesn't cause numeric promotion(#4). :)
(:*******************************************************:)
declare variable $i as xs:double := 1 ; $i