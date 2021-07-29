(:*******************************************************:)
(: Test: K-QuantExprWith-14                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Some-quantification carrying invalid type declarations. :)
(:*******************************************************:)
some $a as xs:integer+ in (1, 2), $b as xs:string* in $a satisfies $b