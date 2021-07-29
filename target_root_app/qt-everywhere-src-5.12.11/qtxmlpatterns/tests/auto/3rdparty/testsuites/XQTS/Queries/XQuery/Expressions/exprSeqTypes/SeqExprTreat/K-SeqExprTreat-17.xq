(:*******************************************************:)
(: Test: K-SeqExprTreat-17                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `(remove((5, 1e0), 2) treat as xs:integer) eq 5`. :)
(:*******************************************************:)
(remove((5, 1e0), 2) treat as xs:integer) eq 5