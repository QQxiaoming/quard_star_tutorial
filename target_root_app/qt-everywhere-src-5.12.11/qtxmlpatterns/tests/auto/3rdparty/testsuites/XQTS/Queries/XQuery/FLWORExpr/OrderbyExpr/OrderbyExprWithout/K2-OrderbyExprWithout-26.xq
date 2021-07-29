(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-26                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: xs:hexBinary values cannot be compared.      :)
(:*******************************************************:)
let $i := (xs:hexBinary("FF"), xs:hexBinary("FF"))
order by $i
return $i