(:*******************************************************:)
(: Test: K-NumericMod-26                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `(xs:untypedAtomic("5") mod xs:double(3)) eq 2`. :)
(:*******************************************************:)
(xs:untypedAtomic("5") mod xs:double(3)) eq 2