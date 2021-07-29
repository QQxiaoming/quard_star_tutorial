(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-17                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Extract the effective boolean value from a order by expression. :)
(:*******************************************************:)
boolean((for $i in (false(), true(), true(), false(), true(), false())
order by $i
return $i)[1])