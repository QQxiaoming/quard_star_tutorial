(:*******************************************************:)
(: Test: K-LogicExpr-8                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Non-empty xs:anyURIs in the left branch of an or-expression has an EBV value of true. :)
(:*******************************************************:)
xs:anyURI("example.com/") or 0