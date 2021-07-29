(:*******************************************************:)
(: Test: K-FilterExpr-47                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: An excessive nesting of various predicates.  :)
(:*******************************************************:)
(0, 2, 4, 5)[1][1][1][true()][1][true()][1] eq 0