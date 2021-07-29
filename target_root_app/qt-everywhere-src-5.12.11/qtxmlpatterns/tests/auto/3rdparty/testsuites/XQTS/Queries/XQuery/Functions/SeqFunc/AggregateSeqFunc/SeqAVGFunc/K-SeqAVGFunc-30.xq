(:*******************************************************:)
(: Test: K-SeqAVGFunc-30                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `avg((xs:float(1), 2, xs:untypedAtomic("3"))) instance of xs:double`. :)
(:*******************************************************:)
avg((xs:float(1), 2, xs:untypedAtomic("3"))) instance of xs:double