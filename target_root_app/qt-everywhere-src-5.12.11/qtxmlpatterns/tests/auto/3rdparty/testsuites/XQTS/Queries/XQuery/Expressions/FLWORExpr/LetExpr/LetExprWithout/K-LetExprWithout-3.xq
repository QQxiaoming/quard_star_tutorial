(:*******************************************************:)
(: Test: K-LetExprWithout-3                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A value in a 'let' variable cannot be assigned to with 'in', it must be ':='. :)
(:*******************************************************:)
let $i in 5 return 3