(:*******************************************************:)
(: Test: K-NumericAdd-36                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-06-13T10:58:40+01:00                       :)
(: Purpose: Invoke operator '+' where one of the operands is of type none. :)
(:*******************************************************:)
exactly-one((error(), 4)) + 3
