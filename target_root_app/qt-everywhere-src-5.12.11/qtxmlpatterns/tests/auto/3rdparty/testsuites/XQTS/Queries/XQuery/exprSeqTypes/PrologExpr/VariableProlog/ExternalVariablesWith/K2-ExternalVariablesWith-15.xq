(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-15                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: variable declarations doesn't cause numeric promotion(#3). :)
(:*******************************************************:)
declare variable $i as xs:float := 1 ; $i