(:*******************************************************:)
(: Test: K-NumericDivide-35                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `(xs:untypedAtomic("3") div xs:double(3)) eq 1`. :)
(:*******************************************************:)
(xs:untypedAtomic("3") div xs:double(3)) eq 1