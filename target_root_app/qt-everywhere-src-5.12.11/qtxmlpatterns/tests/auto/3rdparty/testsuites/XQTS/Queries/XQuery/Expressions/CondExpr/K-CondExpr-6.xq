(:*******************************************************:)
(: Test: K-CondExpr-6                                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An expression involving the if expression that trigger certain optimization paths in some implementations(#3). :)
(:*******************************************************:)
string(if(boolean((1, current-time())[1] treat as xs:integer)) then true() else false()) eq "true"