(:*******************************************************:)
(: Test: K2-LetExprWithout-13                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: let-declarations doesn't cause numeric promotion(#5). :)
(:*******************************************************:)
let $i as xs:double := xs:float(3)
return $i