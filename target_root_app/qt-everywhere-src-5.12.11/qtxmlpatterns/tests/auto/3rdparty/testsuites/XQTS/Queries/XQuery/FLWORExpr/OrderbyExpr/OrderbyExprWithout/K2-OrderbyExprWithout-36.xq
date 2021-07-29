(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-36                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Uncomparable values in order by, but without for clause(stable sort). :)
(:*******************************************************:)
let $i  := (xs:hexBinary("FF"), xs:time("03:03:03Z"), xs:hexBinary("FF"))
    order by $i[1]
    return $i