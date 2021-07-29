(:*******************************************************:)
(: Test: K-FilterExpr-51                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: Filter a sequence with instance of and a second predicate. :)
(:*******************************************************:)
((0, 1, 2, "a", "b", "c")[. instance of xs:string][. treat as xs:string eq "c"] treat as xs:string) eq "c"