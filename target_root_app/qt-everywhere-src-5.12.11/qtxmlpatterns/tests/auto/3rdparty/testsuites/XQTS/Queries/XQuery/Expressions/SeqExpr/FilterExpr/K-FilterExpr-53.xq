(:*******************************************************:)
(: Test: K-FilterExpr-53                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: Two predicates, where one leading to an invalid operator mapping in the second. :)
(:*******************************************************:)
(0, 1, 2, "a", "b", "c")[. instance of xs:integer][. eq "c"] eq 0