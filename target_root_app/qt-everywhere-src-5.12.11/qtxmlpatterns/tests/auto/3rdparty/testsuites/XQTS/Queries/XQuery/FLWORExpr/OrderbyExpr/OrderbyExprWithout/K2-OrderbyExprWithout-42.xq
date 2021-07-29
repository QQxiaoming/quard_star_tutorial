(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-42                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply fn:string() on a for clause that only produces on item, and that cannot easily constant propagate. :)
(:*******************************************************:)
string(for $i in current-date()
    order by $i
return $i)