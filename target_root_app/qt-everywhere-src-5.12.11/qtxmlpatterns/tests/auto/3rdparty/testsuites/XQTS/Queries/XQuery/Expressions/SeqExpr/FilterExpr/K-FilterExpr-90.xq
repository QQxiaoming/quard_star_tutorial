(:*******************************************************:)
(: Test: K-FilterExpr-90                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: Nested predicate with multiple calls to fn:last(). :)
(:*******************************************************:)
(1, 2, 3)[(last(), last())[2]]