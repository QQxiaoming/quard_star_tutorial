(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-13                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A sort with a for-binding whose only purpose is sorting. :)
(:*******************************************************:)
for $a in (3, 2, 1),
    $b in (6, 5, 4)
    stable order by $b
    return $a