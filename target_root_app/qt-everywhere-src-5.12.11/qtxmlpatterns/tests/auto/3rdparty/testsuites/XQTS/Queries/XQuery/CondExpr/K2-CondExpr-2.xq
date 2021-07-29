(:*******************************************************:)
(: Test: K2-CondExpr-2                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An expression that can be simplified to any of the result branches, no matter the test branch. :)
(:*******************************************************:)
empty(if(<e>{current-time()}</e>)
then 1
else 1)