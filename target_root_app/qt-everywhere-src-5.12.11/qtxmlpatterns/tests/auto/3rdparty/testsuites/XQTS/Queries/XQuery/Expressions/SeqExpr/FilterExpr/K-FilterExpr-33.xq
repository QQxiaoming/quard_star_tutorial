(:*******************************************************:)
(: Test: K-FilterExpr-33                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: The context item is used as the predicate, leading to a truth predicate. :)
(:*******************************************************:)
deep-equal((true(), true(), true()),
(false(), true(), true(), false(), true(), false())[.])