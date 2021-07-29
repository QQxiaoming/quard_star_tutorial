(:*******************************************************:)
(: Test: K-QuantExprWith-15                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Some-quantification carrying invalid type declarations. :)
(:*******************************************************:)
some $a as item()* in (1, 2), $b as xs:string in $a satisfies $b