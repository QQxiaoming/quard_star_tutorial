(:*******************************************************:)
(: Test: K-SeqMINFunc-15                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `min((3, xs:untypedAtomic("1"), xs:float(2))) instance of xs:double`. :)
(:*******************************************************:)
min((3, xs:untypedAtomic("1"), xs:float(2))) instance of xs:double