(:*******************************************************:)
(: Test: K-NumericAdd-34                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-06-13T10:58:40+01:00                       :)
(: Purpose: Invoke operator '+' where one of the operands is of type none. :)
(:*******************************************************:)
exactly-one((4, error())) + 3
