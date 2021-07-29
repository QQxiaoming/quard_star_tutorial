(:*******************************************************:)
(: Test: K-CondExpr-10                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An if-test applied on fn:count().            :)
(:*******************************************************:)
if(count((1, 2, 3, current-time(), 4))) then true() else 4