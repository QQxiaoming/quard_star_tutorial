(:*******************************************************:)
(: Test: K-SeqAVGFunc-13                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `avg((xs:untypedAtomic(3), xs:integer(0), xs:decimal(1))) instance of xs:double`. :)
(:*******************************************************:)
avg((xs:untypedAtomic(3), xs:integer(0), xs:decimal(1))) instance of xs:double