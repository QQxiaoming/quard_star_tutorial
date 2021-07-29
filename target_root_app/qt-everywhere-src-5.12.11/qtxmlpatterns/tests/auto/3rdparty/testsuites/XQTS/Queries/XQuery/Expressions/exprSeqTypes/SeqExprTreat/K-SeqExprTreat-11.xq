(:*******************************************************:)
(: Test: K-SeqExprTreat-11                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Implementations using the static typing feature, may raise XPTY0004 because one of the operands to operator 'eq' has cardinality 'one-or-more'. :)
(:*******************************************************:)
("asda" treat as xs:string +) eq "asda"