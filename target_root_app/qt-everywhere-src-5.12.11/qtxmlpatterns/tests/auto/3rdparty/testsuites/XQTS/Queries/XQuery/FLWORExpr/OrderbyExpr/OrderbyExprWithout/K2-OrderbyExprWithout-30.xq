(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-30                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An empty order by and an empty return clause, with atomic value. :)
(:*******************************************************:)
<r>
    {
    for $i in 1
        order by ()
        return ()
    }
</r>