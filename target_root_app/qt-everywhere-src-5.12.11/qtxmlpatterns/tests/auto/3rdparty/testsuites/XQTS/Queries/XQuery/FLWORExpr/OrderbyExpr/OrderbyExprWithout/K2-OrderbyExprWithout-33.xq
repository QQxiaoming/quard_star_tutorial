(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-33                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Uncomparable values in order by, but without for clause(stable sort). :)
(:*******************************************************:)
let $i  := (xs:hexBinary("FF"), xs:hexBinary("FF"))
    stable order by $i[1]
    return $i