(:*******************************************************:)
(: Test: K-QuantExprWith-7                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Every-quantification; the empty-sequence() cannot have an occurrence indicator. :)
(:*******************************************************:)
every $a as empty-sequence()? in (1, 2) satisfies $a