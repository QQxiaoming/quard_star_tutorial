(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-16                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Sort booleans.                               :)
(:*******************************************************:)
for $i in (false(), true(), true(), false(), true(), false())
order by $i
return $i