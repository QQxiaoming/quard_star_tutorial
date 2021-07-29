(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-25                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: xs:hexBinary values cannot be compared(stable sort). :)
(:*******************************************************:)
let $i := (xs:hexBinary("FF"), xs:hexBinary("FF"))
stable order by $i
return $i