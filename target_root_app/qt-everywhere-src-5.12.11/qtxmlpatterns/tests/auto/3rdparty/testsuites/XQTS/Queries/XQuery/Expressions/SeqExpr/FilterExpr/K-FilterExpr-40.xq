(:*******************************************************:)
(: Test: K-FilterExpr-40                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: Multiple xs:untypedAtomic values is an invalid predicate. :)
(:*******************************************************:)
(1, 2, 3)[(xs:untypedAtomic("content"), xs:untypedAtomic("content"))]