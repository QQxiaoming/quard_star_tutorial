(:*******************************************************:)
(: Test: K2-WhereExpr-1                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Multiple where clauses is not allowed.       :)
(:*******************************************************:)
for $a in 1
where true()
where true()
return $a