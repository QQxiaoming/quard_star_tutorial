(:*******************************************************:)
(: Test: K-WhereExpr-8                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A for expression binding to one single value, combined with a negative where clause. :)
(:*******************************************************:)
(for $i in 1 where (1, current-time())[1] treat as xs:integer eq 1 return $i) eq 1