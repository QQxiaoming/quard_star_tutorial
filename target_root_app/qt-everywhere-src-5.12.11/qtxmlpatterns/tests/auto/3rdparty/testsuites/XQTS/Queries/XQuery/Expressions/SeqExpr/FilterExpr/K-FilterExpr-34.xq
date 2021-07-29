(:*******************************************************:)
(: Test: K-FilterExpr-34                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: The context item is used as the predicate, leading to a numeric predicate. :)
(:*******************************************************:)
deep-equal((2, 3, 4, 5, 7, 8, 9),
(0, 2, 3, 4, 5, 5, 7, 8, 10 - 1)[.])