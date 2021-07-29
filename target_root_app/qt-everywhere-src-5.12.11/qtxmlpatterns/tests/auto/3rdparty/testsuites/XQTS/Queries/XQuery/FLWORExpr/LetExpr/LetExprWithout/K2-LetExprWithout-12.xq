(:*******************************************************:)
(: Test: K2-LetExprWithout-12                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: let-declarations doesn't cause numeric promotion(#4). :)
(:*******************************************************:)
let $i as xs:double := 1
return $i