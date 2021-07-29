(:*******************************************************:)
(: Test: K2-CondExpr-5                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: There is no exclamation mark operator in XQuery. :)
(:*******************************************************:)
if(!true())
then 2
else 3