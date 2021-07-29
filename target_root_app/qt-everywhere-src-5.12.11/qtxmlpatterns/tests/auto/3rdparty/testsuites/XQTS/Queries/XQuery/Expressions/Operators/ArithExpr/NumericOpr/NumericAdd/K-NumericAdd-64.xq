(:*******************************************************:)
(: Test: K-NumericAdd-64                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: Invoke operator '+' where one of the operands, using subsequence(), evaluates to an invalid cardinality. :)
(:*******************************************************:)
subsequence("a string", 1, 1) + 1