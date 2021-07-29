(:*******************************************************:)
(: Test: K2-LetExprWithout-10                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: let-declarations doesn't cause numeric promotion(#2). :)
(:*******************************************************:)
let $i as xs:double := 1.1
return $i