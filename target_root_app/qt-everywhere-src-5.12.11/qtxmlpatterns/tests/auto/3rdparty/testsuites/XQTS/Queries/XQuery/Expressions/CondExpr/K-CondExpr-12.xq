(:*******************************************************:)
(: Test: K-CondExpr-12                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: if-then clause combined with fn:boolean().   :)
(:*******************************************************:)
if(boolean((1, 2, 3, current-time())[1] treat as xs:integer)) then true() else 4