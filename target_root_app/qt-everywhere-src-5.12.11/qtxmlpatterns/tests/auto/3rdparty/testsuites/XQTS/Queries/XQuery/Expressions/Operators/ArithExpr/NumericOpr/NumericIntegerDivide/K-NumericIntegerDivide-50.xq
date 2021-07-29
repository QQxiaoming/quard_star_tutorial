(:*******************************************************:)
(: Test: K-NumericIntegerDivide-50                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke 'idiv' where an untypedAtomic conversion fails. :)
(:*******************************************************:)
(xs:untypedAtomic("nine") idiv xs:float(5)) eq 1