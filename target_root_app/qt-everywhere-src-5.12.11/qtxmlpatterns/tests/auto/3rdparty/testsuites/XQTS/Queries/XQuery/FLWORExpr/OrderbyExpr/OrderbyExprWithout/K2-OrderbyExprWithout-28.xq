(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-28                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: xs:time and xs:date values cannot be compared. :)
(:*******************************************************:)
let $i := (xs:date("2001-02-03"), xs:time("01:02:03Z"))
order by $i
return $i