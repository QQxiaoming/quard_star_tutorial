(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-29                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An empty order by and an empty return clause, with node constructor. :)
(:*******************************************************:)
<r>
    {
    for $i in attribute name {()}
        order by ()
        return ()
    }
</r>