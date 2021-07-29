(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-17                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: variable declarations doesn't cause numeric promotion(#5). :)
(:*******************************************************:)
declare variable $i as xs:double := xs:float(3) ; $i