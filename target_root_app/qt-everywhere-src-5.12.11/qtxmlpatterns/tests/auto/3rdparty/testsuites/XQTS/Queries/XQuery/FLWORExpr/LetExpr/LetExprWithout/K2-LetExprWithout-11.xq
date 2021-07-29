(:*******************************************************:)
(: Test: K2-LetExprWithout-11                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: let-declarations doesn't cause numeric promotion(#3). :)
(:*******************************************************:)
let $i as xs:float := 1
return $i