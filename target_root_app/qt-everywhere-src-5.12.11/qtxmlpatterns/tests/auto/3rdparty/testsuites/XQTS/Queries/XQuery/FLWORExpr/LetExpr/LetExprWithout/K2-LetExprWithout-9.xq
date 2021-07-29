(:*******************************************************:)
(: Test: K2-LetExprWithout-9                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: let-declarations doesn't cause numeric promotion. :)
(:*******************************************************:)
let $i as xs:float := 1.1
return $i