(:*******************************************************:)
(: Test: K2-LetExprWithout-14                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: let-declarations doesn't cause string promotion conversion. :)
(:*******************************************************:)
let $i as xs:string := xs:untypedAtomic("a string")
return $i