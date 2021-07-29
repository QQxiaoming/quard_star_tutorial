(:*******************************************************:)
(: Test: K-QuantExprWith-19                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: No 'at' declaration is allowed in 'some'-quantification. :)
(:*******************************************************:)
some $a as item() at $p in (1, 2) satisfies $a