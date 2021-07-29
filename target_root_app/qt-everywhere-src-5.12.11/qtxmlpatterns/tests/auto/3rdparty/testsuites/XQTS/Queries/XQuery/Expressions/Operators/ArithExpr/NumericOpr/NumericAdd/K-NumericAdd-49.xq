(:*******************************************************:)
(: Test: K-NumericAdd-49                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: Complex combination of numeric arithmetics in order to stress operator precedence. :)
(:*******************************************************:)
1 + 2 * 4 + (1 + 2 + 3 * 4) eq 24