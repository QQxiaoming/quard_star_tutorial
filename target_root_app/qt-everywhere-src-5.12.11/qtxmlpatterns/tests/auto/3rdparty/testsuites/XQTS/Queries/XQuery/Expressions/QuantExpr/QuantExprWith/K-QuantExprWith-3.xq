(:*******************************************************:)
(: Test: K-QuantExprWith-3                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Every-quantification carrying type declarations(cardinalities have no impact). :)
(:*******************************************************:)
every $a as item()? in (1, 2), $b as item()? in $a satisfies $b