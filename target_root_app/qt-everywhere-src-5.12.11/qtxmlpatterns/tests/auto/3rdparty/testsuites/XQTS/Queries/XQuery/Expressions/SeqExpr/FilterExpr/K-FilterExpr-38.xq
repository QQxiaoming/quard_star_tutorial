(:*******************************************************:)
(: Test: K-FilterExpr-38                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: xs:anyURI values are invalid predicates.     :)
(:*******************************************************:)
(1, 2, 3)[(xs:anyURI("example.com/"), xs:anyURI("example.com/"))]