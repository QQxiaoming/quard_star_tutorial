(:*******************************************************:)
(: Test: K-NumericMod-27                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `(xs:decimal(5) mod xs:untypedAtomic("3")) eq 2`. :)
(:*******************************************************:)
(xs:decimal(5) mod xs:untypedAtomic("3")) eq 2