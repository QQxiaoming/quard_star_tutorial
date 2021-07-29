(:*******************************************************:)
(: Test: K-GenCompEq-20                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An expression involving the '=' operator that trigger certain optimization paths in some implementations. :)
(:*******************************************************:)
count(remove(remove((current-time(), 1), 1), 1)) = 0