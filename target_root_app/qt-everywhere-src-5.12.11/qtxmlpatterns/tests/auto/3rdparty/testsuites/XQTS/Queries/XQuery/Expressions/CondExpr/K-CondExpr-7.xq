(:*******************************************************:)
(: Test: K-CondExpr-7                                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An if-test which EBV cannot be extracted from. :)
(:*******************************************************:)
(if(current-time()) then 1 else 0) eq 1