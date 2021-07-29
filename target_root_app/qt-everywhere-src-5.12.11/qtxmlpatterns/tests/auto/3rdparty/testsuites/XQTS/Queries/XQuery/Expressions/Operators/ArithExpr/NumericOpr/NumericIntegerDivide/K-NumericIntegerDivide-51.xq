(:*******************************************************:)
(: Test: K-NumericIntegerDivide-51                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke 'idiv' where an untypedAtomic conversion fails. :)
(:*******************************************************:)
(xs:float(9) idiv xs:untypedAtomic("five")) eq 1