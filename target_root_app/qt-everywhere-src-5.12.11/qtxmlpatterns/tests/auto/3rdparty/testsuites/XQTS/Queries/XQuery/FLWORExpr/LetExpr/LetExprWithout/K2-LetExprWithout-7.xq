(:*******************************************************:)
(: Test: K2-LetExprWithout-7                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: let-declarations doesn't cause type conversion. :)
(:*******************************************************:)
let $i as xs:integer := xs:untypedAtomic("1")
return $i