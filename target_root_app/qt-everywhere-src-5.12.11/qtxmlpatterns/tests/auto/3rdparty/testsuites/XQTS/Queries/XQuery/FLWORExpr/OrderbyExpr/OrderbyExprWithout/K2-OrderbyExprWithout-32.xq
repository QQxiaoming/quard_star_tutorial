(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-32                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A type error in order by, but without for clause(stable sort). :)
(:*******************************************************:)
let $i  := (1, 2, 3)
    stable order by $i
    return $i