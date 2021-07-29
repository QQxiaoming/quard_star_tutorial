(:*******************************************************:)
(: Test: K-SeqExprCast-144                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `(xs:decimal(remove((3e3, 1.1), 1))) eq 1.1`. :)
(:*******************************************************:)
(xs:decimal(remove((3e3, 1.1), 1))) eq 1.1