(:*******************************************************:)
(: Test: K-SeqExprCastable-3                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: '+' nor '?' is allowed as a cardinality specifier in 'castable as'. :)
(:*******************************************************:)
("one", "two") castable as xs:string+