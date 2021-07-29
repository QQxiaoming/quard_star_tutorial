(:*******************************************************:)
(: Test: K-FilterExpr-75                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: fn:last() in predicate, leading to invalid operator mapping. :)
(:*******************************************************:)
2 eq (0, 1, "2")[last()]