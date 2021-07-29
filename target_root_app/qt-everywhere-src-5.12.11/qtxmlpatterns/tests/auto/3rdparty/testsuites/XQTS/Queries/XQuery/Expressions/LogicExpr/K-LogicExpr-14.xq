(:*******************************************************:)
(: Test: K-LogicExpr-14                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `true() or (0, current-time())[1] treat as xs:integer`. :)
(:*******************************************************:)
true() or (0, current-time())[1] treat as xs:integer